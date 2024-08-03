#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>
#include <QRunnable>
#include <QTcpSocket>
#include <QScopedPointer>

#include "filespliter.h"

class Receiver : public QObject, public QRunnable {
    Q_OBJECT
public:
    explicit Receiver(QObject *parent = nullptr);

    void setFd(const qintptr &fd);
    void setDir(const QString &dir);

private slots:
    void disconnectClient();

signals:
    void clientConnected(QString, qint64);
    void clientDisconnected(QString, qint64);
    void fileSizeChanged(QString, qint64);
    void receiveSizeChanged(qint64);
    void receiveFinished();
    void mergeSizeChanged(qint64);
    void mergeFinished();

protected:
    void run() override;

private:
    void connectClient();
    bool recvHeadInfo();
    void recvFile();
    void mergeFile();

    bool isDisconnected() const;
    QString getBlockPath() const;
    QString getFilePath() const;

private:
    QScopedPointer<QTcpSocket> m_socket;
    qintptr m_fd;
    QString m_hostPort;
    QString m_dir;
    bool m_isHeadInfo{true};
    FileBlock m_block;
};

#endif // RECEIVER_H
