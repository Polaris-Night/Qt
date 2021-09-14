#ifndef MWORK_H
#define MWORK_H

#include <QObject>
#include <QFile>
#include <QTcpSocket>
#include <QReadWriteLock>

#include "mthread.h"

/*********************************分块文件信息类**************************************/
class FileMsg
{
public:
    FileMsg() {
        fileName.clear();
        fileSize = blockSize = 0;
        block = 0;
    }
    FileMsg(const QString &fileName, const qint64 &fileSize, const int &block, const qint64 &blockSize) {
        this->fileName = fileName;
        this->fileSize = fileSize;
        this->block = block;
        this->blockSize = blockSize;
    }
    FileMsg(const FileMsg &msg) {
        fileName = msg.fileName;
        fileSize = msg.fileSize;
        block = msg.block;
        blockSize = msg.blockSize;
    }
    FileMsg& operator=(const FileMsg &msg) {
        fileName = msg.fileName;
        fileSize = msg.fileSize;
        block = msg.block;
        blockSize = msg.blockSize;
        return *this;
    }
    bool isEmpty() {
        return fileName.isEmpty();
    }

    QString fileName;//文件名
    qint64 fileSize;//文件总大小
    int block;//分块编号
    qint64 blockSize;//分块大小
};
Q_DECLARE_METATYPE(FileMsg)

/**********************************接收文件线程**************************************/
class MWork : public QObject
{
    Q_OBJECT
public:
    explicit MWork(const qintptr &socket, QObject *parent = nullptr);
    ~MWork();

    FileMsg parseFileMsg(QByteArray &msgByteArray);//解析文件信息
    void setSaveDir(QString &sDir);//设置保存目录
    bool createFile(const QString &fileName, const int &block, const qint64 &fileSize);//创建文件
    void calculateProgress(qint64 &len);//计算更新总进度条

    static QReadWriteLock rwLock;//读写锁
    static qint64 totalSize;//全部文件总大小
    static qint64 totalRecvSize;//总接收大小
    static int tempProgress;//临时进度值
    static int progress;//进度值

public slots:
    void startConnect();//创建socket，设置socket描述符
    void recvFile();//接收文件
    void toDisconnectThis();//断开此线程socket连接
    void toDisconnectAll();//断开所有线程socket连接

private:
    MThread *thread;//线程
    QTcpSocket *tcpSocket;//socket
    qintptr socket;//socket描述符
    QString ip;//地址
    quint16 port;//端口
    QFile file;//文件
    QString dir;//保存目录
    FileMsg msg;//文件信息
    qint64 recvSize;//接收大小
    bool startRecv;//开始接收标志

signals:
    void toConnect();//连接信号
    void socketMsg(QString ip, quint16 port);//发送socket信息信号
    void updateTotalSize(qint64 size);//更新全部文件总大小信号
    void overConnect();//结束连接信号
    void updateProgress(int progress);//更新进度条信号
    void fileFinished(FileMsg msg);//文件接收完成信号
};

#endif // MWORK_H
