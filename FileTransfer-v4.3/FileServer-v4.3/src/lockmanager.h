#ifndef LOCKMANAGER_H
#define LOCKMANAGER_H

#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include <QWeakPointer>

class LockManager {
public:
    static LockManager &instance();

    QSharedPointer<QMutex> getLock(const QString &key);

    void clearUnusedLock();

private:
    LockManager() = default;
    Q_DISABLE_COPY_MOVE(LockManager)
    void clearUnused(); // 内部封装一层不加锁的处理

    QMutex m_mutex;                                // 保护map
    QMap<QString, QWeakPointer<QMutex>> m_lockMap; // map中保留弱引用，避免外部没有锁引用后仍然不会销毁锁的问题
};

#endif // LOCKMANAGER_H
