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
    //创建线程
    mthread = new MThread;
    //标志初始化
    serialIsOpen = false;
    //信号和槽初始化
    connectInit();
    //启动线程定时搜索串口
    mthread->start();
    //创建定时器
    mtimer = new QTimer(this);
}

Serial::~Serial()
{
    //退出并销毁线程
    mthread->exit();
    mthread->wait();
    mthread->deleteLater();
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
    connect(mthread, &MThread::newSerial, this, [=](QString port){
        //过滤重复串口
        int i;
        for (i = 0; i < ui->comboBoxCOM->count(); i++) {
            if (ui->comboBoxCOM->itemText(i) == port)
                break;
        }
        //添加新串口
        if (i == ui->comboBoxCOM->count())
            ui->comboBoxCOM->addItem(port);
    });
    //手动搜索串口
    connect(ui->searchCOM, &QAction::triggered, mthread, &MThread::searchSerial);
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
    //定时发送
    connect(ui->checkBoxTiming, &QCheckBox::stateChanged, this, [=](){
        if (serialIsOpen) {
            if (ui->checkBoxTiming->checkState() == Qt::Checked) {
                connect(mtimer, &QTimer::timeout, this, &Serial::sendData);
                mtimer->start(ui->lineEditMs->text().toInt());
            } else {
                mtimer->stop();
                disconnect(mtimer, &QTimer::timeout, this, &Serial::sendData);
            }
        }
    });
}

/**************************************
 *
 *         slots
 *
**************************************/
void Serial::openSerial()
{
    //设置端口
    QString comName = ui->comboBoxCOM->currentText();
    comName = comName.mid(0, comName.indexOf(" "));
    serialPort.setPortName(comName);
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
    statusLabel->setText(QString("%1已连接").arg(comName));
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
