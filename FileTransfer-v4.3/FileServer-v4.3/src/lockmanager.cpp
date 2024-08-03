#include "lockmanager.h"

LockManager &LockManager::instance() {
    static LockManager handler;
    return handler;
}

QSharedPointer<QMutex> LockManager::getLock(const QString &key) {
    QMutexLocker locker(&m_mutex);
    clearUnused();
    // 管理表中若不存在key对应的锁，将创建一个新的锁并返回
    // 管理表中存在key对应的锁则直接返回一个强引用
    if (!m_lockMap.contains(key) || m_lockMap.value(key).isNull()) {
        QSharedPointer<QMutex> strong(new QMutex());
        strong.reset(new QMutex());
        m_lockMap.insert(key, strong.toWeakRef());
        return strong;
    }
    return m_lockMap.value(key).toStrongRef();
}

void LockManager::clearUnusedLock() {
    QMutexLocker locker(&m_mutex);
    clearUnused();
}

void LockManager::clearUnused() {
    auto iter = m_lockMap.begin();
    while (iter != m_lockMap.end()) {
        if (iter->isNull()) {
            iter = m_lockMap.erase(iter);
        } else {
            ++iter;
        }
    }
}
