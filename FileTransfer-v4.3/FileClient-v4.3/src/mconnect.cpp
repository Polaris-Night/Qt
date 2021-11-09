#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThreadPool>

#include "mconnect.h"
#include "msendfilethread.h"

/***********************************工作线程***************************************/
MConnect::MConnect(const QString &ip, const quint16 &port, QObject *parent)
    : QObject(parent)
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
    connect(this, &MConnect::goToConnect, this, &MConnect::toConnect);
    emit goToConnect();
}

MConnect::~MConnect()
{
    tcpSocket->deleteLater();
    tcpSocket->close();
    thread->quit();
}

MFileMsgManager MConnect::getFileMsg(const QStringList &fileList, const int &blockCount)
{
    //获取文件信息
    QFileInfo info;
    MFileMsgManager manager(fileList.size(), blockCount);
    for (int i = 0; i < manager.fileCount; i++) {
        //设置文件
        info.setFile(fileList.at(i));
        //计算文件总大小
        totalSize += info.size();
        //存储文件信息
        manager[i].fileName = info.fileName();
        manager[i].fileSize = info.size();
        manager[i].filePath = fileList.at(i);
        //计算分块大小
        for (int j = 0; j < blockCount; j++) {
            if (j == (blockCount - 1))
                manager[i].blockSize[j] = manager[i].fileSize - manager[i].blockSize[0]*j;
            else
                manager[i].blockSize[j] = manager[i].fileSize / blockCount;
        }
    }
    return manager;
}

void MConnect::toConnect()
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

    //连接断开
    connect(tcpSocket, &QTcpSocket::disconnected, this, &MConnect::overConnect);
}

void MConnect::toDisconnect()
{
    if (tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        tcpSocket->disconnectFromHost();
        if ((tcpSocket->state() == QAbstractSocket::UnconnectedState) || tcpSocket->waitForDisconnected(1000)) {}
    }
}

bool sizeCompare(const BlockMsg &b1, const BlockMsg &b2)
{
    return b1.blockSize < b2.blockSize;
}

void MConnect::toSendFile(const QStringList &fileList)
{
    //初始化文件总大小和进度条
    emit updateSendProgress(0);

    //获取文件信息
    MFileMsgManager manager = getFileMsg(fileList, 4);
    QVector<BlockMsg> blockMsgVector = manager.toBlockMsg();
    MSendFileThread::setTotalSize(manager.totalSize());
    QThreadPool::globalInstance()->setMaxThreadCount(20);

    std::sort(blockMsgVector.begin(), blockMsgVector.end(), sizeCompare);
    //根据文件数量创建线程发送文件
    for (int i = 0, size = blockMsgVector.size(); i < size; i++) {
        //创建线程
        MSendFileThread *thread = new MSendFileThread(ip, port, blockMsgVector[i]);
        //更新进度条信号
        connect(thread, &MSendFileThread::updateSendProgress, this, &MConnect::updateSendProgress);
        connect(thread, &MSendFileThread::sendFinish, this, &MConnect::sendFinish);
        //启动线程
        QThreadPool::globalInstance()->start(thread);
    }
}
