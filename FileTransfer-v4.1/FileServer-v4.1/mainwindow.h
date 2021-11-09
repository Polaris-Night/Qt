#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include <QDir>
#include <QMap>
#include <QFile>
#include <QMenu>
#include <QTimer>
#include <QLabel>
#include <QAction>
#include <QFileDialog>
#include <QReadWriteLock>
#include <QListWidgetItem>
#include <QDesktopServices>

#include "mserver.h"
#include "mwork.h"

/************************************主线程***************************************/
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString speedToString(double &speed);//速度值转带单位的字符串

public slots:
    void readyConnect(qintptr socket);//连接客户端

private:
    Ui::MainWindow *ui;

    QTimer mtimer;//定时器
    MServer *server;//服务端监听socket
    QLabel *statusLabel;//状态栏标签

signals:
    void goToDisconnectThis(QListWidgetItem *delItem);//断开选中项目连接信号
    void toDisconnect();//主动断开连接信号
    void goToDisconnectAll();//断开所有连接信号
    void sendSaveDir(QString saveDir);//发送保存目录信号
};

/**********************************合并文件线程**************************************/
class MergeThread : public QThread
{
    Q_OBJECT
public:
    explicit MergeThread(const QString saveDir, const FileMsg &msg, QObject *parent = nullptr);
    ~MergeThread();

    void calculateProgress(const qint64 &len);//计算更新总进度条

    static qint64 totalSize;//全部文件总大小
    static qint64 totalFinishedSize;//已完成总大小
    static int totalProgress;//总进度值
    static int totalTempProgress;//临时总进度值
    static QReadWriteLock rwLock;//读写锁
    //<文件名, 锁>
    static QMap<QString, QReadWriteLock*> lockMap;//读写锁列表，同一文件的分块用同一个锁

protected:
    void run() override;

private:
    QString saveDir;//保存路径
    FileMsg msg;//文件信息

signals:
    void updateTotalProgress(int totalProgress);//更新总进度条信号
};
#endif // MAINWINDOW_H
