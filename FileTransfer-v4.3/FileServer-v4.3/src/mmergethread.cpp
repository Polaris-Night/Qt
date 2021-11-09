#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QDataStream>

#include "mmergethread.h"

QMutex MMergeThread::mergeMutex;//合并线程互斥锁
int MMergeThread::mergeProgress = 0;//总进度值
MProgress MMergeThread::prog;//进度计算
QMap<QString, QMutex *> MMergeThread::lockMap;//互斥锁列表
QMap<QString, int> MMergeThread::lockUseCount;//使用次数记录表，记录同一文件的互斥锁使用次数

MMergeThread::MMergeThread(const QString saveDir, const BlockMsg &msg, QObject *parent)
    : QThread(parent), dir(std::move(saveDir)), blockMsg(msg)
{
    //每个文件创建一个读写锁
    if (lockMap.contains(msg.fileName))
        return;
    QMutex *lock = new QMutex;
    lockMap.insert(msg.fileName, lock);
    lockUseCount.insert(msg.fileName, 4);
}

MMergeThread::~MMergeThread()
{
    QMutexLocker locker(&mergeMutex);
    if ((--lockUseCount[blockMsg.fileName]) <= 0) {
        delete lockMap[blockMsg.fileName];
        lockMap.remove(blockMsg.fileName);
    }
}

void MMergeThread::setTotalSize(const qint64 &size)
{
    QMutexLocker locker(&mergeMutex);
    prog.setTotalSize(prog.getTotalSize()+size);
}

void MMergeThread::calculateProgress(const qint64 &len)
{
    QMutexLocker locker(&mergeMutex);
    prog.addSize(len);
    int tempMergeProgress = prog.getProgress();
    //进度值不变时解锁并返回
    if (tempMergeProgress == mergeProgress)
        return;
    //更新进度值
    mergeProgress = tempMergeProgress;
    emit updateMergeProgress(mergeProgress);
    if (mergeProgress < 100)
        return;
    //重置
    prog.clear();
    mergeProgress = 0;
}

void MMergeThread::run()
{
    //线程结束自动回收
    connect(this, &MMergeThread::finished, this, &MMergeThread::deleteLater);

    QFile blockFile(QString("%1/%2.00%3").arg(dir).arg(blockMsg.fileName).arg(blockMsg.block+1));
    QFile sourceFile(QString("%1/%2").arg(dir).arg(blockMsg.fileName));
    QDataStream rStream(&blockFile);
    QDataStream wStream(&sourceFile);
    rStream.setVersion(QDataStream::Qt_5_12);
    wStream.setVersion(QDataStream::Qt_5_12);
    if (!blockFile.open(QIODevice::ReadOnly)) {
        qWarning("open file %s error:%s", qUtf8Printable(QString("%1.00%2").arg(blockMsg.fileName).arg(blockMsg.block+1)), qUtf8Printable(blockFile.errorString()));
        return;
    }

    char buffer[4096];
    qint64 len = 0;
    qint64 offset;
    //计算偏移量
    offset = (blockMsg.block == 3) ? (blockMsg.fileSize - blockMsg.blockSize) : (blockMsg.blockSize * blockMsg.block);

    //读分块文件并写入源文件
    lockMap[blockMsg.fileName]->lock();//加锁
    qDebug() << QString("start merging block %1 of file %2").arg(blockMsg.block).arg(blockMsg.fileName);
    if (!sourceFile.open(QIODevice::Append | QIODevice::WriteOnly)) {
        //打开失败，使用QProcess处理后再次打开
        QProcess proc;
        proc.start(sourceFile.fileName(), QStringList(sourceFile.fileName()));
        proc.close();
        if (!sourceFile.open(QIODevice::Append | QIODevice::WriteOnly)) {
            lockMap[blockMsg.fileName]->unlock();//解写锁
            qWarning("open file %s error:%s", qUtf8Printable(blockMsg.fileName), qUtf8Printable(sourceFile.errorString()));
            blockFile.close();
            return;
        }
    }
    sourceFile.seek(offset);
    do {
        memset(buffer, 0, sizeof(buffer));
        len = rStream.readRawData(buffer, sizeof(buffer));
        len = wStream.writeRawData(buffer, len);
        //len = blockFile.read(buffer, sizeof(buffer));
        //len = sourceFile.write(buffer, len);
        sourceFile.flush();
        //计算更新总进度
        calculateProgress(len);
    } while (len > 0);
    sourceFile.close();
    qDebug() << QString("finish merging block %1 of file %2").arg(blockMsg.block).arg(blockMsg.fileName);
    lockMap[blockMsg.fileName]->unlock();//解锁

    //关闭并移除分块文件
    blockFile.close();
    blockFile.remove();
}
