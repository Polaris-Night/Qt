#include "mthread.h"
#include <QDebug>

MThread::MThread(QObject *parent) : QThread(parent)
{
    this->tcpSocket = nullptr;
    this->outputDirPath = "./output";
    this->isFileInfo = true;
    this->fileName.clear();
    this->fileSize = 0;
    this->receiveSize = 0;
}

MThread::~MThread()
{
    if (this->tcpSocket != nullptr) {
        QMetaObject::invokeMethod(this->tcpSocket, std::bind(static_cast<void(QTcpSocket::*)()>(&QTcpSocket::disconnectFromHost), this->tcpSocket));
        QMetaObject::invokeMethod(this->tcpSocket, std::bind(static_cast<void(QTcpSocket::*)()>(&QTcpSocket::close), this->tcpSocket));
        this->tcpSocket->deleteLater();
        this->tcpSocket = nullptr;
        //qDebug() << "release socket";
    }
    //qDebug() << "release mthread";
}

void MThread::initTcpSocket(QString mIp, qint16 mPort)
{
    this->ip = mIp;
    this->port = mPort;
}

void MThread::readFile()
{
    //读取socket接收的数据
    QByteArray receiveArray = this->tcpSocket->readAll();
    //处理文件信息
    if (this->isFileInfo) {
        //解包
        this->fileName = QString(receiveArray).section("##", 0, 0);
        this->fileSize = QString(receiveArray).section("##", 1, 1).toInt();
        this->receiveSize = 0;
        //判断存储目录
        QDir dir;
        if (!dir.exists(this->outputDirPath)) {
            dir.mkpath(QString("./output"));
        }
        //创建文件
        this->file.setFileName(QString("%1/%2").arg(this->outputDirPath).arg(this->fileName));
        if (!this->file.open(QIODevice::WriteOnly)) {
            qDebug() << this->file.errorString();
            QMetaObject::invokeMethod(this->tcpSocket, std::bind(static_cast<void(QTcpSocket::*)()>(&QTcpSocket::disconnectFromHost), this->tcpSocket));
            QMetaObject::invokeMethod(this->tcpSocket, std::bind(static_cast<void(QTcpSocket::*)()>(&QTcpSocket::close), this->tcpSocket));
            this->isFileInfo = true;
            return;
        }
        this->isFileInfo = false;
        //通知主线程初始化进度条
        emit initProgress(this->fileSize/1024);
    } else {
        //处理文件内容
        qint64 len = this->file.write(receiveArray);
        this->receiveSize += len;
        //更新进度条
        //emit updateProgress(this->receiveSize/1024);
        if (this->receiveSize == this->fileSize) {
            //给服务端发送消息
            QMetaObject::invokeMethod(this->tcpSocket, std::bind(static_cast<qint64(QTcpSocket::*)(const QByteArray &)>(&QTcpSocket::write), this->tcpSocket, QByteArray("File done")));
            //关闭socket
            QMetaObject::invokeMethod(this->tcpSocket, std::bind(static_cast<void(QTcpSocket::*)()>(&QTcpSocket::disconnectFromHost), this->tcpSocket));
            QMetaObject::invokeMethod(this->tcpSocket, std::bind(static_cast<void(QTcpSocket::*)()>(&QTcpSocket::close), this->tcpSocket));
            //关闭文件
            this->file.close();
            this->isFileInfo = true;
            //文件接收完成提示信息
            QDir dirPath;
            emit finishProgress(QString("%1/output/%2").arg(dirPath.absolutePath()).arg(this->fileName));
        }
    }
}

void MThread::run()
{
    if (this->tcpSocket == nullptr)
        this->tcpSocket = new QTcpSocket;

    //连接服务端
    this->tcpSocket->connectToHost(this->ip, this->port);

    //判断是否连接成功
    if (!this->tcpSocket->waitForConnected(2000)) {
        emit sendSocketResult(QString("[Error]%1").arg(this->tcpSocket->errorString()));
        return;
    }

    //连接成功给主线程发信号
    emit sendSocketResult(QString("正在与[%1]通信").arg(this->tcpSocket->peerName()));

    //接收文件
    connect(this->tcpSocket, &QTcpSocket::readyRead, this, &MThread::readFile);

    //socket连接断开
    connect(this->tcpSocket, &QTcpSocket::disconnected, this, [=](){
        QMetaObject::invokeMethod(this->tcpSocket, std::bind(static_cast<void(QTcpSocket::*)()>(&QTcpSocket::disconnectFromHost), this->tcpSocket));
        QMetaObject::invokeMethod(this->tcpSocket, std::bind(static_cast<void(QTcpSocket::*)()>(&QTcpSocket::close), this->tcpSocket));
        this->tcpSocket->deleteLater();
        this->tcpSocket = nullptr;
        emit statusDisconnect();
    });

    //保持线程运行
    this->exec();
}
