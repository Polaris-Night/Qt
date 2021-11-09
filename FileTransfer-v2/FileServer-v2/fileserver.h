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

#include "mthread.h"

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

signals:
    void startConnect(QTcpSocket *socket);
    void fileOpen(QString filePath);
    void startSendFile();

public slots:
    void starttListen();

private:
    Ui::FileServer *ui;

    QTcpServer *tcpServer;//监听socket

    QLabel *statusLableListen;//状态栏监听标签
    QLabel *statusLableSocket;//状态栏socket标签

    MThread *mthread;//子线程
};
#endif // FILESERVER_H
