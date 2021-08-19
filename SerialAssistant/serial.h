#ifndef SERIAL_H
#define SERIAL_H

#include <QMainWindow>
#include <QString>
#include <QByteArray>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QLabel>
#include <QTimer>

#include "mthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Serial; }
QT_END_NAMESPACE

class Serial : public QMainWindow
{
    Q_OBJECT

/*---------function-----------*/
public:
    Serial(QWidget *parent = nullptr);
    ~Serial();
    void connectInit();

/*---------slots--------------*/
public slots:
    void openSerial();
    void sendData();
    void recvData();

/*---------variable-----------*/
private:
    Ui::Serial *ui;
    QSerialPort serialPort; //串口
    QLabel *statusLabel; //状态栏标签
    bool serialIsOpen; //串口标志
    MThread *mthread; //线程
    QTimer *mtimer;
};
#endif // SERIAL_H
