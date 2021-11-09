#include <QTcpSocket>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDataStream>

#include "msendfilethread.h"

QMutex MSendFileThread::mutex;
MProgress MSendFileThread::prog;
MSpeed MSendFileThread::speed;

MSendFileThread::MSendFileThread(const QString &ip, const quint16 &port, const BlockMsg &blockMsg, QObject *parent)
    : QObject(parent), QRunnable()
{
    this->setAutoDelete(true);
    this->ip = ip;
    this->port = port;
    this->msg = blockMsg;
}

MSendFileThread::~MSendFileThread()
{

}

void MSendFileThread::setTotalSize(const qint64 &size)
{
    QMutexLocker locker(&mutex);
    prog.setTotalSize(size);
}

QString MSendFileThread::getSpeedSize()
{
    QMutexLocker locker(&mutex);
    return speed.getSpeedSize();
}

void MSendFileThread::run()
{
    QTcpSocket *socket = new QTcpSocket;
    //连接服务端
    socket->connectToHost(ip, port);
    //超时3s连接失败
    if (!socket->waitForConnected(3000)) {
        socket->deleteLater();
        socket->close();
        return;
    }

    //连接成功
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    //计算偏移量
    qint64 offset;
    //若为最后一块，则偏移量=总大小-最后一块大小
    offset = (msg.block == 3) ? (msg.fileSize - msg.blockSize) : (msg.blockSize * msg.block);

    //发送文件信息，格式:文件名+文件大小+分块编号+分块大小
    QJsonObject obj;
    obj.insert(QStringLiteral("fileName"), msg.fileName);
    obj.insert(QStringLiteral("fileSize"), msg.fileSize);
    obj.insert(QStringLiteral("block"), msg.block);
    obj.insert(QStringLiteral("blockSize"), msg.blockSize);

    socket->write(QJsonDocument(obj).toJson());
    socket->waitForBytesWritten();

    QThread::msleep(20);

    //发送文件数据
    //打开文件
    QFile file(msg.filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("open file %s error:%s", qUtf8Printable(msg.fileName), qUtf8Printable(file.errorString()));
        socket->disconnectFromHost();
        return;
    }
    //偏移
    file.seek(offset);
    //读取文件
    qint64 len = 0;
    qint64 blockSize = msg.blockSize;
    char buffer[4096];
    qint64 bufferSize = sizeof(buffer);
    QDataStream rStream(&file);
    rStream.setVersion(QDataStream::Qt_5_12);
    do {
        //读取
        memset(buffer, 0, bufferSize);
        len = rStream.readRawData(buffer, blockSize < bufferSize ? blockSize : bufferSize);
        //发送
        len = socket->write(buffer, len);
        socket->waitForBytesWritten();
        blockSize -= len;
        calculateSpeedAndProgress(len);
    } while (len > 0 && blockSize > 0);
    //关闭文件
    file.close();
    qDebug() << QString("file %1 send finished").arg(QString("%1.00%2").arg(msg.fileName).arg(msg.block+1));

    QThread::sleep(1);
    socket->disconnectFromHost();
}

void MSendFileThread::calculateSpeedAndProgress(const qint64 &len)
{
    QMutexLocker locker(&mutex);
    static int sendProgress = 0;
    speed.addSize(len);
    prog.addSize(len);
    int tempSendProgress = prog.getProgress();
    if (tempSendProgress == sendProgress)
        return;
    sendProgress = tempSendProgress;
    emit updateSendProgress(sendProgress);
    if (sendProgress < 100)
        return;
    sendProgress = 0;
    speed.clear();
    prog.clear();
    emit sendFinish();
}
