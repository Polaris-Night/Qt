#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThreadPool>

#include "worker.h"
#include "sender.h"

/***********************************工作线程***************************************/
Worker::Worker(const QString &ip, const quint16 &port, QObject *parent)
: QObject(parent)
, m_ip(ip)
, m_port(port) {
    m_thread = new QThread;
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    //work在thread线程执行
    this->moveToThread(m_thread);
    //启动线程
    m_thread->start();
}

Worker::~Worker() {
    if (m_socket != nullptr) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    m_thread->quit();
    m_thread->deleteLater();
}

void Worker::setIp(const QString &ip) { m_ip = ip; }

void Worker::setPort(quint16 port) { m_port = port; }

QString Worker::getIp() const { return m_ip; }

quint16 Worker::getPort() const { return m_port; }

QString Worker::getSpeed() { return m_speed.getSpeedSize(); }

void Worker::connectHost() {
    // 异步调用，即回到事件循环后，由Qt底层队列来调用，以达到跨线程调度工作
    QMetaObject::invokeMethod(this, "_connect", Qt::QueuedConnection);
}

void Worker::disconnectHost() {
    // 异步调用，即回到事件循环后，由Qt底层队列来调用，以达到跨线程调度工作
    QMetaObject::invokeMethod(this, "_disconnect", Qt::QueuedConnection);
}

void Worker::sendFile(const QStringList &fileList) {
    // 异步调用，即回到事件循环后，由Qt底层队列来调用，以达到跨线程调度工作
    QMetaObject::invokeMethod(this, "_send", Qt::QueuedConnection, Q_ARG(QStringList, fileList));
}

void Worker::_connect() {
    //创建socket
    m_socket = new QTcpSocket;
    //连接服务端
    m_socket->connectToHost(m_ip, m_port);
    //超时3s连接失败
    if (!m_socket->waitForConnected(3000)) {
        emit connectResult(QString("[Error]%1").arg(m_socket->errorString()));
        qWarning("connect [%s:%s] error:%s",
                 qUtf8Printable(m_ip),
                 qUtf8Printable(QString::number(m_port)),
                 qUtf8Printable(m_socket->errorString()));
        return;
    }
    //连接成功
    emit connectResult(QString("[%1:%2]已连接").arg(m_socket->peerName()).arg(m_socket->peerPort()));

    //连接断开
    connect(m_socket, &QTcpSocket::disconnected, this, &Worker::overConnect);
}

void Worker::_disconnect() {
    auto isDisconnect = [this]() { return m_socket->state() == QAbstractSocket::UnconnectedState; };
    if (!isDisconnect()) {
        int maxTimes = 3;
        do {
            m_socket->disconnectFromHost();
            if (!isDisconnect()) {
                m_socket->waitForDisconnected(1000);
            }
            maxTimes -= 1;
        } while (!isDisconnect() && maxTimes > 0);
    }
}

void Worker::_send(const QStringList &fileList) {
    emit updateSendProgress(0);
    auto pool = QThreadPool::globalInstance();
    int maxThread = 20;
    if (pool->maxThreadCount() != maxThread) {
        pool->setMaxThreadCount(maxThread);
    }
    m_progress.clear();
    m_speed.clear();
    // 遍历文件列表，将每个文件分割为4份，每份创建一个任务交给线程池执行
    for (auto &file : fileList) {
        auto blockList = FileSpliter::split(file, 4);
        if (!blockList.isEmpty()) {
            m_progress.addTotalSize(blockList.first().fileSize);
        }
        while (!blockList.isEmpty()) {
            auto block = blockList.takeFirst();
            Sender *worker = new Sender();
            worker->setHost(m_ip, m_port);
            worker->setBlock(std::move(block));
            connect(worker, &Sender::sizeSentChanged, this, [this](qint64 size) {
                m_speed.addSize(size);
                m_progress.addSize(size);
                emit updateSendProgress(m_progress.getProgress());
            });
            pool->start(worker);
        }
    }
}
