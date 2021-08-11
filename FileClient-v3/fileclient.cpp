#include "fileclient.h"
#include "ui_fileclient.h"

FileClient::FileClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FileClient)
{
    ui->setupUi(this);
    //窗口信息
    this->setWindowTitle(QString("文件传输Client-v3.0"));
    this->setWindowIcon(QIcon(QString("://transfer.png")));
    socketLabel = new QLabel(this);
    ui->statusbar->addWidget(socketLabel);
    ui->buttonDisconnect->setEnabled(false);
    ui->lcdNumber->setSegmentStyle(QLCDNumber::Flat);

    //调试用
    ui->lineEditIP->setText(QString("127.0.0.1"));

    //创建线程对象
    mthread = new QThread;//不可指定父类
    //创建任务对象
    recvFile = new RecvFile;
    //工作对象移动到子线程
    recvFile->moveToThread(mthread);

    //连接服务端
    connect(ui->buttonConnect, &QPushButton::clicked, this, [=](){
        //启动子线程
        mthread->start();
        emit startConnectServer(ui->lineEditIP->text(), ui->comboBox->currentText().toInt());
    });
    connect(this, &FileClient::startConnectServer, recvFile, &RecvFile::connectServer);

    //处理子线程发送的连接结果信号
    connect(recvFile, &RecvFile::connectResult, this, [=](QString result){
        if (result.at(1) == "E") {
            ui->textEdit->setText(result);
            return;
        }
        secTime = 0;//初始化计时
        ui->buttonDisconnect->setEnabled(true);//使能断开连接按钮
        ui->lcdNumber->display(secTime);
        ui->progressBar->setValue(0);//初始化进度条
        ui->textEdit->setText(result);//显示连接结果
        socketLabel->setText(QString("正在与[%1]通信").arg(result.mid(result.indexOf("[")+1, result.indexOf(":")-result.indexOf("[")-1)));//设置状态栏
    });
    //处理子线程发送的初始化进度条信号
    connect(recvFile, &RecvFile::initProgress, this, [=](int maximum){
        ui->progressBar->setRange(0, maximum);//设置进度条范围
        ui->textEdit->append(QString("正在接收文件..."));//提示信息
        mtimer.start(1000);//开始计时
    });
    //处理子线程发送的更新进度条信号
    connect(recvFile, &RecvFile::updateProgress, this, [=](int progress){
        ui->progressBar->setValue(progress);//更新进度条
    });
    //处理子线程发送的文件接收完成信号
    connect(recvFile, &RecvFile::finishRecv, this, [=](QString fileSavePath){
        ui->textEdit->append(QString("文件接收完成"));
        ui->textEdit->append(QString("文件保存路径:%1").arg(fileSavePath));
        QMessageBox::information(this, "信息", "文件接收完成");
    });

    //定时器超时
    connect(&mtimer, &QTimer::timeout, this, [=](){
        ui->lcdNumber->display(++secTime);
    });

    //传输速率
    connect(recvFile, &RecvFile::getSpeedSize, this, [=](qint64 size){
        double speed = size/8.0;//字节
        if (speed < 1024) { //小于1KB
            ui->speedLabel->setText(QString("%1B/s").arg(QString::number(speed, 'f', 2)));
        } else if (speed < 1024*1024) {//小于1MB
            speed /= 1024;
            ui->speedLabel->setText(QString("%1KB/s").arg(QString::number(speed, 'f', 2)));
        }
        else {//大于1MB
            speed /= 1024*1024;
            ui->speedLabel->setText(QString("%1MB/s").arg(QString::number(speed, 'f', 2)));
        }
    });

    //主动断开连接
    connect(ui->buttonDisconnect, &QPushButton::clicked, this, [=](){
        emit disconnectServer();
    });
    connect(this, &FileClient::disconnectServer, recvFile, &RecvFile::disconnectServer);

    //连接断开
    connect(recvFile, &RecvFile::connectOver, this, [=](){
        //退出子线程
        mthread->exit();
        mthread->wait();
        mtimer.stop();//停止计时
        ui->speedLabel->setText(QString("0B/s"));
        ui->buttonDisconnect->setEnabled(false);
        ui->textEdit->append(QString("已断开连接"));
        socketLabel->setText(QString());
    });

    ui->buttonConnect->setShortcut(Qt::Key_Enter);
}

FileClient::~FileClient()
{
    mthread->exit();
    mthread->wait();
    recvFile->deleteLater();
    mthread->deleteLater();
    delete ui;
}

