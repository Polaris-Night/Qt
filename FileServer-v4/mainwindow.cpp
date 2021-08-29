#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QList>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("文件传输-v4.0");
    this->setWindowIcon(QIcon("://transfer.png"));
    statusLabel = new QLabel(this);
    ui->statusbar->addWidget(statusLabel);

    //设置默认保存路径
    QDir defaultDir;
    if (!defaultDir.exists("./output"))
        defaultDir.mkdir("./output");
    saveDir = QString("%1/output").arg(defaultDir.absolutePath());
    ui->lineEditPath->setText(saveDir);

    //加载样式表
    QFile qssFile(":/Ubuntu.qss");
    if(qssFile.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(qssFile.readAll());
        qApp->setStyleSheet(qss);
        qssFile.close();
    }

    //创建监听
    server = new MServer(this);

    //点击监听
    connect(ui->btnListen, &QPushButton::clicked, this, [=](){
        quint16 port = ui->comboBox->currentText().toUShort();
        server->close();
        server->listen(QHostAddress::Any, port);
        statusLabel->setText(QString("正在监听[%1]").arg(port));
    });

    //停止监听
    connect(ui->btnStop, &QPushButton::clicked, this, [=](){
        server->close();
        statusLabel->setText(QString());
    });

    //浏览保存路径
    connect(ui->btnView, &QPushButton::clicked, this, [=](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(ui->lineEditPath->text()));
    });

    //修改保存路径
    connect(ui->btnMod, &QPushButton::clicked, this, [=](){
        QString modPath = QFileDialog::getExistingDirectory(this, "选择文件夹", ui->lineEditPath->text());
        if (!modPath.isEmpty()) {
            saveDir = modPath;
            ui->lineEditPath->setText(saveDir);
        }
    });

    //有新连接
    connect(server, &MServer::haveNewConnect, this, &MainWindow::readyConnect);

    //QLiswWidget右键菜单
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, [=](){
        QMenu *popMenu = new QMenu(this);
        QAction *delThis = new QAction("断开此连接", this);
        QAction *delAll = new QAction("断开所有连接", this);

        //若有选中项，则添加"断开此连接"选项
        QList<QListWidgetItem *> itemList = ui->listWidget->selectedItems();
        if (!itemList.isEmpty())
            popMenu->addAction(delThis);

        //添加"断开所有连接"选项
        popMenu->addAction(delAll);

        //断开选中的连接
        connect(delThis, &QAction::triggered, this, [=](){
            for (int i = 0; i < itemList.size(); i++)
                emit goToDisconnectThis(itemList.at(i));
        });

        //断开所有连接
        connect(delAll, &QAction::triggered, this, [=](){
            emit goToDisconnectAll();
        });

        popMenu->exec(QCursor::pos());
        delete delAll;
        delete delThis;
        delete popMenu;
    });

    //QTextBrowser右键菜单
    connect(ui->textBrowser, &QTextBrowser::customContextMenuRequested, this, [=](){
        QMenu *popMenu = new QMenu(this);
        QAction *clearAll = new QAction("清空", this);

        //添加"清空"选项
        popMenu->addAction(clearAll);

        //清空
        connect(clearAll, &QAction::triggered, ui->textBrowser, &QTextBrowser::clear);

        popMenu->exec(QCursor::pos());
        delete clearAll;
        delete popMenu;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readyConnect(qintptr socket)
{
    MWork *work = new MWork(socket);
    QListWidgetItem *item = new QListWidgetItem;

    //socket信息
    connect(work, &MWork::socketMsg, this, [=](QString ip, quint16 port){
        item->setText((QString("[%1]:[%2]").arg(ip).arg(port)));
        ui->listWidget->addItem(item);
        //断开选中的连接
        connect(this, &MainWindow::goToDisconnectThis, this, [=](QListWidgetItem *delItem){
            if (delItem == item) {
                connect(this, &MainWindow::toDisconnect, work, &MWork::toDisconnectThis);
                emit toDisconnect();
            }
        });
    });

    //连接断开
    connect(work, &MWork::overConnect, this, [=](){
        work->deleteLater();
        delete item;
    });

    //获取保存目录
    connect(work, &MWork::goToGetSaveDir, this, [=](){
        emit sendSaveDir(saveDir);
    });
    connect(this, &MainWindow::sendSaveDir, work, &MWork::toGetSaveDir);

    //更新进度条
    connect(work, &MWork::updateProgress, this, [=](int progress){
        ui->progressBar->setValue(progress);
    });

    //更新速度
    connect(work, &MWork::updateSpeedSize, this, [=](qint64 speedSize){
        double speed = speedSize/8.0;//B
        if (speed < 1024) {
            //小于1KB/s
            ui->speedLabel->setText(QString("%1B/s").arg(QString::number(speed, 'f', 2)));
        } else if (speed < 1024*1024) {
            //小于1MB/s
            speed /= 1024;
            ui->speedLabel->setText(QString("%1KB/s").arg(QString::number(speed, 'f', 2)));
        } else {
            //大于1MB/s
            speed /= 1024*1024;
            ui->speedLabel->setText(QString("%1MB/s").arg(QString::number(speed, 'f', 2)));
        }
    });

    //单个文件接收完成
    connect(work, &MWork::fileFinish, this, [=](QString fileName){
        ui->speedLabel->setText("0B/s");
        ui->textBrowser->append(fileName);
    });

    //右键点击断开所有连接
    connect(this, &MainWindow::goToDisconnectAll, work, &MWork::toDisconnectAll);
}

