#ifndef MMERGETHREAD_H
#define MMERGETHREAD_H

#include <QThread>
#include <QMap>
#include <QMutex>
#include <QSharedPointer>

#include "mfilemsgmanager.h"
#include "mprogress.h"

/**********************************合并文件线程**************************************/
class MMergeThread : public QThread
{
    Q_OBJECT
public:
    explicit MMergeThread(const QString saveDir, const BlockMsg &msg, QObject *parent = nullptr);
    ~MMergeThread();

    /**
     * @brief setTotalSize
     * @details 设置总文件大小
     * @param size 总文件大小
     */
    static void setTotalSize(const qint64 &size);

protected:
    void run() override;

private:
    /**
     * @brief calculateProgress
     * @details 计算更新总进度值，并发送更新总进度值信号
     * @param len 本次写入数据的大小
     */
    void calculateProgress(const qint64 &len);

    static QMutex mergeMutex;//合并线程互斥锁
    static int mergeProgress;//总进度值
    static MProgress prog;//进度计算
    //<文件名, 锁>
    static QMap<QString, QMutex *> lockMap;//互斥锁列表，同一文件的分块用同一个锁
    //<文件名，使用次数>
    static QMap<QString, int> lockUseCount;//使用次数记录表，记录同一文件的互斥锁使用次数

    QString dir;//保存路径
    BlockMsg blockMsg;//文件信息

signals:
    void updateMergeProgress(int mergeProgress);//更新总进度条信号
};
#endif // MMERGETHREAD_H
