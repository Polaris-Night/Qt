#include "mwork.h"
#include <QDebug>
#include <QMutex>

extern QMutex mutex;//全局线程同步互斥锁
extern qint64 speedSize;//单位时间接收总大小
extern QString saveDir;//全局保存目录

QReadWriteLock MWork::rwLock;
qint64 MWork::totalSize = 0;//全部文件总大小
qint64 MWork::totalRecvSize = 0;//总接收大小
int MWork::tempProgress = 0;//临时进度值
int MWork::progress = 0;//进度值

MWork::MWork(const qintptr &socket, QObject *parent) : QObject(parent)
{
    startRecv = true;
    recvSize = 0;

    //创建线程
    thread = new MThread;
    //work依赖thread线程执行
    this->moveToThread(thread);
    //启动线程
    thread->start();
    //获取socket描述符
    this->socket = socket;
    connect(this, &MWork::toConnect, this, &MWork::startConnect);
    //连接
    emit toConnect();
}

MWork::~MWork()
{
    thread->quit();
}

void MWork::startConnect()
{
    //创建socket
    tcpSocket = new QTcpSocket;
    //设置socket描述符
    tcpSocket->setSocketDescriptor(socket);
    //发送socket信息
    emit socketMsg(QHostAddress(tcpSocket->peerAddress().toIPv4Address()).toString(), tcpSocket->peerPort());

    //连接断开
    connect(tcpSocket, &QTcpSocket::disconnected, this, [=](){
        emit overConnect();
        tcpSocket->deleteLater();
        tcpSocket->close();
    });

    //有数据
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MWork::recvFile);
}

void MWork::recvFile()
{
    QByteArray recvArray = tcpSocket->readAll();

    if (startRecv) {
        //校验JSON元素是否出错
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(recvArray, &err);
        if (doc.isNull() || err.error != QJsonParseError::NoError) {
            qDebug() << err.errorString();
            return;
        }
        //解析JSON数据，获取文件信息
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            msg.fileName = obj["fileName"].toString();
            msg.fileSize = obj["fileSize"].toVariant().toLongLong();
            msg.block = obj["block"].toInt();
            msg.blockSize = obj["blockSize"].toVariant().toLongLong();
            //每个文件由第一分块记录总大小并发送更新进度条信号
            if (msg.block == 0) {
                mutex.lock();//加锁
                totalSize += msg.fileSize;
                emit updateTotalSize(totalSize);
                mutex.unlock();//解锁
                emit updateProgress(progress);
            }
        }
        //获取保存目录
        rwLock.lockForRead();//加读锁
        dir = saveDir;
        rwLock.unlock();//解读锁
        //判断存储目录是否存在，不在则创建
        if (!QDir().exists(dir))
            QDir().mkdir(dir);
        //创建文件
        file.setFileName(QString("%1/%2.00%3").arg(dir).arg(msg.fileName).arg(msg.block+1));
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << file.errorString();
            return;
        }
        //预设文件大小，防止存储空间不足
        file.resize(msg.blockSize);
        //若最后一块为大小为0，则直接返回
        if (msg.block == 3 && msg.blockSize == 0) {
            file.close();
            emit fileFinish(msg);
            return;
        }
        startRecv = false;
    } else {
        //写入文件
        qint64 len = file.write(recvArray);
        recvSize += len;
        //统计单位时间接收大小
        mutex.lock();//加锁
        speedSize += len;
        mutex.unlock();//解锁
        //计算进度值并更新进度条
        rwLock.lockForWrite();//加写锁
        totalRecvSize += len;
        tempProgress = static_cast<double>(totalRecvSize)/static_cast<double>(totalSize)*100;
        if (tempProgress != progress) {
            progress = tempProgress;
            emit updateProgress(progress);
            if (progress == 100) {
                totalSize = totalRecvSize = 0;
                progress = tempProgress = 0;
            }
        }
        rwLock.unlock();//解写锁
        //单个文件接收完成
        if (recvSize == msg.blockSize) {
            file.close();
            startRecv = true;
            //返回保存文件名
            emit fileFinish(msg);
        }
    }
}

void MWork::toDisconnectThis()
{
    if (tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        tcpSocket->disconnectFromHost();
        if (tcpSocket->state() == QAbstractSocket::UnconnectedState || tcpSocket->waitForDisconnected(1000)) {}
    }
}

void MWork::toDisconnectAll()
{
    if (tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        tcpSocket->disconnectFromHost();
        if (tcpSocket->state() == QAbstractSocket::UnconnectedState || tcpSocket->waitForDisconnected(1000)) {}
    }
}
