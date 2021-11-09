#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QMenu>
#include <QList>
#include <QMessageBox>
#include <QAction>
#include <QFileDialog>
#include <QDesktopServices>
#include <QScopedPointer>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mlogmanager.h"
#include "mhostaddress.h"
#include "titlebar.h"
#include "mrecvfilethread.h"
#include "mmergethread.h"

#define HELP_FILE QStringLiteral("help.txt")

#define CONFIG_FILE QStringLiteral("config.ini")
#define DIR_KEY QStringLiteral("default/dir")
#define THEME_KEY QStringLiteral("default/theme")
#define PORT_KEY QStringLiteral("default/port")

#define PORT_DEFAULT 50050
#define THEME_DEFAULT QStringLiteral("flatwhite.css")
#define DIR_DEFAULT QStringLiteral("./output")

/************************************主线程***************************************/
QString saveDir;//全局保存目录

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    MLogManager::spaceLine();
    qDebug() << "program start";

    //关闭防火墙
    setFirewall(FirewallMode::DISABLE);

    //初始化变量
    initVariable();

    //加载保存路径，为空则默认为当前目录下的output目录
    QString dir = configSetting->value(DIR_KEY).toString();
    QDir defaultDir;
    if (!defaultDir.exists(DIR_DEFAULT))
        defaultDir.mkdir(DIR_DEFAULT);
    if (dir.isEmpty()) {
        dir = QString("%1/%2").arg(defaultDir.absolutePath()).arg(defaultDir.relativeFilePath(DIR_DEFAULT));
        configSetting->setValue(DIR_KEY, dir);
    }
    setSaveDir(dir);
    //加载样式表，为空则默认为flatwhite.css
    theme = configSetting->value(THEME_KEY).toString();
    if (theme.isEmpty()) {
        theme = THEME_DEFAULT;
        configSetting->setValue(THEME_KEY, theme);
    }
    TitleBar::msetStyleSheet(QString(":/qss/%1").arg(theme));
    //加载监听端口，为0则默认为50050
    port = configSetting->value(PORT_KEY).toString().toUShort();
    if (port == 0) {
        port = PORT_DEFAULT;
        configSetting->setValue(PORT_KEY, port);
    }
    server->listen(QHostAddress::Any, port);
    ui->portLabel->setText(QString::number(port));

    //发送广播
    broadcastSock = new MUdpBroadcast(port, this);
    broadcastSock->sendBroadcast();

    //按钮添加图标
    ui->btnView->setIcon(style()->standardIcon(QStyle::SP_DirHomeIcon));
    ui->btnMod->setIcon(style()->standardIcon(QStyle::SP_CommandLink));

    //主题切换
    connect(ui->theme, &QMenu::triggered, this, [=](QAction *action) {
        QString themeText = QString("%1.css").arg(action->text());
        theme = themeText;
        TitleBar::msetStyleSheet(QString(":/qss/%1").arg(theme));
    });

    //打开使用说明窗口
    connect(ui->actionHelp, &QAction::triggered, helpDialog, &MDialog::show);

    //获取本机活动IP
    ipLabel->setText(QString("本机地址[%1]").arg(MHostAddress::getHostAddressTable().join(" | ")));
    connect(ui->actionIpTabel, &QAction::triggered, this, [=]() {
        QStringList ipTable = MHostAddress::getHostAddressTable();
        QMessageBox::information(this, QStringLiteral("本机地址表"), ipTable.join("\n"), QMessageBox::Ok);
    });

    //打开日志目录
    connect(ui->actionLogDir, &QAction::triggered, this, [=]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(MLogManager::getLogFileDir()));
    });

    //浏览保存目录
    connect(ui->btnView, &QPushButton::clicked, this, [=]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(ui->lineEditPath->text()));
    });

    //修改保存目录
    connect(ui->btnMod, &QPushButton::clicked, this, [=]() {
        QString modPath = QFileDialog::getExistingDirectory(this, QStringLiteral("选择文件夹"), ui->lineEditPath->text());
        if (!modPath.isEmpty()) {
            configSetting->setValue(DIR_KEY, modPath);
            setSaveDir(modPath);
        }
    });

    //有新连接
    connect(server, &MServer::haveNewConnect, this, &MainWindow::readyConnect);

    //更新速度及时间
    connect(mtimer, &QTimer::timeout, this, &MainWindow::updateSpeedAndTime);

    //QLiswWidget右键菜单
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, [=]() {
        QScopedPointer<QMenu> popMenu(new QMenu);
        QScopedPointer<QAction> delThis(new QAction(QStringLiteral("断开此连接")));
        QScopedPointer<QAction> delAll(new QAction(QStringLiteral("断开所有连接")));
        //若有选中项，则添加"断开此连接"选项
        QList<QListWidgetItem*> itemList = ui->listWidget->selectedItems();
        if (!itemList.isEmpty())
            popMenu->addAction(delThis.data());
        //添加"断开所有连接"选项
        popMenu->addAction(delAll.data());
        //断开选中的连接
        connect(delThis.data(), &QAction::triggered, this, [=]() {
            foreach (QListWidgetItem *item, itemList)
                emit goToDisconnectThis(item);
        });
        //断开所有连接
        connect(delAll.data(), &QAction::triggered, this, &MainWindow::goToDisconnectAll);
        popMenu->exec(QCursor::pos());
    });

    //QTextBrowser右键菜单
    connect(ui->textBrowser, &QTextBrowser::customContextMenuRequested, this, [=]() {
        QScopedPointer<QMenu> popMenu(new QMenu);
        QScopedPointer<QAction> clearAll(new QAction(QStringLiteral("清空")));
        //添加"清空"选项
        popMenu->addAction(clearAll.data());
        //清空
        connect(clearAll.data(), &QAction::triggered, ui->textBrowser, &QTextBrowser::clear);
        popMenu->exec(QCursor::pos());
    });
}

MainWindow::~MainWindow()
{
    setFirewall(FirewallMode::ENABLE);
    configSetting->setValue(DIR_KEY, saveDir);
    configSetting->setValue(THEME_KEY, theme);
    configSetting->setValue(PORT_KEY, port);
    qDebug() << "program end";
    delete ui;
}

void MainWindow::setSaveDir(const QString &dir)
{
    MRecvFileThread::rwLock.lockForWrite();//加写锁
    saveDir = dir;
    MRecvFileThread::rwLock.unlock();//解写锁
    ui->lineEditPath->setText(dir);
}

void MainWindow::initVariable()
{
    //加载配置
    configSetting = new QSettings(CONFIG_FILE, QSettings::IniFormat, this);
    server = new MServer(this);
    mtimer = new QTimer(this);
    ipLabel = new QLabel(this);
    ui->statusbar->addWidget(ui->speedLabel);
    ui->statusbar->addWidget(ui->timeLabel);
    ui->statusbar->addPermanentWidget(ipLabel);
    //加载使用说明
    helpDialog = new MDialog(this);
    helpDialog->setWindowTitle(QStringLiteral("使用说明"));
    helpDialog->setText(loadHelpText());
}

QString MainWindow::loadHelpText()
{
    QFile helpFile(HELP_FILE);
    if (!helpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("open file %s error:%s", qUtf8Printable(QString("help.txt")), qUtf8Printable(helpFile.errorString()));
        return QString();
    }
    QString helpText(helpFile.readAll());
    helpFile.close();
    return helpText;
}

void MainWindow::setFirewall(FirewallMode mode)
{
#ifdef Q_OS_WIN
//    QMessageBox msgBox;
//    QPushButton *btnAllow = new QPushButton("允许规则", &msgBox);
//    QPushButton *btnBlock = new QPushButton("禁止规则", &msgBox);
//    msgBox.setIconPixmap(QPixmap(":/firewall.png").scaled(40, 40));
//    msgBox.setWindowTitle(QStringLiteral("防火墙规则"));
//    msgBox.setText(QStringLiteral("允许或禁止防火墙规则"));
//    msgBox.addButton(btnAllow, QMessageBox::AcceptRole);
//    msgBox.addButton(btnBlock, QMessageBox::RejectRole);

//    QVector<QAbstractButton *> btnVect{btnAllow, btnBlock};
//    connect(&msgBox, &QMessageBox::buttonClicked, this, [=](QAbstractButton *button) {
//        int index = btnVect.indexOf(button);
//        //防火墙进程
//        QProcess firewallProc;
//        //规则
//        QStringList ruleStr {QStringLiteral("allow"), QStringLiteral("block")};
//        //获取程序名
//        QString appName = QApplication::applicationName();
//        //命令
//        QString cmdStr = QString("netsh advfirewall firewall set rule name=\"%1\" new enable=yes action=%2\r\n").arg(appName).arg(ruleStr.at(index));
//        //修改防火墙规则
//        firewallProc.start(QStringLiteral("cmd.exe"), QStringList() << "/c" << cmdStr);
//        firewallProc.waitForStarted(3000);
//        firewallProc.waitForFinished(3000);
//        qDebug() << QString("%1 firewall set %2").arg(appName).arg(ruleStr.at(index));
//    });
//    msgBox.exec();
//    QMessageBox::information(this, QStringLiteral("信息"), QStringLiteral("已修改规则"));

    QStringList argvList{QStringLiteral("on"), QStringLiteral("off")};
    QString argv = QString("netsh advfirewall set allprofiles state %1\r\n").arg(argvList.at(mode));
    QProcess proc;
    proc.start(QStringLiteral("cmd.exe"), QStringList{QStringLiteral("/c"), argv});
    proc.waitForStarted();
    proc.waitForFinished();
#else

#endif
}

void MainWindow::updateSpeedAndTime()
{
    ui->timeLabel->setText(clock.getClock(QStringLiteral("hh:mm:ss")));
    ui->speedLabel->setText(MRecvFileThread::getSpeedSize());
}

void MainWindow::readyConnect(qintptr socket)
{
    MRecvFileThread *recvThread = new MRecvFileThread(socket);
    MListWidgetItem *item = new MListWidgetItem(ui->listWidget);
    emit recvThread->toConnect();

    //socket信息
    connect(recvThread, &MRecvFileThread::socketMsg, item, &MListWidgetItem::setMText);

    //连接断开
    connect(recvThread, &MRecvFileThread::overConnect, recvThread, &MRecvFileThread::deleteLater);
    connect(recvThread, &MRecvFileThread::overConnect, item, &MListWidgetItem::deleteLater);

    //更新全部文件总大小
    connect(recvThread, &MRecvFileThread::updateTotalSize, this, &MMergeThread::setTotalSize);

    //更新进度条
    connect(recvThread, &MRecvFileThread::updateRecvProgress, this, [=](int progress) {
        ui->progressBarRecv->setValue(progress);
        if (!mtimer->isActive() && progress == 0) {
            ui->timeLabel->setText(clock.getClock(QStringLiteral("hh:mm:ss")));
            ui->progressBarTotal->setValue(0);
            mtimer->start(1000);
            clock.start();
        } else if (progress == 100) {
            ui->speedLabel->setText(QStringLiteral("0.00B/s"));
            mtimer->stop();
            clock.clear();
        }
    });

    //单个文件接收完成
    connect(recvThread, &MRecvFileThread::finishRecv, this, [=](BlockMsg msg) {
        ui->textBrowser->append(QString("[完成]%1.00%2").arg(msg.fileName).arg(msg.block+1));
        //创建线程合并文件
        MMergeThread *mergeThread = new MMergeThread(saveDir, msg);
        //更新总进度条
        connect(mergeThread, &MMergeThread::updateMergeProgress, ui->progressBarTotal, &QProgressBar::setValue);
        //启动线程
        mergeThread->start();
    });

    //断开选中的连接
    connect(this, &MainWindow::goToDisconnectThis, this, [=](QListWidgetItem *delItem) {
        if (delItem == item)
            emit recvThread->goToDisconnect();
    });

    //右键点击断开所有连接
    connect(this, &MainWindow::goToDisconnectAll, recvThread, &MRecvFileThread::goToDisconnect);
}
