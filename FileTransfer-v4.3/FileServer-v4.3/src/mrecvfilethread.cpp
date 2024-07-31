#include <QDebug>
#include <QDir>
#include <QJsonObject>
#include <QHostAddress>
#include <QJsonParseError>
#include <QTextCodec>

#include "mrecvfilethread.h"

extern QString saveDir; //全局保存目录

QReadWriteLock MRecvFileThread::rwLock; //线程读写锁
QMutex MRecvFileThread::threadMutex;    //线程互斥锁
int MRecvFileThread::recvProgress = 0;  //进度值
bool MRecvFileThread::allStart = false; //开始接收标志
MProgress MRecvFileThread::prog;        //进度计算
MSpeed MRecvFileThread::speed;          //速度计算

MRecvFileThread::MRecvFileThread(const qintptr &socket, QObject *parent)
: QObject(parent)
, m_fd(socket) {
    //创建线程
    m_thread = new QThread;
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    //work依赖thread线程执行
    this->moveToThread(m_thread);
    connect(this, &MRecvFileThread::toConnect, this, &MRecvFileThread::_connect);
    connect(this, &MRecvFileThread::goToDisconnect, this, &MRecvFileThread::_disconnect);
    //数据流
    m_stream.setDevice(&m_file);
    m_stream.setVersion(QDataStream::Qt_5_12);
    //启动线程
    m_thread->start();
}

MRecvFileThread::~MRecvFileThread() {
    if (m_socket != nullptr) {
        m_socket->close();
        m_socket->deleteLater();
    }
    m_thread->quit();
}

QString MRecvFileThread::getSpeedSize() {
    QMutexLocker locker(&threadMutex);
    return speed.getSpeedSize();
}

void MRecvFileThread::_disconnect() {
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

void MRecvFileThread::getSaveDir() {
    //获取保存目录
    rwLock.lockForRead(); //加读锁
    m_dir = saveDir;
    rwLock.unlock(); //解读锁
    //判断存储目录是否存在，不存在则创建
    if (!QDir().exists(m_dir))
        QDir().mkdir(m_dir);
}

bool MRecvFileThread::createFile() {
    m_file.setFileName(QString("%1/%2.00%3").arg(m_dir).arg(m_block.fileName).arg(m_block.blockIndex + 1));
    if (!m_file.open(QIODevice::WriteOnly)) {
        qWarning("open file %s error:%s", qUtf8Printable(m_block.fileName), qUtf8Printable(m_file.errorString()));
        return false;
    }
    //预设文件大小，防止存储空间不足
    m_file.resize(m_block.blockSize);
    return true;
}

void MRecvFileThread::calculateSpeedAndProgress(const qint64 &len) {
    QMutexLocker locker(&threadMutex);
    //统计速度
    speed.addSize(len);
    //统计总接收大小
    prog.addSize(len);
    //计算进度值
    int tempRecvProgress = prog.getProgress();
    //进度值不变时解锁并返回
    if (tempRecvProgress == recvProgress)
        return;
    //更新进度值
    recvProgress = tempRecvProgress;
    emit updateRecvProgress(recvProgress);
    if (recvProgress < 100)
        return;
    //重置
    recvProgress = 0;
    speed.clear();
    prog.clear();
    allStart = false;
}

void MRecvFileThread::_connect() {
    //创建socket
    m_socket = new QTcpSocket;
    //设置socket描述符
    m_socket->setSocketDescriptor(m_fd);
    m_ip = QHostAddress(m_socket->peerAddress().toIPv4Address()).toString();
    m_port = m_socket->peerPort();
    //发送socket信息
    emit socketMsg(QString("[%1:%2]").arg(m_ip).arg(m_port));

    //连接断开
    connect(m_socket, &QTcpSocket::disconnected, this, &MRecvFileThread::overConnect);
    //有数据
    connect(m_socket, &QTcpSocket::readyRead, this, &MRecvFileThread::recvFile);
}

void MRecvFileThread::recvFile() {
    QByteArray recvArray = m_socket->readAll();

    if (m_isHeadInfo) {
        Message<FileBlock> request;
        request.fromJson(recvArray);
        Message<void> response(0, "response");
        QByteArray responseData = response.toJson();
        m_socket->write(responseData);
        m_socket->waitForBytesWritten();

        //获取文件信息
        m_block = std::move(request.data);
        if (m_block.fileName.isEmpty()) {
            qWarning("file message is empty");
            m_socket->disconnectFromHost();
            return;
        }
        //每个文件由第一分块记录总大小并发送更新进度条信号
        if (m_block.blockIndex == 0) {
            QMutexLocker locker(&threadMutex);
            prog.setTotalSize(prog.getTotalSize() + m_block.fileSize);
            emit updateTotalSize(m_block.fileSize);
            if (!allStart) {
                allStart = true;
                emit updateRecvProgress(recvProgress);
            }
        }
        //获取保存目录
        getSaveDir();
        //创建文件
        if (!createFile())
            return;
        //若块大小为0，则直接返回
        if (m_block.blockSize == 0) {
            m_file.close();
            emit finishRecv(BlockMsg(m_block.fileName, m_block.fileSize, m_block.blockIndex, m_block.blockSize));
            qDebug() << QString("receive file %1.00%2 finished").arg(m_block.fileName).arg(m_block.blockIndex + 1);
            return;
        }
        m_isHeadInfo = false;
    } else {
        //写入文件
        qint64 len = m_stream.writeRawData(recvArray, recvArray.size());
        m_recvSize += len;
        //计算速度及进度值
        calculateSpeedAndProgress(len);
        //单个文件接收完成
        if (m_recvSize == m_block.blockSize) {
            m_file.close();
            m_isHeadInfo = true;
            //返回保存文件名
            emit finishRecv(BlockMsg(m_block.fileName, m_block.fileSize, m_block.blockIndex, m_block.blockSize));
            qDebug() << QString("receive file %1.00%2 finished").arg(m_block.fileName).arg(m_block.blockIndex + 1);
        }
    }
}
