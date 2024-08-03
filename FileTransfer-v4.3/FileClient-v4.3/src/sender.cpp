#include "sender.h"
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QDebug>

Sender::Sender(QObject *parent)
: QObject{parent} { }

void Sender::setHost(const QString &ip, quint16 port) {
    m_ip = ip;
    m_port = port;
}

void Sender::setBlock(const FileBlock &block) { m_block = block; }

void Sender::setBlock(FileBlock &&block) { m_block = std::move(block); }

void Sender::run() {
    // 1.连接
    if (!connectHost()) {
        return;
    }
    // 2.发送文件信息
    if (!sendHeadInfo()) {
        disconnectHost();
        return;
    }
    // 3.发送文件
    sendFile();
    emit finished();
}

bool Sender::connectHost() {
    if (m_socket.isNull()) {
        m_socket.reset(new QTcpSocket());
    }
    m_socket->connectToHost(m_ip, m_port);
    if (!m_socket->waitForConnected(3000)) {
        // 连接超时
        qWarning() << "connect timeout" << m_socket->errorString();
        return false;
    }
    connect(m_socket.get(), &QTcpSocket::disconnected, m_socket.get(), &QTcpSocket::deleteLater);
    return true;
}

void Sender::disconnectHost() { m_socket->disconnectFromHost(); }

bool Sender::sendHeadInfo() {
    FileBlock block(m_block);
    block.fileName = QFileInfo(block.fileName).fileName(); // 原来为文件路径，传输时只需保留文件名
    Message<FileBlock> request(0, "request", block);
    QByteArray headData = request.toJson();
    if (m_socket->write(headData) != headData.size()) {
        return false;
    }
    if (!m_socket->waitForBytesWritten() || !m_socket->waitForReadyRead()) {
        return false;
    }
    Message<void> response;
    response.fromJson(m_socket->readAll());
    return response.code == 0;
}

void Sender::sendFile() {
    QFile file(m_block.fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "open file error" << file.errorString();
        return;
    }
    file.seek(m_block.blockStartAddress);
    auto blockSize = m_block.blockSize;
    QDataStream stream(&file);
    qint64 len = 0;
    char buffer[4096];
    const qint64 bufferSize = sizeof(buffer);
    do {
        memset(buffer, 0, bufferSize);
        len = stream.readRawData(buffer, blockSize < bufferSize ? blockSize : bufferSize);
        len = m_socket->write(buffer, len);
        m_socket->waitForBytesWritten();
        blockSize -= len;
        emit sendSizeChanged(len);
    } while (len > 0 && blockSize > 0);
}
