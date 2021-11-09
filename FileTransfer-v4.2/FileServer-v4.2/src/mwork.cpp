#include "mwork.h"
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QJsonObject>
#include <QHostAddress>
#include <QJsonParseError>

extern QMutex mutex;//全局互斥锁
extern qint64 speedSize;//单位时间接收总大小
extern QString saveDir;//全局保存目录

QMutex MWork::workMutex;//线程互斥锁
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
    tcpSocket->deleteLater();
    tcpSocket->close();
    thread->quit();
}

FileMsg MWork::parseFileMsg(QByteArray &msgByteArray)
{
    //校验JSON元素是否出错
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(msgByteArray, &error);
    if (doc.isNull() || error.error != QJsonParseError::NoError) {
        qWarning("parse json error:%s", qUtf8Printable(error.errorString()));
        return FileMsg();
    }
    //解析JSON数据，获取文件信息
    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        FileMsg msg;
        msg.fileName = obj["fileName"].toString();
        msg.fileSize = obj["fileSize"].toVariant().toLongLong();
        msg.block = obj["block"].toInt();
        msg.blockSize = obj["blockSize"].toVariant().toLongLong();
        return msg;
    }
    return FileMsg();
}

void MWork::setSaveDir(const QString &sDir)
{
    //获取保存目录
    workMutex.lock();//加锁
    dir = sDir;
    workMutex.unlock();//解锁
    //判断存储目录是否存在，不存在则创建
    if (!QDir().exists(dir))
        QDir().mkdir(dir);
}

bool MWork::createFile(const QString &fileName, const int &block, const qint64 &fileSize)
{
    file.setFileName(QString("%1/%2.00%3").arg(dir).arg(fileName).arg(block+1));
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("open file %s error:%s", qUtf8Printable(fileName), qUtf8Printable(file.errorString()));
        return false;
    }
    //预设文件大小，防止存储空间不足
    file.resize(fileSize);
    return true;
}

void MWork::calculateProgress(qint64 &len)
{
    workMutex.lock();//加锁
    totalRecvSize += len;
    tempProgress = static_cast<double>(totalRecvSize) / static_cast<double>(totalSize) * 100;
    if (tempProgress != progress) {
        progress = tempProgress;
        emit updateProgress(progress);
        if (progress == 100) {
            totalSize = totalRecvSize = 0;
            progress = tempProgress = 0;
        }
    }
    workMutex.unlock();//解锁
}

void MWork::startConnect()
{
    //创建socket
    tcpSocket = new QTcpSocket;
    //设置socket描述符
    tcpSocket->setSocketDescriptor(socket);
    ip = QHostAddress(tcpSocket->peerAddress().toIPv4Address()).toString();
    port = tcpSocket->peerPort();
    //发送socket信息
    emit socketMsg(ip, port);
    qDebug() << QString("connect [%1:%2] successful").arg(ip).arg(port);

    //连接断开
    connect(tcpSocket, &QTcpSocket::disconnected, this, [=](){
        qDebug() << QString("disconnect [%1:%2]").arg(ip).arg(port);
        emit overConnect();
    });

    //有数据
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MWork::recvFile);
}

void MWork::recvFile()
{
    QByteArray recvArray = tcpSocket->readAll();

    if (startRecv) {
        //获取文件信息
        msg = parseFileMsg(recvArray);
        if (msg.isEmpty()) {
            qWarning("file message is empty");
            return;
        }
        //每个文件由第一分块记录总大小并发送更新进度条信号
        if (msg.block == 0) {
            workMutex.lock();//加锁
            totalSize += msg.fileSize;
            workMutex.unlock();//解锁
            emit updateTotalSize(msg.fileSize);
            emit updateProgress(progress);
        }
        //设置保存目录
        setSaveDir(saveDir);
        //创建文件
        if (!createFile(msg.fileName, msg.block, msg.blockSize))
            return;
        //若块为大小为0，则直接返回
        if (msg.blockSize == 0) {
            file.close();
            emit fileFinished(msg);
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
        calculateProgress(len);
        //单个文件接收完成
        if (recvSize == msg.blockSize) {
            file.close();
            startRecv = true;
            //返回保存文件名
            emit fileFinished(msg);
            qDebug() << QString("receive file %1.00%2 finished").arg(msg.fileName).arg(msg.block+1);
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
