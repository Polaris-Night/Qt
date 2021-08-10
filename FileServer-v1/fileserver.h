#ifndef FILESERVER_H
#define FILESERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include <QMessageBox>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class FileServer; }
QT_END_NAMESPACE

class FileServer : public QMainWindow
{
    Q_OBJECT

public:
    FileServer(QWidget *parent = nullptr);
    ~FileServer();
    void sendFile();

public slots:
    void portListen();
    void tcpConnect();
    void fileOpen();
    void fileSend();

private:
    Ui::FileServer *ui;
    QTcpServer *tcpServer;//监听socket
    QTcpSocket *tcpSocket;//通信socket

    QString fileName;//文件名
    qint64 fileSize;//文件大小
    qint64 sendSize;//已发送文件大小

    QFile file;//文件
    QTimer mtimer;//定时器

    QLabel *statusLableListen;//状态栏监听标签
    QLabel *statusLableSocket;//状态栏socket标签
};
#endif // FILESERVER_H
