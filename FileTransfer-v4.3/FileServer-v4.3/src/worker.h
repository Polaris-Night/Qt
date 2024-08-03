#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QSet>
#include <QMap>
#include <QPointer>

#include "mprogress.h"
#include "mspeed.h"
#include "receiver.h"

class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    Worker(const QString &dir, QObject *parent = nullptr);

    void setSaveDir(const QString &dir);
    void reset();

    QString getSpeed();

public slots:
    void appendClient(qintptr fd);
    /**
     * @brief 手动断开连接，通常无法生效，因为Receiver一直阻塞在run中未回到事件循环
     * @param fd
     */
    void disconnectClient(qintptr fd);

signals:
    void clientConnected(QString, qint64);
    void clientDisconnected(QString, qint64);
    void receiveProgressChanged(int);
    void receiveFinished();
    void mergeProgressChanged(int);
    void mergeFinished();

private:
    QString m_dir;
    MSpeed m_speed;
    MProgress m_receiveProgress;
    MProgress m_mergeProgress;
    QSet<QString> m_fileSet;
    QMap<qintptr, QPointer<Receiver>> m_watcher; // 监控Receiver生命周期，映射fd与Receiver的关系
};

#endif // WORKER_H
