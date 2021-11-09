#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<FileMsg>("FileMsg");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
