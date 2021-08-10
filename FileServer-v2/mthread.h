#ifndef MTHREAD_H
#define MTHREAD_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QFileInfo>
#include <QFile>
#include <QTimer>

class MThread : public QThread
{
    Q_OBJECT
public:
    explicit MThread(QObject *parent = nullptr);
    ~MThread();
    void startSendFile();

protected:
    void run() override;

signals:
    void getSocketMessage(QString ip, qint16 port);
    void statusDisconnect();
    void sendFileFinished();
    void initProgress(int maximum);
    void updateProgress(int progress);

public slots:
    void tcpConnect(QTcpSocket *socket);
    void openFile(QString filePath);
    void sendFile();

private:
    QTcpSocket *tcpSocket;//通信socket
    qint16 port;//监听端口号

    QString fileName;//文件名
    qint64 fileSize;//文件大小
    qint64 sendSize;//已发送文件大小

    QFile file;//文件
    QTimer mtimer;//定时器
};

#endif // MTHREAD_H
