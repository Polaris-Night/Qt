#ifndef MWORK_H
#define MWORK_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTimer>
#include <QEventLoop>

#include "mthread.h"

class MWork : public QObject
{
    Q_OBJECT
public:
    explicit MWork(QString ip, quint16 port, QObject *parent = nullptr);
    ~MWork();

public slots:
    void toConnect();
    void toDisconnect();
    void toSendFile(QStringList fileList);

private:
    MThread *thread;//线程
    QTcpSocket *tcpSocket;//socket
    QTimer *mtimer;
    QEventLoop loop;
    QString ip;//ip
    quint16 port;//端口

    QFile file;
    QFileInfo info;
    QString fileName;

    qint64 fileSize;
    qint64 sendSize;

    int fileCount;
    int sendCount;
    int progress;
    int tempProgress;

signals:
    void goToConnect();
    void connectResult(QString result);
    void overConnect();
    void updateProgress(int progress);
    void sendFinish();
};

#endif // MWORK_H
