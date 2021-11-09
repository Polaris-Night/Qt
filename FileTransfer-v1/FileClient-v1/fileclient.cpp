#include "fileclient.h"
#include "ui_fileclient.h"
#include <QDebug>

FileClient::FileClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FileClient)
{
    ui->setupUi(this);
    this->setWindowTitle("文件传输Client-demo");
    this->setWindowIcon(QIcon(QString("://transfer.png")));

    this->statusLable = new QLabel(this);

    ui->statusbar->addWidget(this->statusLable);

    tcpInit();

    //点击连接执行tcpConnect
    connect(ui->buttonConnect, &QPushButton::clicked, this, &FileClient::tcpConnect);

    //连接成功
    connect(this->tcpSocket, &QTcpSocket::connected, [=](){
        QString serverIp = this->tcpSocket->peerName();
        ui->textEdit->setText(QString("[%1]已连接").arg(serverIp));
        this->statusLable->setText(QString("正在与[%1]通信").arg(serverIp));
        connect(this->tcpSocket, &QTcpSocket::disconnected, [=](){
            this->statusLable->setText(QString());
        });
    });

    //文件接收
    connect(this->tcpSocket, &QTcpSocket::readyRead, this, &FileClient::tcpRead);

    ui->buttonConnect->setShortcut(Qt::Key_Enter);
}

FileClient::~FileClient()
{
    delete ui;
}

void FileClient::tcpInit()
{
    this->tcpSocket = new QTcpSocket(this);
    this->outputDirPath = "./output";
    this->isFileInfo = true;
    this->fileName.clear();
    this->fileSize = 0;
    this->receiveSize = 0;
}

void FileClient::tcpConnect()
{
    QString ip = ui->lineEditIP->text();
    qint16 port = ui->lineEditPort->text().toInt();
    this->tcpSocket->connectToHost(ip, port);
    if (!this->tcpSocket->waitForConnected(2000)) {
        ui->textEdit->setText(QString("[Error]%1").arg(this->tcpSocket->errorString()));
    }
}

void FileClient::tcpRead()
{
    //读取socket接收的数据
    QByteArray receiveArray = this->tcpSocket->readAll();
    //处理文件信息
    if (this->isFileInfo) {
        //解包
        this->fileName = QString(receiveArray).section("##", 0, 0);
        this->fileSize = QString(receiveArray).section("##", 1, 1).toInt();
        this->receiveSize = 0;
        //判断存储目录
        QDir dir;
        if (!dir.exists(this->outputDirPath)) {
            dir.mkpath(QString("./output"));
        }
        //创建文件
        this->file.setFileName(QString("%1/%2").arg(this->outputDirPath).arg(this->fileName));
        if (!this->file.open(QIODevice::WriteOnly)) {
            qDebug() << ("文件接收失败");
            this->tcpSocket->disconnectFromHost();
            this->tcpSocket->close();
            this->isFileInfo = true;
            return;
        }
        this->isFileInfo = false;
        //初始化进度条
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(this->fileSize/1024);
        ui->progressBar->setValue(0);
        ui->textEdit->append(QString("正在接收文件..."));
    } else {
        //处理文件内容
        qint64 len = this->file.write(receiveArray);
        this->receiveSize += len;
        //更新进度条
        ui->progressBar->setValue(this->receiveSize/1024);
        if (this->receiveSize == this->fileSize) {
            //给服务端发送消息
            this->tcpSocket->write("File done");
            //关闭socket
            this->tcpSocket->disconnectFromHost();
            this->tcpSocket->close();
            //关闭文件
            this->file.close();
            this->isFileInfo = true;
            //文件接收完成提示信息
            QMessageBox::information(this, "信息", "文件接收完成");
            ui->textEdit->append(QString("文件接收完成"));
            QDir dirPath;
            ui->textEdit->append(QString("保存路径:%1/output/%2").arg(dirPath.absolutePath()).arg(this->fileName));
        }
    }
}
