#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QStringList>
#include <QMessageBox>

#include "mwork.h"

/***********************************主线程****************************************/
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void msetStyleSheet(const QString styleSheet);
    QString loadHelpText();

public slots:
    void toConnect();//连接服务端

private:
    Ui::MainWindow *ui;

    QMessageBox *helpBox;//帮助信息对话框
    QStringList fileList;//文件列表
    QTimer mtimer;//定时器
    QLabel *statusLabel;//状态栏标签
    bool isConnect;//连接标志

signals:
    void goToDisconnect();//连接信号
    void goToSendFile(QStringList fileList);//发送文件信号
};
#endif // MAINWINDOW_H
