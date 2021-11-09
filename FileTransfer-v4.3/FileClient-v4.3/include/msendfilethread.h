#ifndef MSENDFILETHREAD_H
#define MSENDFILETHREAD_H

#include <QThread>
#include <QRunnable>
#include <QMutex>

#include "mfilemsgmanager.h"
#include "mprogress.h"
#include "mspeed.h"

class MSendFileThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit MSendFileThread(const QString &ip, const quint16 &port, const BlockMsg &blockMsg, QObject *parent = nullptr);
    ~MSendFileThread();

    /**
     * @brief setTotalSize
     * @details 设置总文件大小
     * @param size 总文件大小
     */
    static void setTotalSize(const qint64 &size);

    static QString getSpeedSize();

protected:
    /**
     * @brief run
     * @details 线程函数
     */
    void run() override;

private:
    /**
     * @brief calculateSendProgress
     * @details 计算进度值并发送更新信号
     * @param len
     */
    void calculateSpeedAndProgress(const qint64 &len);

    static QMutex mutex;//线程互斥锁
    static MProgress prog;//进度计算
    static MSpeed speed;//速度计算
    QString ip;//IP
    quint16 port;//端口
    BlockMsg msg;//分块文件信息

signals:
    void updateSendProgress(int progress);
    void sendFinish();
};
#endif // MSENDFILETHREAD_H
