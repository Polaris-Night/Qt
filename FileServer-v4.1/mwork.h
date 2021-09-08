#ifndef MWORK_H
#define MWORK_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTcpSocket>
#include <QJsonObject>
#include <QHostAddress>
#include <QReadWriteLock>
#include <QJsonParseError>

#include "mthread.h"

struct FileMsg
{
    QString fileName;//文件名
    qint64 fileSize;//文件总大小
    int block;//分块编号
    qint64 blockSize;//分块大小
};

/**********************************接收文件线程**************************************/
class MWork : public QObject
{
    Q_OBJECT
public:
    explicit MWork(const qintptr &socket, QObject *parent = nullptr);
    ~MWork();

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
    QTcpSocket *tcpSocket;//socket

    qintptr socket;//socket描述符

    MThread *thread;//线程

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
    void readySaveDir();//已获取保存目录信号
    void updateProgress(int progress);//更新进度条信号
    void fileFinish(FileMsg msg);//文件接收完成信号
};

#endif // MWORK_H
