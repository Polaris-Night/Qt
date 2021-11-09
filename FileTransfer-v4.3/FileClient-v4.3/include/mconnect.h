#ifndef MCONNECT_H
#define MCONNECT_H

#include <QObject>
#include <QTcpSocket>
#include <QStringList>
#include <QVector>
#include <QThread>
#include <QMutex>

#include "mfilemsgmanager.h"

/**********************************工作线程类***************************************/
class MConnect : public QObject
{
    Q_OBJECT
public:
    explicit MConnect(const QString &ip, const quint16 &port, QObject *parent = nullptr);
    ~MConnect();

    /**
     * @brief getFileMsg
     * @details 获取文件信息
     * @param fileList 文件列表
     * @param blockCount 分块数量
     * @return 文件信息管理对象
     */
    MFileMsgManager getFileMsg(const QStringList &fileList, const int &blockCount);

public slots:
    /**
     * @brief toConnect
     * @details 连接服务端
     */
    void toConnect();

    /**
     * @brief toDisconnect
     * @details 断开与服务端的连接
     */
    void toDisconnect();

    /**
     * @brief toSendFile
     * @details 发送文件
     * @param fileList 文件列表
     */
    void toSendFile(const QStringList &fileList);

private:
    /**
     * @brief The MThread class
     * @details 线程类，MWork对象依赖于该类执行
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

    MThread *thread;//线程
    QTcpSocket *tcpSocket;//socket
    QString ip;//ip
    quint16 port;//端口
    qint64 totalSize;//全部文件总大小

signals:
    void goToConnect();//连接信号
    void connectResult(QString result);//连接结果信号
    void overConnect();//结束连接信号
    void updateSendProgress(int sendProgress);//更新进度条信号
    void sendFinish();//全部文件发送完成信号
};
#endif // MWORK_H
