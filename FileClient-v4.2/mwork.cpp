#include "mwork.h"
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>

/***********************************工作线程***************************************/
MWork::MWork(const QString &ip, const quint16 &port, QObject *parent) : QObject(parent)
{
    thread = new MThread;
    //work依赖thread线程执行
    this->moveToThread(thread);
    //启动线程
    thread->start();
    //获取地址和端口
    this->ip = ip;
    this->port = port;

    //连接
    connect(this, &MWork::goToConnect, this, &MWork::toConnect);
    emit goToConnect();
}

MWork::~MWork()
{
    tcpSocket->deleteLater();
    tcpSocket->close();
    thread->quit();
}

FileMsg MWork::getFileMsg(const QStringList &fileList, const int &blockCount)
{
    //获取文件信息
    QFileInfo info;
    FileMsg msg(fileList.size(), blockCount);
    for (int i = 0; i < msg.fileCount; i++) {
        info.setFile(fileList.at(i));
        //计算文件总大小
        totalSize += info.size();
        //存储文件信息
        msg.fileMsg[i].fileName = info.fileName();
        msg.fileMsg[i].fileSize = info.size();
        msg.fileMsg[i].filePath = fileList.at(i);
        //计算分块大小
        for (int j = 0, blockCount = msg.blockCount; j < blockCount; j++) {
            if (j == blockCount-1)
                msg.fileMsg[i].blockSize[j] = msg.fileMsg[i].fileSize - msg.fileMsg[i].blockSize[0]*j;
            else
                msg.fileMsg[i].blockSize[j] = msg.fileMsg[i].fileSize / blockCount;
        }
    }
    return msg;
}

void MWork::toConnect()
{
    //创建socket
    tcpSocket = new QTcpSocket;
    //连接服务端
    tcpSocket->connectToHost(ip, port);
    //超时3s连接失败
    if (!tcpSocket->waitForConnected(3000)) {
        emit connectResult(QString("[Error]%1").arg(tcpSocket->errorString()));
        qWarning("connect [%s:%s] error:%s", qUtf8Printable(ip), qUtf8Printable(QString::number(port)), qUtf8Printable(tcpSocket->errorString()));
        return;
    }
    //连接成功
    emit connectResult(QString("[%1:%2]已连接").arg(tcpSocket->peerName()).arg(tcpSocket->peerPort()));
    qDebug() << QString("connect [%1:%2] successful").arg(tcpSocket->peerName()).arg(tcpSocket->peerPort());

    //连接断开
    connect(tcpSocket, &QTcpSocket::disconnected, this, [=](){
        qDebug() << QString("disconnect [%1:%2]").arg(tcpSocket->peerName()).arg(tcpSocket->peerPort());
        emit overConnect();
    });
}

void MWork::toDisconnect()
{
    if (tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        tcpSocket->disconnectFromHost();
        if (tcpSocket->state() == QAbstractSocket::UnconnectedState || tcpSocket->waitForDisconnected(1000)) {}
    }
}

void MWork::toSendFile(const QStringList &fileList)
{
    //初始化文件总大小和进度条
    totalSize = totalSendSize = 0;
    progress = tempProgress = 0;
    emit updateProgress(progress);

    //获取文件信息
    FileMsg msg = getFileMsg(fileList, 4);
    QVector<BlockMsg> blockMsg = msg.toBlockMsg();

    //根据文件数量创建线程发送文件
    for (int i = 0, size = blockMsg.size(); i < size; i++) {
        //创建线程
        SendFileThread *work = new SendFileThread(ip, port);
        //设置文件信息
        work->setFileMeg(blockMsg[i]);
        //接收返回文件已发送大小并计算进度值
        connect(work, &SendFileThread::partSendSize, this, [=](qint64 sendSize){
            totalSendSize += sendSize;
            tempProgress = static_cast<double>(totalSendSize) / static_cast<double>(totalSize) * 100;
            if (tempProgress != progress) {
                progress = tempProgress;
                emit updateProgress(progress);
                if (progress == 100)
                    emit sendFinish();
            }
        });
        //启动线程
        work->start();
    }
}

/*********************************发送文件线程***************************************/
SendFileThread::SendFileThread(const QString &ip, const quint16 &port, QObject *parent) : QThread(parent)
{
    this->ip = ip;
    this->port = port;
}

SendFileThread::~SendFileThread()
{

}

void SendFileThread::setFileMeg(const QString &fileName, const qint64 &fileSize, const QString &filePath, const int &block, const qint64 &blockSize)
{
    msg.fileName = fileName;
    msg.fileSize = fileSize;
    msg.filePath = filePath;
    msg.block = block;
    msg.blockSize = blockSize;
}

void SendFileThread::setFileMeg(const BlockMsg &blockMsg)
{
    msg = blockMsg;
}

void SendFileThread::run()
{
    connect(this, &SendFileThread::finished, this, &SendFileThread::deleteLater);

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
    obj.insert("fileName", msg.fileName);
    obj.insert("fileSize", msg.fileSize);
    obj.insert("block", msg.block);
    obj.insert("blockSize", msg.blockSize);

    socket->write(QJsonDocument(obj).toJson());
    socket->waitForBytesWritten();

    msleep(20);

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
    char buffer[4*1024];
    size_t bufferSize = sizeof(buffer);
    do {
        //读取
        memset(buffer, 0, bufferSize);
        len = file.read(buffer, blockSize < bufferSize ? blockSize : bufferSize);
        //发送
        len = socket->write(buffer, len);
        socket->waitForBytesWritten();
        blockSize -= len;
        emit partSendSize(len);
    } while (len > 0 && blockSize > 0);
    //关闭文件
    file.close();
    qDebug() << QString("file %1 send finished").arg(QString("%1.00%2").arg(msg.fileName).arg(msg.block+1));

    MThread::sleep(1);
    socket->disconnectFromHost();
}
