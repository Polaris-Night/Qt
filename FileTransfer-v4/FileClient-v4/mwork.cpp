#include "mwork.h"
#include <QDebug>

MWork::MWork(QString ip, quint16 port, QObject *parent) : QObject(parent)
{
    thread = new MThread;
    //work依赖thread线程执行
    this->moveToThread(thread);
    //启动线程
    thread->start();
    //获取地址和端口
    this->ip = ip;
    this->port = port;
    connect(this, &MWork::goToConnect, this, &MWork::toConnect);

    //连接
    emit goToConnect();
}

MWork::~MWork()
{
    mtimer->deleteLater();
    thread->quit();
}

void MWork::toConnect()
{
    //创建定时器
    mtimer = new QTimer;
    connect(mtimer, &QTimer::timeout, this, [=](){
        mtimer->stop();
        loop.quit();
    });
    //创建socket
    tcpSocket = new QTcpSocket;
    //连接服务端
    tcpSocket->connectToHost(ip, port);
    //超时3s连接失败
    if (!tcpSocket->waitForConnected(3000)) {
        tcpSocket->deleteLater();
        tcpSocket->close();
        emit connectResult(QString("[Error]%1").arg(tcpSocket->errorString()));
        return;
    }
    //连接成功
    emit connectResult(QString("[%1:%2]已连接").arg(tcpSocket->peerName()).arg(tcpSocket->peerPort()));

    //连接断开
    connect(tcpSocket, &QTcpSocket::disconnected, this, [=](){
        emit overConnect();
        tcpSocket->deleteLater();
        tcpSocket->close();
    });
}

void MWork::toDisconnect()
{
    if (tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        tcpSocket->disconnectFromHost();
        if (tcpSocket->state() == QAbstractSocket::UnconnectedState || tcpSocket->waitForDisconnected(1000)) {}
    }
}

void MWork::toSendFile(QStringList fileList)
{
    qint64 len = 0;
    fileCount = fileList.size();
    //发送文件数量
    QJsonObject jsonObj;
    jsonObj.insert("fileCount", QJsonValue::fromVariant(QString("%1").arg(fileCount).toUtf8()));
    len = tcpSocket->write(QJsonDocument(jsonObj).toBinaryData());
    tcpSocket->waitForBytesWritten();
    mtimer->start(20);
    loop.exec();
    sendCount = 0;
    if (len <= 0) {
        qDebug() << tcpSocket->errorString();
        return;
    }
    for (int i = 0; i < fileCount; i++) {
        //获取文件信息
        file.setFileName(fileList.at(i));
        info.setFile(file);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << file.errorString();
            return;
        }
        fileName = info.fileName();
        fileSize = info.size();
        //发送文件信息
        QJsonObject jsonObj;
        jsonObj.insert("fileName", fileName);
        jsonObj.insert("fileSize", QJsonValue::fromVariant(QString("%1").arg(fileSize).toUtf8()));
        len = tcpSocket->write(QJsonDocument(jsonObj).toBinaryData());
        tcpSocket->waitForBytesWritten();
        mtimer->start(20);
        loop.exec();
        if (len <= 0) {
            qDebug() << tcpSocket->errorString();
            return;
        }
        //初始化数据
        progress = 0;
        sendSize = 0;
        len = 0;
        char buffer[4*1024];
        emit updateProgress(progress);
        //发送文件
        do {
            //读取文件内容
            len = file.read(buffer, sizeof(buffer));
            //发送
            len = tcpSocket->write(buffer, len);
            tcpSocket->waitForBytesWritten();
            //记录
            sendSize += len;
            tempProgress = static_cast<double>(sendSize)/static_cast<double>(fileSize)*100;
            if (tempProgress != progress) {
                progress = tempProgress;
                emit updateProgress(progress);
            }
        } while (len > 0);
        //单个文件发送完成
        if (sendSize == fileSize) {
            sendCount++;
            file.close();
            mtimer->start(20);
            loop.exec();
        }
    }
    //全部文件发送完成
    if (sendCount == fileCount)
        emit sendFinish();
}
