#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QHostAddress>
#include <QJsonParseError>
#include <QTextCodec>

#include "mrecvfilethread.h"

extern QString saveDir;//全局保存目录

QReadWriteLock MRecvFileThread::rwLock;//线程读写锁
QMutex MRecvFileThread::threadMutex;//线程互斥锁
int MRecvFileThread::recvProgress = 0;//进度值
bool MRecvFileThread::allStart = false;//开始接收标志
MProgress MRecvFileThread::prog;//进度计算
MSpeed MRecvFileThread::speed;//速度计算

MRecvFileThread::MRecvFileThread(const qintptr &socket, QObject *parent)
    : QObject(parent), socketFd(socket), recvSize(0)
{
    isFileMsg = true;
    //创建线程
    thread = new MThread;
    //work依赖thread线程执行
    this->moveToThread(thread);
    //启动线程
    thread->start();
    connect(this, &MRecvFileThread::toConnect, this, &MRecvFileThread::startConnect);
    connect(this, &MRecvFileThread::goToDisconnect, this, &MRecvFileThread::toDisconnect);
    //数据流
    wStream.setDevice(&file);
    wStream.setVersion(QDataStream::Qt_5_12);
}

MRecvFileThread::~MRecvFileThread()
{
    tcpSocket->deleteLater();
    tcpSocket->close();
    thread->quit();
}

QString MRecvFileThread::getSpeedSize()
{
    QMutexLocker locker(&threadMutex);
    return speed.getSpeedSize();
}

void MRecvFileThread::toDisconnect()
{
    if (tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        tcpSocket->disconnectFromHost();
        if ((tcpSocket->state() == QAbstractSocket::UnconnectedState) || tcpSocket->waitForDisconnected(1000)) {}
    }
}

BlockMsg MRecvFileThread::parseFileMsg(const QByteArray &msgByteArray)
{
    //校验JSON元素是否出错
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(msgByteArray, &error);
    if (doc.isNull() || error.error != QJsonParseError::NoError) {
        qCritical("parse json error:%s", qUtf8Printable(error.errorString()));
        return BlockMsg();
    }
    if (!doc.isObject()) {
        qCritical("doc is not a object");
        return BlockMsg();
    }
    //解析JSON数据，获取文件信息
    QJsonObject obj = doc.object();
    BlockMsg msg;
    msg.fileName = obj["fileName"].toString();
    msg.fileSize = obj["fileSize"].toVariant().toLongLong();
    msg.block = obj["block"].toInt();
    msg.blockSize = obj["blockSize"].toVariant().toLongLong();
    return msg;
}

void MRecvFileThread::getSaveDir()
{
    //获取保存目录
    rwLock.lockForRead();//加读锁
    dir = saveDir;
    rwLock.unlock();//解读锁
    //判断存储目录是否存在，不存在则创建
    if (!QDir().exists(dir))
        QDir().mkdir(dir);
}

bool MRecvFileThread::createFile()
{
    file.setFileName(QString("%1/%2.00%3").arg(dir).arg(msg.fileName).arg(msg.block+1));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("open file %s error:%s", qUtf8Printable(msg.fileName), qUtf8Printable(file.errorString()));
        return false;
    }
    //预设文件大小，防止存储空间不足
    file.resize(msg.blockSize);
    return true;
}

void MRecvFileThread::calculateSpeedAndProgress(const qint64 &len)
{
    QMutexLocker locker(&threadMutex);
    //统计速度
    speed.addSize(len);
    //统计总接收大小
    prog.addSize(len);
    //计算进度值
    int tempRecvProgress = prog.getProgress();
    //进度值不变时解锁并返回
    if (tempRecvProgress == recvProgress)
        return;
    //更新进度值
    recvProgress = tempRecvProgress;
    emit updateRecvProgress(recvProgress);
    if (recvProgress < 100)
        return;
    //重置
    recvProgress = 0;
    speed.clear();
    prog.clear();
    allStart = false;
}

void MRecvFileThread::startConnect()
{
    //创建socket
    tcpSocket = new QTcpSocket;
    //设置socket描述符
    tcpSocket->setSocketDescriptor(socketFd);
    ip = QHostAddress(tcpSocket->peerAddress().toIPv4Address()).toString();
    port = tcpSocket->peerPort();
    //发送socket信息
    emit socketMsg(QString("[%1:%2]").arg(ip).arg(port));

    //连接断开
    connect(tcpSocket, &QTcpSocket::disconnected, this, &MRecvFileThread::overConnect);
    //有数据
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MRecvFileThread::recvFile);
}

void MRecvFileThread::recvFile()
{
    QByteArray recvArray = tcpSocket->readAll();

    if (isFileMsg) {
        //获取文件信息
        msg = parseFileMsg(recvArray);
        if (msg.isEmpty()) {
            qWarning("file message is empty");
            tcpSocket->disconnectFromHost();
            return;
        }
        //每个文件由第一分块记录总大小并发送更新进度条信号
        if (msg.block == 0) {
            QMutexLocker locker(&threadMutex);
            prog.setTotalSize(prog.getTotalSize() + msg.fileSize);
            emit updateTotalSize(msg.fileSize);
            if (!allStart) {
                allStart = true;
                emit updateRecvProgress(recvProgress);
            }
        }
        //获取保存目录
        getSaveDir();
        //创建文件
        if (!createFile())
            return;
        //若块大小为0，则直接返回
        if (msg.blockSize == 0) {
            file.close();
            emit finishRecv(msg);
            qDebug() << QString("receive file %1.00%2 finished").arg(msg.fileName).arg(msg.block+1);
            return;
        }
        isFileMsg = false;
    } else {
        //写入文件
        qint64 len = wStream.writeRawData(recvArray, recvArray.size());
        recvSize += len;
        //计算速度及进度值
        calculateSpeedAndProgress(len);
        //单个文件接收完成
        if (recvSize == msg.blockSize) {
            file.close();
            isFileMsg = true;
            //返回保存文件名
            emit finishRecv(msg);
            qDebug() << QString("receive file %1.00%2 finished").arg(msg.fileName).arg(msg.block+1);
        }
    }
}
