#ifndef MWORK_H
#define MWORK_H

#include <QObject>
#include <QFile>
#include <QTcpSocket>
#include <QMutex>
#include <QReadWriteLock>
#include <QThread>
#include <QDataStream>

#include "mfilemsgmanager.h"
#include "mprogress.h"
#include "mspeed.h"

/**********************************接收文件线程**************************************/
class MRecvFileThread : public QObject
{
    Q_OBJECT
public:
    explicit MRecvFileThread(const qintptr &socket, QObject *parent = nullptr);
    ~MRecvFileThread();

    /**
     * @brief getSpeedSize
     * @details 定时调用该函数，获取单位时间内所有线程接收的大小
     * @return 单位时间内所有线程接收的大小
     */
    static QString getSpeedSize();

    static QReadWriteLock rwLock;//线程读写锁

private:
    /**
     * @brief The MThread class
     * @details 线程类，MWork依赖于该类执行
     */
    class MThread : public QThread
    {
    public:
        explicit MThread(QObject *parent = nullptr) : QThread(parent) {}
        ~MThread() {}

    protected:
        void run() override
        {
            connect(this, &MThread::finished, this, &MThread::deleteLater);
            exec();
        }
    };

    /**
     * @brief parseFileMsg
     * @details 解析分块文件信息，并返回分块文件信息对象
     * @param msgByteArray 待解析的数据
     * @return 分块文件信息对象
     */
    BlockMsg parseFileMsg(const QByteArray &msgByteArray);

    /**
     * @brief getSaveDir
     * @details 获取文件保存目录
     */
    void getSaveDir();

    /**
     * @brief createFile
     * @details 创建文件
     * @return 成功返回true，否则返回false
     */
    bool createFile();

    /**
     * @brief calculateProgress
     * @details 计算更新进度值，并发送更新进度值信号
     * @param len 本次接收数据的大小
     */
    void calculateSpeedAndProgress(const qint64 &len);

    static QMutex threadMutex;//线程互斥锁
    static int recvProgress;//进度值
    static bool allStart;//开始接收标志
    static MProgress prog;//进度计算
    static MSpeed speed;//速度计算

    MThread *thread;//线程
    QTcpSocket *tcpSocket;//socket
    qintptr socketFd;//socket描述符

    QString ip;//地址
    quint16 port;//端口

    QDataStream wStream;//文件数据流
    QFile file;//文件
    QString dir;//保存目录
    BlockMsg msg;//文件信息
    qint64 recvSize;//接收大小
    bool isFileMsg;//文件信息标志

private slots:
    /**
     * @brief startConnect
     * @details 创建socket，并发送带有socket信息的信号
     */
    void startConnect();

    /**
     * @brief toDisconnect
     * @details 断开socket连接
     */
    void toDisconnect();

    /**
     * @brief recvFile
     * @details 有数据，接收文件数据
     */
    void recvFile();

signals:
    void toConnect();//连接信号
    void socketMsg(const QString &msg);//发送socket信息信号
    void updateTotalSize(const qint64 totalSize);//更新全部文件总大小信号
    void overConnect();//结束连接信号
    void updateRecvProgress(int recvProgress);//更新进度条信号
    void finishRecv(BlockMsg msg);//文件接收完成信号
    void goToDisconnect();
};

#endif // MWORK_H
