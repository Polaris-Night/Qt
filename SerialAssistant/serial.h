#ifndef SERIAL_H
#define SERIAL_H

#include <QMainWindow>
#include <QString>
#include <QByteArray>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QLabel>
#include <QTimer>


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
    void searchSerial();

/*---------slots--------------*/
public slots:
    void openSerial();
    void sendData();
    void recvData();

/*---------variable-----------*/
private:
    Ui::Serial *ui;
    QSerialPort serialPort;
    QLabel *statusLabel;
    bool serialIsOpen;
};
#endif // SERIAL_H
