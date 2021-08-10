#include "fileclient.h"
#include "ui_fileclient.h"
#include <QDebug>

FileClient::FileClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FileClient)
{
    ui->setupUi(this);
    this->setWindowTitle(QString("文件传输Client-v2.0"));
    this->setWindowIcon(QIcon(QString("://transfer.png")));
    this->statusLable = new QLabel(this);
    ui->statusbar->addWidget(this->statusLable);

    //创建子线程
    this->mthread = new MThread;

    //启动子线程
    connect(ui->buttonConnect, &QPushButton::clicked, this, [=](){
        //给子线程发送信号进行初始化
        emit startConnect(ui->lineEditIP->text(), ui->lineEditPort->text().toInt());
        //启动子线程执行run函数
        mthread->start();
    });
    //子线程收到主线程信号，进行初始化
    connect(this, &FileClient::startConnect, mthread, &MThread::initTcpSocket);
    //主线程收到子线程信号，接收返回的数据
    connect(mthread, &MThread::sendSocketResult, this, [=](QString connectResult){
        if (connectResult.at(1) == "E") {
            //接收返回的错误
            ui->textEdit->setText(connectResult);
            return;
        }
        ui->progressBar->setValue(0);
        ui->textEdit->setText(QString("%1已连接").arg(connectResult.mid(connectResult.indexOf("["), connectResult.indexOf("]")-connectResult.indexOf("[")+1)));
        this->statusLable->setText(connectResult);
    });

    //监控socket状态
    connect(mthread, &MThread::statusDisconnect, this, [=](){
        //结束本次线程，以开始新连接
        this->mthread->exit();
        this->mthread->wait();
        this->statusLable->setText(QString());
        ui->textEdit->append(QString("已断开连接"));
    });

    //初始化进度条
    connect(this->mthread, &MThread::initProgress, this, [=](int maximum){
        ui->progressBar->setRange(0, maximum);
        ui->progressBar->setValue(0);
        ui->textEdit->append(QString("正在接收文件..."));
    });

    //更新进度条
    connect(this->mthread, &MThread::updateProgress, this, [=](int progress){
        ui->progressBar->setValue(progress);
    });

    //接收完成提示信息
    connect(this->mthread, &MThread::finishProgress, this, [=](QString fileSavePath){
        ui->textEdit->append(QString("文件接收完成"));
        ui->textEdit->append(QString("保存路径:%1").arg(fileSavePath));
        QMessageBox::information(this, "信息", "文件接收完成");
    });

    ui->buttonConnect->setShortcut(Qt::Key_Enter);
}

FileClient::~FileClient()
{
    if (this->mthread != nullptr) {
        this->mthread->quit();
        this->mthread->wait();
        this->mthread->deleteLater();
        this->mthread = nullptr;
    }
    delete ui;
}

