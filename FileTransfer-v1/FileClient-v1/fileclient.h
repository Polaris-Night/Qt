#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QLabel>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui { class FileClient; }
QT_END_NAMESPACE

class FileClient : public QMainWindow
{
    Q_OBJECT

public:
    FileClient(QWidget *parent = nullptr);
    ~FileClient();
    void tcpInit();

public slots:
    void tcpConnect();
    void tcpRead();

private:
    Ui::FileClient *ui;

    QTcpSocket *tcpSocket;//通信socket
    QFile file;//文件
    bool isFileInfo;//接收文件信息标志
    QString fileName;//文件名
    qint64 fileSize;//文件大小
    qint64 receiveSize;//已接收文件大小
    QString outputDirPath;//接收目录

    QLabel *statusLable;
};
#endif // FILECLIENT_H
