#include "fileserver.h"
#include "ui_fileserver.h"

FileServer::FileServer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FileServer)
{
    ui->setupUi(this);
    this->setWindowTitle(QString("文件传输Server-v3.0"));
    this->setWindowIcon(QIcon(QString("://transfer.png")));
    listenLable = new QLabel(this);
    socketLable = new QLabel(this);
    ui->statusbar->addWidget(listenLable);
    ui->statusbar->addWidget(socketLable);
    ui->buttonOpen->setEnabled(false);
    ui->buttonSend->setEnabled(false);
    ui->buttonDisconnect->setEnabled(false);

    //创建监听socket
    tcpServer = new MTcpServer(this);
    //创建线程对象
    mthread = new QThread;
    //创建工作对象
    msendfile = new SendFile;
    //工作对象移动到线程
    msendfile->moveToThread(mthread);

    //开始监听
    connect(ui->buttonListen, &QPushButton::clicked, this, [=](){
        tcpServer->close();
        quint16 port = ui->comboBox->currentText().toUShort();
        tcpServer->listen(QHostAddress::Any, port);
        listenLable->setText(QString("正在监听[%1]").arg(port));
    });

    //有客户端连接
    connect(tcpServer, &MTcpServer::haveNewConnect, this, [=](qintptr socket){
        //接收客户端socket
        //QTcpSocket *socket = tcpServer->nextPendingConnection();
        //启动线程
        mthread->start();
        //发送socket
        emit connectClient(socket);
        ui->buttonDisconnect->setEnabled(true);
        ui->buttonOpen->setEnabled(true);
    });
    connect(this, &FileServer::connectClient, msendfile, &SendFile::connectClient);

    //接收socket信息
    connect(msendfile, &SendFile::connectSocketMessage, this, [=](QString ip, quint16 port){
        ui->textEdit->setText(QString("[%1:%2]已连接").arg(ip).arg(port));
        socketLable->setText(QString("正在与[%1]通信").arg(ip));
    });

    //选择文件
    connect(ui->buttonOpen, &QPushButton::clicked, this, [=](){
        //获取文件路径
        QString filePath = QFileDialog::getOpenFileName(this, "打开", "../");
        if (filePath.isEmpty()) {
            qDebug() << "路径不合法";
            return;
        }
        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(0);
        ui->textEdit->append(filePath);
        ui->buttonSend->setEnabled(true);
        //给子线程文件路径信号
        emit openFile(filePath);
    });
    //子线程处理主线程的文件路径信号
    connect(this, &FileServer::openFile, msendfile, &SendFile::openFile);

    //发送文件
    connect(ui->buttonSend, &QPushButton::clicked, this, [=](){
        emit startToSend();
    });
    //子线程处理主线程的发送文件信号
    connect(this, &FileServer::startToSend, msendfile, &SendFile::startToSend);

    //文件信息发送完成
    connect(msendfile, &SendFile::fileMessageHadSend, this, [=](){
        ui->textEdit->append(QString("正在发送文件..."));
        mtimer.start(20);
    });
    //开始发送文件内容
    connect(&mtimer, &QTimer::timeout, this, [=](){
        mtimer.stop();
        emit startSendFile();
    });
    //子线程处理主线程的发送文件内容信号
    connect(this, &FileServer::startSendFile, msendfile, &SendFile::startSendFile);

    //更新进度条
    connect(msendfile, &SendFile::updateProgress, this, [=](int progress){
        ui->progressBar->setValue(progress);
    });

    //处理子线程发送的文件发送完成信号
    connect(msendfile, &SendFile::finishSend, this, [=](){
        ui->textEdit->append(QString("文件发送完成"));
        QMessageBox::information(this, "信息", "文件发送完成");
    });

    //主动断开连接
    connect(ui->buttonDisconnect, &QPushButton::clicked, this, [=](){
        emit disconnectClient();
    });
    //子线程处理主线程的断开连接信号
    connect(this, &FileServer::disconnectClient, msendfile, &SendFile::disconnectClient);

    //连接断开
    connect(msendfile, &SendFile::connectOver, this, [=](){
        //退出子线程
        mthread->exit();
        mthread->wait();
        ui->buttonOpen->setEnabled(false);
        ui->buttonSend->setEnabled(false);
        ui->buttonDisconnect->setEnabled(false);
        ui->textEdit->append(QString("已断开连接"));
        socketLable->setText(QString());
    });
}

FileServer::~FileServer()
{
    mthread->exit();
    mthread->wait();
    msendfile->deleteLater();
    mthread->deleteLater();
    delete ui;
}
