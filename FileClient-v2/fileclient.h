#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QMainWindow>
#include <QString>
#include <QMessageBox>
#include <QLabel>

#include "mthread.h"

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
    void startConnect(QString ip, qint16 port);

private:
    Ui::FileClient *ui;

    QFile file;//文件
    bool isFileInfo;//接收文件信息标志
    QString fileName;//文件名
    qint64 fileSize;//文件大小
    qint64 receiveSize;//已接收文件大小
    QString outputDirPath;//接收目录

    QLabel *statusLable;//状态栏标签

    MThread *mthread;//子线程
};
#endif // FILECLIENT_H
