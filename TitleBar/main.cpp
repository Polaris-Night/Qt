#include "titlebar.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TitleBar w;//标题栏
    QWidget wid;//主窗口
    w.setMainWidget(&wid);//设置主窗口
    w.show();
    return a.exec();
}
