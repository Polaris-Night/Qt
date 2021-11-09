#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mlogmanager.h"
#include <QMenu>
#include <QAction>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    MLogManager::spaceLine();
    qDebug() << "program start";

    statusLabel = new QLabel("未连接", this);
    ui->statusbar->addWidget(statusLabel);
    ui->btnDisconnect->setEnabled(false);
    ui->btnAdd->setEnabled(false);
    ui->btnSend->setEnabled(false);

    //按钮添加图标
    ui->btnConnect->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnDisconnect->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->btnAdd->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    ui->btnSend->setIcon(style()->standardIcon(QStyle::SP_CommandLink));

    //加载样式表
    msetStyleSheet(":/qss/dark.css");
    //主题切换
    connect(ui->actionDark, &QAction::triggered, this, [=](){
        msetStyleSheet(":/qss/dark.css");
    });
    connect(ui->actionBlue, &QAction::triggered, this, [=](){
        msetStyleSheet(":/qss/lightblue.css");
    });
    connect(ui->actionUbuntu, &QAction::triggered, this, [=](){
        msetStyleSheet(":/qss/Ubuntu.qss");
    });
    connect(ui->actionLight, &QAction::triggered, this, [=](){
        msetStyleSheet(":/qss/flatwhite.css");
    });

    //加载使用说明
    helpBox = new QMessageBox(this);
    helpBox->resize(500, 400);
    helpBox->setIconPixmap(QPixmap(":/transfer.png").scaled(40, 40));
    helpBox->setText(loadHelpText());
    connect(ui->actionhelp, &QAction::triggered, helpBox, &QMessageBox::exec);

    //点击连接
    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::toConnect);

    //断开连接
    connect(ui->btnDisconnect, &QPushButton::clicked, this, &MainWindow::goToDisconnect);

    //添加文件
    connect(ui->btnAdd, &QPushButton::clicked, this, [=](){
        QStringList list = QFileDialog::getOpenFileNames(this, "添加");
        if (!list.isEmpty()) {
            fileList.append(list);
            fileList.removeDuplicates();
            ui->listWidget->clear();
            ui->listWidget->addItems(fileList);
            ui->btnSend->setEnabled(true);
        }
    });

    //发送文件
    connect(ui->btnSend, &QPushButton::clicked, this, [=](){
        emit goToSendFile(fileList);
    });

    //定时检测QListWidget是否为空
    connect(&mtimer, &QTimer::timeout, this, [=](){
        if (ui->listWidget->count() > 0 && isConnect)
            ui->btnSend->setEnabled(true);
        else
            ui->btnSend->setEnabled(false);
    });

    //QLiswWidget右键菜单
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, [=](){
        QMenu *popMenu = new QMenu(this);
        QAction *delThis = new QAction("删除此项", this);
        QAction *delAll = new QAction("删除所有项", this);

        //若有选中项，则添加"删除此项"选项
        QList<QListWidgetItem *> itemList = ui->listWidget->selectedItems();
        if (!itemList.isEmpty())
            popMenu->addAction(delThis);

        //添加"删除所有项"
        popMenu->addAction(delAll);

        //删除选中的项
        connect(delThis, &QAction::triggered, this, [=](){
            for (int i = 0; i < itemList.size(); i++) {
                fileList.removeOne(itemList.at(i)->text());
                ui->listWidget->takeItem(ui->listWidget->row(itemList.at(i)));
            }
        });

        //删除所有项
        connect(delAll, &QAction::triggered, this, [=](){
            fileList.clear();
            ui->listWidget->clear();
        });

        popMenu->exec(QCursor::pos());
        delete delAll;
        delete delThis;
        delete popMenu;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::msetStyleSheet(const QString styleSheet)
{
    QFile sheetFile(styleSheet);
    if(sheetFile.open(QFile::ReadOnly)) {
        QString sheet = QLatin1String(sheetFile.readAll());
        qApp->setStyleSheet(sheet);
        sheetFile.close();
    }
}

void MainWindow::toConnect()
{
    statusLabel->setText("正在连接...");

    //创建工作线程
    QString ip = ui->lineEditIP->text();
    quint16 port = ui->lineEditPort->text().toUShort();
    MWork *work = new MWork(ip, port);

    //连接结果
    connect(work, &MWork::connectResult, this, [=](QString result){
        statusLabel->setText(result);
        if (result.at(1) == 'E') {
            work->deleteLater();
            return;
        }
        ui->btnConnect->setEnabled(false);
        ui->btnDisconnect->setEnabled(true);
        ui->btnAdd->setEnabled(true);
        isConnect = true;
        mtimer.start(500);
    });

    //连接断开
    connect(work, &MWork::overConnect, this, [=](){
        work->deleteLater();
        statusLabel->setText("未连接");
        ui->btnConnect->setEnabled(true);
        ui->btnDisconnect->setEnabled(false);
        ui->btnAdd->setEnabled(false);
        ui->btnSend->setEnabled(false);
        isConnect = false;
        mtimer.stop();
    });

    //断开连接
    connect(this, &MainWindow::goToDisconnect, work, &MWork::toDisconnect);

    //发送文件
    connect(this, &MainWindow::goToSendFile, work, &MWork::toSendFile);

    //更新进度
    connect(work, &MWork::updateProgress, ui->progressBar, &QProgressBar::setValue);

    //全部文件发送完成
    connect(work, &MWork::sendFinish, this, [=](){
        QMessageBox::information(this, "信息", "全部文件发送完成");
    });
}

QString MainWindow::loadHelpText()
{
    QFile helpFile("help.txt");
    if (!helpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("open file %s error:%s", qUtf8Printable(QString("help.txt")), qUtf8Printable(helpFile.errorString()));
        return QString();
    }
    QString helpText(helpFile.readAll());
    helpFile.close();
    return helpText;
}
