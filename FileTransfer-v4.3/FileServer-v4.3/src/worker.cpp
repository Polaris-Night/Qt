#include "worker.h"
#include <QThreadPool>
#include <QDir>

namespace {
const int maxThread = 20;
} // namespace

Worker::Worker(QObject *parent)
: QObject{parent} { }

Worker::Worker(const QString &dir, QObject *parent)
: QObject(parent) {
    setSaveDir(dir);
}

void Worker::setSaveDir(const QString &dir) {
    if (!QDir(dir).exists()) {
        QDir().mkdir(dir);
    }
    m_dir = dir;
}

void Worker::reset() {
    m_speed.clear();
    m_receiveProgress.clear();
    m_mergeProgress.clear();
    m_fileSet.clear();
    m_watcher.clear();
}

QString Worker::getSpeed() { return m_speed.getSpeedSize(); }

void Worker::appendClient(qintptr fd) {
    auto *pool = QThreadPool::globalInstance();
    if (pool->maxThreadCount() != ::maxThread) {
        pool->setMaxThreadCount(::maxThread);
    }
    Receiver *receiver = new Receiver();
    receiver->setDir(m_dir);
    receiver->setFd(fd);
    m_watcher.insert(fd, receiver);
    connect(receiver, &Receiver::clientConnected, this, &Worker::clientConnected);
    connect(receiver, &Receiver::clientDisconnected, this, &Worker::clientDisconnected);
    connect(receiver, &Receiver::fileSizeChanged, this, [this](QString file, qint64 size) {
        if (m_fileSet.contains(file)) {
            return;
        }
        m_receiveProgress.addTotalSize(size);
        m_mergeProgress.addTotalSize(size);
        m_fileSet.insert(file);
    });
    connect(receiver, &Receiver::receiveSizeChanged, this, [this](qint64 size) {
        m_speed.addSize(size);
        m_receiveProgress.addSize(size);
        emit receiveProgressChanged(m_receiveProgress.getProgress());
        if (m_receiveProgress.isFinished()) {
            emit receiveFinished();
        }
    });
    connect(receiver, &Receiver::mergeSizeChanged, this, [this](qint64 size) {
        m_mergeProgress.addSize(size);
        emit mergeProgressChanged(m_mergeProgress.getProgress());
        if (m_mergeProgress.isFinished()) {
            reset();
            emit mergeFinished();
        }
    });
    pool->start(receiver);
}

void Worker::disconnectClient(qintptr fd) {
    if (!m_watcher.contains(fd) || m_watcher.value(fd).isNull()) {
        return;
    }
    // 暂不实现
}
