#include "mthread.h"
#include <QDebug>

MThread::MThread(QObject *parent) : QThread(parent)
{

}

void MThread::run()
{
    QTimer mtimer;

    connect(&mtimer, &QTimer::timeout, this, &MThread::searchSerial);

    mtimer.start(2000);

    this->exec();
    mtimer.stop();
}

void MThread::searchSerial()
{
    //搜索串口
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QSerialPort serial;
        serial.setPort(info);
        //过滤掉不可用串口
        if(serial.open(QIODevice::ReadWrite)) {
            emit newSerial(QString("%1 %2").arg(serial.portName()).arg(info.description()));
            serial.close();
        }
    }
}
