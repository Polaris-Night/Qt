#ifndef FILESERVER_H
#define FILESERVER_H

#include <QMainWindow>
#include <QLabel>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFileDialog>
#include <QString>
#include <QThread>
#include <QMessageBox>
#include <QTimer>

#include "sendfile.h"
#include "mtcpserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FileServer; }
QT_END_NAMESPACE

class FileServer : public QMainWindow
{
    Q_OBJECT

public:
    FileServer(QWidget *parent = nullptr);
    ~FileServer();

signals:
    void connectClient(qintptr socket);
    void openFile(QString filePath);
    void startToSend();
    void startSendFile();
    void disconnectClient();

private:
    Ui::FileServer *ui;

    MTcpServer *tcpServer;//监听
    QThread *mthread;//子线程
    SendFile *msendfile;//子线程工作类

    QLabel *listenLable;//监听标签
    QLabel *socketLable;//socket状态标签

    QTimer mtimer;//定时器
};
#endif // FILESERVER_H
