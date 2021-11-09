#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QMainWindow>
#include <QLabel>
#include <QThread>
#include <QMessageBox>
#include <QTimer>

#include "recvfile.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FileClient; }
QT_END_NAMESPACE

class FileClient : public QMainWindow
{
    Q_OBJECT

public:
    FileClient(QWidget *parent = nullptr);
    ~FileClient();

signals:
    void startConnectServer(QString ip, quint16 port);
    void disconnectServer();

private:
    Ui::FileClient *ui;

    QLabel *socketLabel;//socket状态标签

    QThread *mthread;//子线程

    RecvFile *recvFile;//子线程工作对象

    QTimer mtimer;//定时器
    qint32 secTime;
};
#endif // FILECLIENT_H
