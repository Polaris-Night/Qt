#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDir>
#include <QDesktopServices>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLabel *splitLabel;
    QLabel *mergeLabel;

    QString splitFileName;//分割的文件名
    QStringList mergeFileList;//合并的文件列表
    QString saveDir;//保存路径
};
#endif // MAINWINDOW_H
