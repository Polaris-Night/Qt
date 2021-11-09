#include "fileserver.h"
#include "ui_fileserver.h"

FileServer::FileServer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FileServer)
{
    ui->setupUi(this);
    this->setWindowTitle("文件传输Server-demo");
    this->setWindowIcon(QIcon(QString("://transfer.png")));

    //初始化按钮为不可用
    ui->buttonOpen->setEnabled(false);
    ui->buttonSend->setEnabled(false);
    //combobox添加2个端口号选项
    ui->comboBox->addItem(QString("5050"));
    ui->comboBox->addItem(QString("6060"));

    this->tcpServer = new QTcpServer(this);

    this->statusLableListen = new QLabel(this);
    this->statusLableSocket = new QLabel(this);
    ui->statusbar->addWidget(this->statusLableListen);
    ui->statusbar->addWidget(this->statusLableSocket);

    connect(this->tcpServer, &QTcpServer::newConnection, this, &FileServer::tcpConnect);

    connect(ui->buttonOpen, &QPushButton::clicked, this, &FileServer::fileOpen);

    connect(ui->buttonSend, &QPushButton::clicked, this, &FileServer::fileSend);

    connect(ui->buttonListen, &QPushButton::clicked, this, &FileServer::portListen);

    connect(&this->mtimer, &QTimer::timeout, [=](){
        //关闭定时器
        this->mtimer.stop();
        //发送文件
        this->sendFile();
    });
}

FileServer::~FileServer()
{
    delete ui;
}

void FileServer::sendFile()
{
    ui->textEdit->append(QString("正在发送文件..."));
    qint64 len = 0;
    do {
        char buffer[4*1024] = {0};
        //读取文件内容
        len = this->file.read(buffer, sizeof(buffer));
        //发送
        len = this->tcpSocket->write(buffer, len);
        //记录已发送大小
        this->sendSize += len;
    } while (len > 0);

    connect(this->tcpSocket, &QTcpSocket::readyRead, [=](){
        QByteArray receiveArray = this->tcpSocket->readAll();
        if (QString(receiveArray) == "File done") {
            //关闭文件
            this->file.close();
            //关闭socket
            this->tcpSocket->disconnectFromHost();
            this->tcpSocket->close();
            //文件发送完成提示信息
            QMessageBox::information(this, "信息", "文件发送完成");
            ui->textEdit->append(QString("文件发送完成"));
        }
    });
}

/****************************slots**************************************/
void FileServer::portListen()
{
    this->tcpServer->close();
    int port = ui->comboBox->currentText().toInt();
    this->tcpServer->listen(QHostAddress::Any, port);
    this->statusLableListen->setText(QString("正在监听[%1]").arg(port));
}

void FileServer::tcpConnect()
{
    //取出等待队列的下一个socket
    this->tcpSocket = this->tcpServer->nextPendingConnection();

    //取出socket中的信息
    QString ip = this->tcpSocket->peerAddress().toString();
    quint16 port = this->tcpSocket->peerPort();

    //信息打印到文本编辑器
    ui->textEdit->setText(QString("[%1:%2]已连接").arg(ip).arg(port));

    //使能选择文件按钮
    ui->buttonOpen->setEnabled(true);

    //修改状态栏
    this->statusLableSocket->setText(QString("正在与[%1]通信").arg(ip));
    connect(this->tcpSocket, &QTcpSocket::disconnected, [=](){
        ui->buttonOpen->setEnabled(false);
        ui->buttonSend->setEnabled(false);
        this->statusLableSocket->setText(QString());
    });
}

void FileServer::fileOpen()
{
    //打开文件
    QString filePath = QFileDialog::getOpenFileName(this, "打开", "../");
    if (filePath.isEmpty())
        return;

    ui->buttonOpen->setEnabled(false);
    this->sendSize = 0;
    //获取文件信息
    QFileInfo fileInfo(filePath);
    this->fileName = fileInfo.fileName();
    this->fileSize = fileInfo.size();

    //打开文件
    this->file.setFileName(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    //将文件路径打印到文本编辑器
    ui->textEdit->append(filePath);

    //使能发送文件按钮
    ui->buttonSend->setEnabled(true);
}

void FileServer::fileSend()
{
    //先发送文件信息
    QString fileMessage = QString("%1##%2").arg(this->fileName).arg(this->fileSize);//客户端按指定格式解包

    //发送
    qint64 len = this->tcpSocket->write(fileMessage.toUtf8().data());
    if (len <= 0) {
        qDebug() << ("文件信息发送失败");
        this->tcpSocket->disconnectFromHost();
        this->tcpSocket->close();
        this->file.close();
    }

    //发送文件内容
    //定时器解决粘包问题
    mtimer.start(20);
}

