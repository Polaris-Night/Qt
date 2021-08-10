#ifndef SENDFILE_H
#define SENDFILE_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QDir>

class RecvFile : public QObject
{
    Q_OBJECT
public:
    explicit RecvFile(QObject *parent = nullptr);

signals:
    void connectResult(QString result);//返回连接结果
    void connectOver();//连接断开
    void initProgress(int maximum);//初始化进度条
    void updateProgress(int progress);//更新进度条
    void finishRecv(QString fileSavePath);//返回文件存储路径

public slots:
    //连接服务端
    void connectServer(QString ip, quint16 port);
    //取消连接
    void disconnectServer();
    //接收文件
    void receiveFile();

private:
    QTcpSocket *tcpSocket;//通信socket

    bool isFileMessage;//文件信息标志
    QString fileName;//文件名
    qint64 fileSize;//文件大小
    qint64 readSize;//已接受文件大小
    QFile file;//文件
    QString outputDirPath;//存储目录

    int progress;//进度值
};

#endif // SENDFILE_H
