#include "mudpbroadcast.h"
#include <QUdpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkInterface>

#define KEY_FLAG (QStringLiteral("flag"))

MUdpBroadcast::MUdpBroadcast(const quint16 &broadcastPort, QObject *parent)
    : QObject(parent), port(broadcastPort)
{
    sock = new QUdpSocket(this);
    sock->bind(QHostAddress::AnyIPv4, port, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    connect(sock, &QUdpSocket::readyRead, this, &MUdpBroadcast::slotRead);
}

MUdpBroadcast::~MUdpBroadcast()
{
    sendBroadcast(Leave);
}

void MUdpBroadcast::sendBroadcast(SendFlag flag)
{
    //封装Json数据
    QJsonObject obj;
    obj.insert(KEY_FLAG, flag);
    QByteArray sendArray = QJsonDocument(obj).toJson();

    //遍历网络接口
    QList<QNetworkInterface> networkinterfaces = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interfaces, networkinterfaces) {
        if (interfaces.flags().testFlag(QNetworkInterface::IsUp)
            && !interfaces.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            //遍历网络接口的广播地址
            foreach (QNetworkAddressEntry entry, interfaces.addressEntries()) {
              QHostAddress broadcastAddress = entry.broadcast();
              QHostAddress ip = entry.ip();
              //广播地址不为空，且IP为IPV4地址时发送广播
              if (!broadcastAddress.isNull()
                && QAbstractSocket::IPv4Protocol == ip.protocol())
                    sock->writeDatagram(sendArray, broadcastAddress, port);
            }
        }
    }
}

void MUdpBroadcast::setBroadcastPort(const quint16 &broadcastPort)
{
    port = broadcastPort;
}

void MUdpBroadcast::slotRead()
{
    QHostAddress peerAddress;
    quint16 peerPort = 0;
    QByteArray recvArray;
    bool hasChange = false;
    while (sock->hasPendingDatagrams()) {
        recvArray.clear();
        recvArray.resize(sock->pendingDatagramSize());
        //读取数据，获取地址及端口
        sock->readDatagram(recvArray.data(), recvArray.size(), &peerAddress, &peerPort);
        //地址转字符串
        QString ipStr = peerAddress.toString();
        //解析Json数据
        QJsonDocument doc = QJsonDocument::fromJson(recvArray);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            SendFlag flag = static_cast<SendFlag>(obj[KEY_FLAG].toInt());
            if (!addressList.contains(ipStr) && Join == flag) {
                hasChange = true;
                //发现新主机，加入列表，并广播
                addressList.insert(ipStr);
                sendBroadcast();
            } else if (Leave == flag) {
                hasChange = true;
                //主机退出，从列表中删除
                addressList.remove(ipStr);
            }
        }
    }

    if (hasChange)
        emit addressChange(addressList.values());
}
