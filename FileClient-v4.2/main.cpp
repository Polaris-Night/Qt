#include "mainwindow.h"
#include "mlogmanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(MLogManager::outputMessage);
    atexit(MLogManager::spaceLine);
    MainWindow w;
    w.show();
    return a.exec();
}
