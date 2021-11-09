#include "sendfile.h"
#include <QDebug>

SendFile::SendFile(QObject *parent) : QObject(parent)
{

}

void SendFile::connectClient(qintptr socket)
{
    tcpSocket = new QTcpSocket;
    tcpSocket->setSocketDescriptor(socket);
    //向主进程返回socket信息
    emit connectSocketMessage(this->tcpSocket->peerAddress().toString(), this->tcpSocket->peerPort());
    //连接断开
    connect(tcpSocket, &QTcpSocket::disconnected, this, [=](){
        tcpSocket->close();
        tcpSocket->deleteLater();
        emit connectOver();
    });
}

void SendFile::openFile(QString filePath)
{
    //打开文件
    file.setFileName(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
        return;
    }
    //获取文件信息
    QFileInfo info(filePath);
    fileName = info.fileName();
    fileSize = info.size();
    sendSize = 0;
}

void SendFile::startToSend()
{
    //先发送文件信息
    QString message = QString("%1##%2").arg(fileName).arg(fileSize);
    qint64 len = tcpSocket->write(QByteArray(message.toUtf8().data()));
    if (len <= 0) {
        qDebug() << "文件信息发送失败";
        file.close();
    }

    //文件信息发送成功，通知主线程启动定时器防止粘包
    emit fileMessageHadSend();
}

void SendFile::startSendFile()
{
    qint64 len = 0;
    int tempProgress = 0;
    progress = 0;
    char buffer[4*1024] = {0};
    do {
        //读取文件内容
        len = file.read(buffer, sizeof(buffer));
        //发送
        len = tcpSocket->write(buffer, len);
        tcpSocket->waitForBytesWritten();//等待数据发送出去，否则可能会造成内存溢出
        //记录已发送大小
        sendSize += len;
        //通知主线程更新进度条
        tempProgress = static_cast<double>(sendSize)/static_cast<double>(fileSize)*100;
        if (tempProgress != progress) {
            progress = tempProgress;
            emit updateProgress(progress);
        }

    } while (len > 0);

    //文件发送完毕
    connect(tcpSocket, &QTcpSocket::readyRead, this, [=](){
        QByteArray receiveArray = tcpSocket->readAll();
        if (QString(receiveArray) == "File done") {
            //通知主线程文件发送完成
            emit finishSend();
            //断开连接
            disconnectClient();
            //关闭文件
            file.close();
        }
    });
}

void SendFile::disconnectClient()
{
    tcpSocket->disconnectFromHost();
}

