#include "mainwindow.h"
#include "titlebar.h"
#include "mlogmanager.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    qInstallMessageHandler(MLogManager::outputMessage);
    MLogManager::setLogFile("serverLog.log");
    atexit(MLogManager::spaceLine);

    QApplication a(argc, argv);
    a.setApplicationName("fileserver-v4.3");
    TitleBar titleBar;
    MainWindow w;
    titleBar.setMainWidget(&w);
    titleBar.setTitleBarText("文件传输-v4.3   、生活几何｀ 制作");
    titleBar.setTitleBarIcon(":/transfer.png");
    titleBar.show();

    return a.exec();
}
