#include "mwork.h"
#include <QDebug>
#include <QEventLoop>

MWork::MWork(qintptr socket, QObject *parent) : QObject(parent)
{
    fileName.clear();
    startRecv = true;
    isFileInfo = true;

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

void MWork::toGetSaveDir(QString dir)
{
    saveDir = dir;
    emit readSaveDir();
}

void MWork::recvFile()
{
    QByteArray recvArray = tcpSocket->readAll();
    if (startRecv) {
        //解析数量信息
        fileCount = QString(recvArray).section("##", 0, 0).toInt();
        startRecv = false;
        recvCount = 0;
        //获取保存目录
        emit goToGetSaveDir();
        QEventLoop loop;
        connect(this, &MWork::readSaveDir, &loop, &QEventLoop::quit);
        loop.exec();
        //判断存储目录是否存在，不在则创建
        if (!QDir().exists(saveDir))
            QDir().mkdir(saveDir);
    } else if (isFileInfo) {
        //解析文件信息
        fileName = QString(recvArray).section("##", 0, 0);
        fileSize = QString(recvArray).section("##", 1, 1).toLong();
        //创建文件
        file.setFileName(QString("%1/%2").arg(saveDir).arg(fileName));
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << file.errorString();
            return;
        }
        //预设文件大小，防止磁盘空间不足
        if (!file.resize(fileSize)) {
            qDebug() << file.errorString();
            file.close();
            return;
        }
        //初始化数据
        recvSize = 0;
        progress = 0;
        speedSize = 0;
        emit updateProgress(progress);
        emit updateSpeedSize(progress);
        //创建定时器
        mtimer = new QTimer(this);
        connect(mtimer, &QTimer::timeout, this, [=](){
            emit updateSpeedSize(speedSize);
            speedSize = 0;
        });
        mtimer->start(1000);

        isFileInfo = false;
    } else {
        //写入文件
        qint64 len = file.write(recvArray);
        recvSize += len;
        speedSize += len;
        //更新进度条
        tempProgress = static_cast<double>(recvSize)/static_cast<double>(fileSize)*100;
        if (tempProgress != progress) {
            progress = tempProgress;
            emit updateProgress(progress);
        }
        //单个文件接收完成
        if (recvSize == fileSize) {
            recvCount++;
            isFileInfo = true;
            mtimer->deleteLater();
            mtimer->stop();
            file.close();
            //返回保存文件名
            emit fileFinish(QString("[完成]%1").arg(fileName));
        }
        //所有文件接收完成
        if (recvCount == fileCount)
            startRecv = true;
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
