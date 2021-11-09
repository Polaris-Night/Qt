#ifndef SENDFILE_H
#define SENDFILE_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QHostAddress>

class SendFile : public QObject
{
    Q_OBJECT
public:
    explicit SendFile(QObject *parent = nullptr);

signals:
    void connectSocketMessage(QString ip, quint16 port);
    void fileMessageHadSend();
    void connectOver();
    void finishSend();
    void updateProgress(int progress);

public slots:
    void connectClient(qintptr socket);
    void openFile(QString filePath);
    void startToSend();
    void startSendFile();
    void disconnectClient();

private:
    QTcpSocket *tcpSocket;//通信socket
    QFile file;//文件
    QString fileName;//文件名
    qint64 fileSize;//文件大小
    qint64 sendSize;//已发送文件大小
    int progress;//发送进度
};

#endif // SENDFILE_H
