#include "mainwindow.h"
#include "mlogmanager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<FileMsg>("FileMsg");
    qInstallMessageHandler(MLogManager::outputMessage);
    atexit(MLogManager::spaceLine);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
