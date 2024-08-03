#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThreadPool>
#include <QTcpSocket>

#include "worker.h"
#include "sender.h"

namespace {
const int maxThread = 20;
const int blockCount = 4;
} // namespace

/***********************************工作线程***************************************/
Worker::Worker(QObject *parent)
: QObject(parent) { }

Worker::Worker(const QString &ip, const quint16 &port, QObject *parent)
: QObject(parent)
, m_ip(ip)
, m_port(port) { }

Worker::~Worker() { }

void Worker::setIp(const QString &ip) { m_ip = ip; }

void Worker::setPort(quint16 port) { m_port = port; }

QString Worker::getIp() const { return m_ip; }

quint16 Worker::getPort() const { return m_port; }

Worker::State Worker::getState() const { return m_state; }

QString Worker::getSpeed() { return m_speed.getSpeedSize(); }

void Worker::sendFile(const QStringList &fileList) {
    // 异步调用，即回到事件循环后，由Qt底层队列来调用，以达到此函数异步不阻塞的目的
    QMetaObject::invokeMethod(this, "_send", Qt::QueuedConnection, Q_ARG(QStringList, fileList));
}

void Worker::_send(const QStringList &fileList) {
    emit sendProgressChanged(0);
    if (fileList.isEmpty()) {
        return;
    }
    auto pool = QThreadPool::globalInstance();
    if (pool->maxThreadCount() != ::maxThread) {
        pool->setMaxThreadCount(::maxThread);
    }
    m_progress.clear();
    m_speed.clear();
    setState(State::Sending);
    // 遍历文件列表，将每个文件分割为4份，每份创建一个任务交给线程池执行
    for (auto &file : fileList) {
        auto blockList = FileSpliter::split(file, ::blockCount);
        if (!blockList.isEmpty()) {
            m_progress.addTotalSize(blockList.first().fileSize);
        }
        while (!blockList.isEmpty()) {
            auto block = blockList.takeFirst();
            Sender *sender = new Sender();
            sender->setHost(m_ip, m_port);
            sender->setBlock(std::move(block));
            connect(sender, &Sender::sendSizeChanged, this, [this](qint64 size) {
                m_speed.addSize(size);
                m_progress.addSize(size);
                emit sendProgressChanged(m_progress.getProgress());
                if (m_progress.isFinished()) {
                    setState(State::Inactive);
                    emit sendFinish();
                }
            });
            pool->start(sender);
        }
    }
}

void Worker::setState(State state) {
    if (m_state == state) {
        return;
    }
    m_state = state;
    emit stateChanged(m_state);
}
