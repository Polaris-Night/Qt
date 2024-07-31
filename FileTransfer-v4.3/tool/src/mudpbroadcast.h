#ifndef MUDPBROADCAST_H
#define MUDPBROADCAST_H

#include <QObject>
#include <QSet>

class QUdpSocket;

enum SendFlag {
    Join = 2,
    Leave = 4
};

class MUdpBroadcast : public QObject
{
    Q_OBJECT
public:
    explicit MUdpBroadcast(const quint16 &broadcastPort, QObject *parent = nullptr);
    ~MUdpBroadcast();

    /**
     * @brief setBroadcastPort 设置广播端口
     * @param broadcastPort 广播端口
     */
    void setBroadcastPort(const quint16 &broadcastPort);

public slots:
    /**
     * @brief sendBroadcast 向所有网络接口发送广播
     * @param flag Join--发送加入广播；Leave--发送退出广播
     */
    void sendBroadcast(SendFlag flag = Join);

private:
    QUdpSocket *sock;
    quint16 port;
    QSet<QString> addressList;

private slots:
    void slotRead();

signals:
    void addressChange(QStringList addressList);
};

#endif // MUDPBROADCAST_H
