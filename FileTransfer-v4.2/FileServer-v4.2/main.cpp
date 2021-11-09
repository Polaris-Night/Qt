#include "mainwindow.h"
#include "titlebar.h"
#include "mlogmanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<FileMsg>("FileMsg");
    qInstallMessageHandler(MLogManager::outputMessage);
    atexit(MLogManager::spaceLine);

    QApplication a(argc, argv);
    TitleBar titleBar;
    MainWindow w;
    titleBar.setMainWidget(&w);
    titleBar.setTitleBarText("文件传输-v4.2");
    titleBar.setTitleBarIcon(":/transfer.png");
    titleBar.show();
    return a.exec();
}
