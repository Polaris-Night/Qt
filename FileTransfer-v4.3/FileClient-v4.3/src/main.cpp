#include "mainwindow.h"
#include "titlebar.h"
#include "mlogmanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // qInstallMessageHandler(MLogManager::outputMessage);
    // MLogManager::setLogFile("clientLog.log");
    // atexit(MLogManager::spaceLine);

    QApplication a(argc, argv);
    TitleBar titleBar;
    MainWindow w;
    titleBar.setMainWidget(&w);
    titleBar.setTitleBarText("文件传输-v4.3   、生活几何｀ 制作");
    titleBar.setTitleBarIcon(":/transfer.png");
    titleBar.show();
    return a.exec();
}
