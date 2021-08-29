#ifndef MWORK_H
#define MWORK_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QFile>
#include <QDir>
#include <QTimer>

#include "mthread.h"

class MWork : public QObject
{
    Q_OBJECT
public:
    explicit MWork(qintptr socket, QObject *parent = nullptr);
    ~MWork();

public slots:
    void startConnect();
    void toGetSaveDir(QString dir);
    void recvFile();
    void toDisconnectThis();
    void toDisconnectAll();

private:
    QTcpSocket *tcpSocket;
    qintptr socket;

    MThread *thread;//线程
    QTimer *mtimer;//定时器

    QFile file;
    QString fileName;
    QString saveDir;

    qint64 fileSize;
    qint64 recvSize;
    qint64 speedSize;

    int fileCount;
    int recvCount;
    int tempProgress;
    int progress;

    bool startRecv;
    bool isFileInfo;

signals:
    void toConnect();
    void socketMsg(QString ip, quint16 port);
    void overConnect();
    void goToGetSaveDir();
    void readSaveDir();
    void updateProgress(int progress);
    void updateSpeedSize(qint64 speedSize);
    void fileFinish(QString path);
};

#endif // MWORK_H
