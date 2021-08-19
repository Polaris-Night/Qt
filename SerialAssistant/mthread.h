#ifndef MTHREAD_H
#define MTHREAD_H

#include <QThread>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QTimer>

class MThread : public QThread
{
    Q_OBJECT
/*---------function-----------*/
public:
    explicit MThread(QObject *parent = nullptr);

protected:
    void run() override;

/*---------slots--------------*/
public slots:
    void searchSerial();

/*---------signals-----------*/
signals:
    void newSerial(QString serial);
};

#endif // MTHREAD_H
