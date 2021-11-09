#include "mthread.h"

MThread::MThread(QObject *parent) : QThread(parent)
{
    this->tcpSocket = nullptr;
}

MThread::~MThread()
{
    if (this->tcpSocket != nullptr) {
        this->tcpSocket->disconnectFromHost();
        this->tcpSocket->close();
        this->tcpSocket->deleteLater();
        this->tcpSocket = nullptr;
        //qDebug() << "release socket";
    }
}

void MThread::startSendFile()
{
    qint64 len = 0;
    do {
        char buffer[4*1024] = {0};
        //读取文件内容
        len = this->file.read(buffer, sizeof(buffer));
        //发送
        len = this->tcpSocket->write(buffer, len);
        //记录已发送大小
        this->sendSize += len;
        //通知主线程更新进度条
        emit updateProgress(this->sendSize/1024);
    } while (len > 0);

    connect(this->tcpSocket, &QTcpSocket::readyRead, [=](){
        QByteArray receiveArray = this->tcpSocket->readAll();
        if (QString(receiveArray) == "File done") {
            //关闭文件
            this->file.close();
            //文件发送完成提示信息
            emit sendFileFinished();
            //关闭socket
            this->tcpSocket->disconnectFromHost();
            this->tcpSocket->close();
        }
    });
}

void MThread::tcpConnect(QTcpSocket *socket)
{
    this->tcpSocket = socket;

    //向主线程返回socket信息
    emit getSocketMessage(this->tcpSocket->peerAddress().toString(), this->tcpSocket->peerPort());

    connect(this->tcpSocket, &QTcpSocket::disconnected, this, [=](){
        emit statusDisconnect();
    });
}

void MThread::openFile(QString filePath)
{
    this->sendSize = 0;
    //获取文件信息
    QFileInfo fileInfo(filePath);
    this->fileName = fileInfo.fileName();
    this->fileSize = fileInfo.size();
    //通知主线程初始化进度条
    emit initProgress(this->fileSize/1024);
    //打开文件
    this->file.setFileName(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;
}

void MThread::sendFile()
{
    //先发送文件信息
    QString fileMessage = QString("%1##%2").arg(this->fileName).arg(this->fileSize);//客户端按指定格式解包

    //发送
    qint64 len = this->tcpSocket->write(fileMessage.toUtf8().data());
    if (len <= 0) {
        qDebug() << ("文件信息发送失败");
        this->tcpSocket->disconnectFromHost();
        this->tcpSocket->close();
        this->file.close();
    }

    //启动定时器
    this->mtimer.start(20);
}

void MThread::run()
{
    if (this->tcpSocket == nullptr)
        this->tcpSocket = new QTcpSocket;

    connect(&this->mtimer, &QTimer::timeout, [=](){
        //关闭定时器
        this->mtimer.stop();
        //发送文件
        this->startSendFile();
    });

    this->exec();
}
