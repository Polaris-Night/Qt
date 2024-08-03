#include "receiver.h"
#include <QHostAddress>
#include <QFile>
#include <QDataStream>
#include <QDebug>

#include "lockmanager.h"

namespace {
const qint64 bufferSize = 8192;
} // namespace

Receiver::Receiver(QObject *parent)
: QObject{parent} { }

void Receiver::setFd(const qintptr &fd) { m_fd = fd; }

void Receiver::setDir(const QString &dir) { m_dir = dir; }

void Receiver::disconnectClient() {
    if (m_socket.isNull()) {
        return;
    }
    m_socket->disconnectFromHost();
}

void Receiver::run() {
    // 1.连接
    connectClient();
    // 2.接收文件信息
    if (!recvHeadInfo()) {
        return;
    }
    // 3.接收文件
    recvFile();
    emit receiveFinished();
    disconnectClient();
    // 4.合并文件
    mergeFile();
    emit mergeFinished();
}

void Receiver::connectClient() {
    if (m_socket.isNull()) {
        m_socket.reset(new QTcpSocket());
    }
    m_socket->setSocketDescriptor(m_fd);
    QString ip = QHostAddress(m_socket->peerAddress().toIPv4Address()).toString();
    QString port = QString::number(m_socket->peerPort());
    m_hostPort = QString("[%1:%2]").arg(ip).arg(port);
    //连接断开
    connect(m_socket.get(), &QTcpSocket::disconnected, m_socket.get(), &QTcpSocket::deleteLater);
    connect(m_socket.get(), &QTcpSocket::disconnected, this, [this]() {
        emit clientDisconnected(m_hostPort, static_cast<qint64>(m_fd));
    });
    emit clientConnected(m_hostPort, static_cast<qint64>(m_fd));
}

bool Receiver::recvHeadInfo() {
    // 间隔至多1秒等待文件信息数据
    while (!m_socket->waitForReadyRead(1000) && !isDisconnected()) {
    }
    if (isDisconnected()) {
        return false;
    }

    QByteArray data = m_socket->readAll();
    Message<FileBlock> request;
    request.fromJson(data);
    Message<void> response(0, "read head");
    m_socket->write(response.toJson());
    m_socket->waitForBytesWritten();

    // 文件名为空时直接结束
    m_block = std::move(request.data);
    if (m_block.fileName.isEmpty()) {
        m_socket->disconnectFromHost();
        return false;
    }
    // 块大小为0时无需处理
    if (m_block.blockSize == 0) {
        emit receiveFinished();
        return true;
    }
    emit fileSizeChanged(m_block.fileName, m_block.fileSize);
    return true;
}

void Receiver::recvFile() {
    // 创建文件，并预设文件大小，防止存储空间不足
    QFile file(getBlockPath());
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    file.resize(m_block.blockSize);
    QDataStream stream(&file);
    auto blockSize = 0;
    do {
        // 间隔至多1秒等待数据读取
        if (m_socket->waitForReadyRead(1000)) {
            QByteArray data = m_socket->readAll();
            qint64 len = stream.writeRawData(data, data.size());
            blockSize += len;
            emit receiveSizeChanged(len);
        }
    } while (blockSize < m_block.blockSize && !isDisconnected());
    file.flush();
}

// TOOD: 将合并操作挪到外部，因为多线程合并一个文件有竞争问题导致效率低，且会导致此工作线程未结束，无法处理其他网络请求，单线程合并文件更快
void Receiver::mergeFile() {
    // 写操作为独占式，不可多个线程同时写入同一文件，因此以文件名作为锁的索引
    auto lock = LockManager::instance().getLock(m_block.fileName);
    QMutexLocker locker(lock.get());

    QFile file(getFilePath());
    QFile blockFile(getBlockPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append) || !blockFile.open(QIODevice::ReadOnly)) {
        return;
    };
    qDebug() << "start merge" << m_block.fileName << "block" << m_block.blockIndex;
    // 原始文件偏移到块起始地址，读取块文件数据并写入原始文件
    file.seek(m_block.blockStartAddress);
    QDataStream wStream(&file);
    QDataStream rStream(&blockFile);
    qint64 len = 0;
    char buffer[::bufferSize];
    do {
        memset(buffer, 0, ::bufferSize);
        len = rStream.readRawData(buffer, ::bufferSize);
        len = wStream.writeRawData(buffer, len);
        emit mergeSizeChanged(len);
    } while (len > 0);
    file.flush();
    qDebug() << "end merge" << m_block.fileName << "block" << m_block.blockIndex;
    // 完成后删除块文件
    blockFile.remove();
}

bool Receiver::isDisconnected() const { return m_socket->state() == QAbstractSocket::UnconnectedState; }

QString Receiver::getBlockPath() const {
    return QString("%1/%2.00%3").arg(m_dir).arg(m_block.fileName).arg(m_block.blockIndex);
}

QString Receiver::getFilePath() const { return QString("%1/%2").arg(m_dir).arg(m_block.fileName); }
