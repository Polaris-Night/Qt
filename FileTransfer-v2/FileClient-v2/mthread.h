#ifndef MTHREAD_H
#define MTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>
#include <QFileInfo>
#include <QFile>
#include <QDir>

class MThread : public QThread
{
    Q_OBJECT
public:
    explicit MThread(QObject *parent = nullptr);
    ~MThread();
    void initTcpSocket(QString ip, qint16 port);

protected:
    void run() override;

signals:
    void sendSocketResult(QString connectResult);
    void statusDisconnect();
    void initProgress(int maximum);
    void updateProgress(int progress);
    void finishProgress(QString fileSavePath);

public slots:
    void readFile();

private:
    QTcpSocket *tcpSocket;
    QString ip;
    qint16 port;

    QFile file;//文件
    bool isFileInfo;//接收文件信息标志
    QString fileName;//文件名
    qint64 fileSize;//文件大小
    qint64 receiveSize;//已接收文件大小
    QString outputDirPath;//接收目录

};

#endif // MTHREAD_H
