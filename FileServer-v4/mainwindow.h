#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QAction>
#include <QMenu>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>

#include "mserver.h"
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
    void readyConnect(qintptr socket);

private:
    Ui::MainWindow *ui;

    MServer *server;

    QString saveDir;

    QLabel *statusLabel;

signals:
    void goToDisconnectThis(QListWidgetItem *delItem);
    void goToDisconnectAll();
    void toDisconnect();
    void sendSaveDir(QString saveDir);
};
#endif // MAINWINDOW_H
