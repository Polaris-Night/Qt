#include "serial.h"
#include "ui_serial.h"
#include <QDebug>

Serial::Serial(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Serial)
{
    ui->setupUi(this);
    //窗口标题
    this->setWindowTitle(QString("串口助手"));
    //窗口图标
    this->setWindowIcon(QIcon(QString("://serial.png")));
    //状态栏
    statusLabel = new QLabel(this);
    ui->statusbar->addWidget(statusLabel);
    //标志初始化
    serialIsOpen = false;
    //信号和槽初始化
    connectInit();
    //搜索串口
    searchSerial();
}

Serial::~Serial()
{
    delete ui;
}

/**************************************
 *
 *         function
 *
**************************************/
void Serial::connectInit()
{
    //定时搜索串口
    connect(ui->searchCOM, &QAction::triggered, this, &Serial::searchSerial);
    //开关串口
    connect(ui->buttonSerial, &QPushButton::clicked, this, [=](){
        if (!serialIsOpen) {
            openSerial();
            if (serialIsOpen)
                ui->buttonSerial->setText(QString("关闭串口"));
        } else {
            serialIsOpen = false;
            serialPort.close();
            statusLabel->setText(QString());
            ui->buttonSerial->setText(QString("打开串口"));
        }
    });
    //串口发送
    connect(ui->buttonSend, &QPushButton::clicked, this, &Serial::sendData);
    //串口接收
    connect(&serialPort, &QSerialPort::readyRead, this, &Serial::recvData);
    //清空发送窗口
    connect(ui->buttonClearSend, &QPushButton::clicked, ui->textSend, &QTextEdit::clear);
    //清空接收窗口
    connect(ui->buttonClearRecv, &QPushButton::clicked, ui->textRecv, &QTextBrowser::clear);
}

void Serial::searchSerial()
{
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite)) {
            ui->comboBoxCOM->addItem(QString("%1 %2").arg(serial.portName()).arg(info.description()));
            serial.close();
        }
    }
}

/**************************************
 *
 *         slots
 *
**************************************/
void Serial::openSerial()
{
    //设置端口
    QString comInfo = ui->comboBoxCOM->currentText();
    comInfo = comInfo.mid(0, comInfo.indexOf(" "));
    serialPort.setPortName(comInfo);
    //设置波特率
    serialPort.setBaudRate(ui->comboBoxBaudrate->currentText().toInt());
    //设置数据位
    serialPort.setDataBits(static_cast<QSerialPort::DataBits>(ui->comboBoxDataBit->currentText().toInt()));
    //设置停止位
    if (ui->comboBoxStopBit->currentText().toInt() == 1.5)
        serialPort.setStopBits(QSerialPort::OneAndHalfStop);
    else
        serialPort.setStopBits(static_cast<QSerialPort::StopBits>(ui->comboBoxStopBit->currentText().toInt()));
    //设置校验位
    switch (ui->comboBoxParity->currentIndex()) {
        case 0:
            serialPort.setParity(QSerialPort::NoParity);
            break;
        case 1:
            serialPort.setParity(QSerialPort::EvenParity);
            break;
        case 2:
            serialPort.setParity(QSerialPort::OddParity);
            break;
    }
    //打开串口
    if (!serialPort.open(QIODevice::ReadWrite)) {
        qDebug() << serialPort.errorString();
        return;
    }
    statusLabel->setText(QString("%1已连接").arg(comInfo));
    //修改标志
    serialIsOpen = true;
}

void Serial::sendData()
{
    QString data(ui->textSend->toPlainText());
    if (ui->checkBoxHex->checkState() == Qt::Checked)
        serialPort.write(QByteArray(data.toUtf8().data()).toHex(' ').toUpper());
    else
        serialPort.write(QByteArray(data.toUtf8().data()));

}

void Serial::recvData()
{
    QByteArray recvArray = serialPort.readAll();
    ui->textRecv->append(QString(recvArray));
}
