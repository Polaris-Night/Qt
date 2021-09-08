#ifndef MWORK_H
#define MWORK_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QFileInfo>
#include <QTcpSocket>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVector>

#include "mthread.h"

/*********************************分块文件信息结构体************************************/
struct BlockMsg
{
    QString fileName;//文件名
    qint64 fileSize;//文件总大小
    QString filePath;//文件路径
    int block;//分块编号
    qint64 blockSize;//分块大小
};
Q_DECLARE_METATYPE(BlockMsg)
/*********************************全文件信息结构体*************************************/
struct Msg
{
    QString fileName;//文件名
    qint64 fileSize;//文件总大小
    QString filePath;//文件路径
    QVector<qint64> blockSize;//分块文件大小
};
Q_DECLARE_METATYPE(Msg)
/*********************************全文件信息类***************************************/
class FileMsg
{
public:
    FileMsg() {
        fileCount = 0;
        blockCount = 0;
    }
    FileMsg(const int &FileCount, const int &BlockCount) {
        fileCount = FileCount;
        blockCount = BlockCount;
        for (int i = 0; i < fileCount; i++) {
            fileMsg.push_back(Msg());
            for (int j = 0; j < blockCount; j++) {
                qint64 size = 0;
                fileMsg[i].blockSize.push_back(size);
            }
        }
    }
    FileMsg(const FileMsg &msg) {
        fileCount = msg.fileCount;
        blockCount = msg.blockCount;
        fileMsg = msg.fileMsg;
        for (int i = 0; i < fileCount; i++) {
            fileMsg[i].fileName = msg.fileMsg[i].fileName;
            fileMsg[i].fileSize = msg.fileMsg[i].fileSize;
            fileMsg[i].filePath = msg.fileMsg[i].filePath;
            for (int j = 0; j < blockCount; j++)
                fileMsg[i].blockSize[j] = msg.fileMsg[i].blockSize[j];
        }
    }
    ~FileMsg() {}
    FileMsg& operator=(const FileMsg &msg) {
        fileCount = msg.fileCount;
        blockCount = msg.blockCount;
        fileMsg = msg.fileMsg;
        for (int i = 0; i < fileCount; i++) {
            fileMsg[i].fileName = msg.fileMsg[i].fileName;
            fileMsg[i].fileSize = msg.fileMsg[i].fileSize;
            fileMsg[i].filePath = msg.fileMsg[i].filePath;
            for (int j = 0; j < blockCount; j++)
                fileMsg[i].blockSize[j] = msg.fileMsg[i].blockSize[j];
        }
        return *this;
    }

    int fileCount;//文件数量
    QVector<Msg> fileMsg;//文件信息
    int blockCount;//分块数量
};
Q_DECLARE_METATYPE(FileMsg)

/**********************************工作线程类***************************************/
class MWork : public QObject
{
    Q_OBJECT
public:
    explicit MWork(const QString &ip, const quint16 &port, QObject *parent = nullptr);
    ~MWork();

    FileMsg getFileMsg(const QStringList &fileList, const int &block);

public slots:
    void toConnect();//连接
    void toDisconnect();//主动断开连接
    void toSendFile(const QStringList &fileList);//发送文件

private:
    MThread *thread;//线程
    QTcpSocket *tcpSocket;//socket
    QString ip;//ip
    quint16 port;//端口

    qint64 totalSize;//全部文件总大小
    qint64 totalSendSize;//已发送文件大小

    int progress;//进度值
    int tempProgress;//临时进度值

signals:
    void goToConnect();//连接信号
    void connectResult(QString result);//连接结果信号
    void overConnect();//结束连接信号
    void updateProgress(int progress);//更新进度条信号
    void sendFinish();//全部文件发送完成信号
};

/********************************发送文件工作线程**************************************/
class SendFileThread : public QThread
{
    Q_OBJECT
public:
    explicit SendFileThread(const QString &ip, const quint16 &port, QObject *parent = nullptr);
    ~SendFileThread();

    void setFileMeg(const QString &fileName, const qint64 &fileSize, const QString &filePath, const int &block, const qint64 &blockSize);//设置文件信息

protected:
    void run() override;

private:
    QString ip;//IP
    quint16 port;//端口

    BlockMsg msg;//分块文件信息

signals:
    void partSendSize(qint64 sendSize);//分块已发送大小信号
};


#endif // MWORK_H
