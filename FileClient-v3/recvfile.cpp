#include "recvfile.h"
#include <QDebug>

RecvFile::RecvFile(QObject *parent) : QObject(parent)
{
    tcpSocket = nullptr;
    fileName.clear();
    fileSize = 0;
    readSize = 0;
    outputDirPath = "./output";
    isFileMessage = true;
}

void RecvFile::connectServer(QString ip, quint16 port)
{
    tcpSocket = new QTcpSocket;

    //开始连接
    tcpSocket->connectToHost(ip, port);

    //连接失败
    if (!tcpSocket->waitForConnected(2000)) {
        tcpSocket->close();
        tcpSocket->deleteLater();
        emit connectResult(QString("[Error]%1").arg(tcpSocket->errorString()));
        return;
    }

    //连接成功
    emit connectResult(QString("[%1:%2]已连接").arg(tcpSocket->peerName()).arg(tcpSocket->peerPort()));

    //接收文件
    connect(tcpSocket, &QTcpSocket::readyRead, this, &RecvFile::receiveFile);

    //连接断开
    connect(tcpSocket, &QTcpSocket::disconnected, this, [=](){
        tcpSocket->close();
        tcpSocket->deleteLater();
        emit connectOver();
    });
}

void RecvFile::disconnectServer()
{
    tcpSocket->disconnectFromHost();
}

void RecvFile::receiveFile()
{
    //读取socket数据
    QByteArray receiveArray = tcpSocket->readAll();
    int tempProgress;

    //处理文件信息
    if (isFileMessage) {
        //解包文件信息
        fileName = QString(receiveArray).section("##", 0, 0);
        fileSize = QString(receiveArray).section("##", 1, 1).toInt();
        readSize = 0;
        //判断存储目录是否存在，不存在则在工作目录创建一个output目录
        QDir dir;
        if (!dir.exists(outputDirPath))
            dir.mkdir(outputDirPath);

        //创建文件
        file.setFileName(QString("%1/%2").arg(outputDirPath).arg(fileName));
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << file.errorString();
            disconnectServer();
            isFileMessage = true;
            return;
        }

        isFileMessage = false;
        //通知主线程初始化进度条
        emit initProgress(100);
        //emit initProgress(fileSize/1024);
    } else {
        //处理文件内容
        qint64 len = file.write(receiveArray);
        readSize += len;
        //通知主线程更新进度条
        tempProgress = static_cast<double>(readSize)/static_cast<double>(fileSize)*100;
        if (tempProgress % 1 == 0 && tempProgress != progress) {
            progress = tempProgress;
            emit updateProgress(progress);
        }
        //emit updateProgress(readSize/1024);
        //写入文件大小与总文件大小相同时结束写入
        if (readSize == fileSize) {
            //给服务端发送消息结束通信
            tcpSocket->write(QByteArray("File done"));
            //断开连接
            disconnectServer();
            //关闭文件
            file.close();
            isFileMessage = true;
            //通知主线程文件接收完成，返回存储路径
            QDir dir;
            emit finishRecv(QString("%1/output/%2").arg(dir.absolutePath()).arg(fileName));
        }
    }

}
