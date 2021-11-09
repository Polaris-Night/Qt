#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QFileDialog>
#include <QStringList>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QTimer>

#include "mwork.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void toConnect();

private:
    Ui::MainWindow *ui;

    QStringList fileList;

    QTimer mtimer;

    QLabel *statusLabel;

    bool isConnect;

signals:
    void goToDisconnect();
    void goToSendFile(QStringList fileList);
};
#endif // MAINWINDOW_H
