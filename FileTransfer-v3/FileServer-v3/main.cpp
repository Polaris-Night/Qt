#include "fileserver.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<qintptr>("qintptr");
    FileServer w;
    w.show();
    return a.exec();
}
