#include "fileserver.h"
#include "ui_fileserver.h"

FileServer::FileServer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FileServer)
{
    ui->setupUi(this);
    this->setWindowTitle("文件传输Server-v2.0");
    this->setWindowIcon(QIcon(QString("://transfer.png")));
    this->statusLableListen = new QLabel(this);
    this->statusLableSocket = new QLabel(this);
    ui->statusbar->addWidget(this->statusLableListen);
    ui->statusbar->addWidget(this->statusLableSocket);

    //初始化按钮为不可用
    ui->buttonOpen->setEnabled(false);
    ui->buttonSend->setEnabled(false);

    this->tcpServer = new QTcpServer(this);
    this->mthread = new MThread;

    //点击监听
    connect(ui->buttonListen, &QPushButton::clicked, this, &FileServer::starttListen);

    //有用户连接
    connect(this->tcpServer, &QTcpServer::newConnection, this, [=](){
        //启动子线程
        this->mthread->start();
        //发送信号给子线程
        emit startConnect(this->tcpServer->nextPendingConnection());
    });
    //子线程收到主线程信号，进行socket连接
    connect(this, &FileServer::startConnect, this->mthread, &MThread::tcpConnect);
    //主线程收到子线程信号，接收子线程返回的数据
    connect(this->mthread, &MThread::getSocketMessage, this, [=](QString ip, quint16 port){
        ui->textEdit->setText(QString("[%1:%2]已连接").arg(ip).arg(port));
        ui->buttonOpen->setEnabled(true);
        this->statusLableSocket->setText(QString("正在与[%1]通信").arg(ip));
    });

    //监听socket状态
    connect(this->mthread, &MThread::statusDisconnect, this, [=](){
        ui->buttonSend->setEnabled(false);
        ui->buttonOpen->setEnabled(false);
        ui->textEdit->append(QString("已断开连接"));
        this->statusLableSocket->setText(QString());
    });

    //选择文件
    connect(ui->buttonOpen, &QPushButton::clicked, this, [=](){
        QString filePath = QFileDialog::getOpenFileName(this, "打开", "../");
        if (filePath.isEmpty())
            return;
        ui->textEdit->append(filePath);
        ui->buttonSend->setEnabled(true);
        ui->buttonOpen->setEnabled(false);
        emit fileOpen(filePath);
    });
    //子线程收到选择文件信号
    connect(this, &FileServer::fileOpen, this->mthread, &MThread::openFile);

    //初始化进度条
    connect(this->mthread, &MThread::initProgress, this, [=](int maximum){
        ui->progressBar->setRange(0, maximum);
        ui->progressBar->setValue(0);
    });

    //更新进度条
    connect(this->mthread, &MThread::updateProgress, this, [=](int progress){
        ui->progressBar->setValue(progress);
    });

    //发送文件
    connect(ui->buttonSend, &QPushButton::clicked, this, [=](){
        ui->textEdit->append(QString("正在发送文件..."));
        emit startSendFile();
    });
    //子线程收到发送文件信号
    connect(this, &FileServer::startSendFile, this->mthread, &MThread::sendFile);

    //发送完成信号
    connect(this->mthread, &MThread::sendFileFinished, this, [=](){
        ui->textEdit->append(QString("文件发送完成"));
        QMessageBox::information(this, "信息", "文件发送完成");
    });
}

FileServer::~FileServer()
{
    if (this->mthread != nullptr) {
        this->mthread->quit();
        this->mthread->wait();
        delete this->mthread;
        this->mthread = nullptr;
    }
    delete ui;
}

void FileServer::starttListen()
{
    this->tcpServer->close();
    int port = ui->comboBox->currentText().toInt();
    this->tcpServer->listen(QHostAddress::Any, port);
    this->statusLableListen->setText(QString("正在监听[%1]").arg(port));
}

