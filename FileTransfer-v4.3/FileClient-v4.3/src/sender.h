#ifndef SENDER_H
#define SENDER_H

#include <QObject>
#include <QTcpSocket>
#include <QRunnable>
#include <QScopedPointer>

#include "filespliter.h"

class Sender : public QObject, public QRunnable {
    Q_OBJECT
public:
    explicit Sender(QObject *parent = nullptr);

    void setHost(const QString &ip, quint16 port);
    void setBlock(const FileBlock &block);
    void setBlock(FileBlock &&block);

signals:
    void sendSizeChanged(qint64);
    void finished();

protected:
    void run() override;

private:
    bool connectHost();
    void disconnectHost();
    bool sendHeadInfo();
    void sendFile();

    QScopedPointer<QTcpSocket> m_socket;
    QString m_ip;
    quint16 m_port{};
    FileBlock m_block;
};

#endif // SENDER_H
