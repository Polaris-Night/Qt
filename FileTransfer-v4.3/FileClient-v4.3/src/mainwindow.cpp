#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QScopedPointer>
#include <QProcess>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "worker.h"
#include "mlogmanager.h"
#include "titlebar.h"

#define HELP_FILE QStringLiteral("help.txt")

#define CONFIG_FILE QStringLiteral("config.ini")
#define DIR_KEY QStringLiteral("default/dir")
#define THEME_KEY QStringLiteral("default/theme")
#define PORT_KEY QStringLiteral("default/port")

#define PORT_DEFAULT 50050
#define THEME_DEFAULT QStringLiteral("flatwhite.css")

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow) {
    ui->setupUi(this);

    MLogManager::spaceLine();
    qDebug() << "program start";

    //关闭防火墙
    // setFirewall(FirewallMode::DISABLE);

    //初始化变量
    speedTimer = new QTimer(this);
    scanTimer = new QTimer(this);
    statusLabel = new QLabel(this);
    speedLabel = new QLabel(this);
    statusLabel->setFont(QFont(QStringLiteral("微软雅黑"), 9));
    speedLabel->setMinimumSize(100, 0);
    speedLabel->setAlignment(Qt::AlignCenter);
    speedLabel->setFont(QFont(QStringLiteral("微软雅黑"), 9));
    ui->statusbar->addWidget(speedLabel);
    ui->statusbar->addWidget(statusLabel);
    initVariable();

    //加载配置
    configSetting = new QSettings(CONFIG_FILE, QSettings::IniFormat, this);
    //加载样式表，为空则默认为flatwhite.css
    theme = configSetting->value(THEME_KEY).toString();
    if (theme.isEmpty()) {
        theme = THEME_DEFAULT;
        configSetting->setValue(THEME_KEY, theme);
    }
    TitleBar::msetStyleSheet(QString(":/qss/%1").arg(theme));
    //加载端口，为0则默认为50050
    port = configSetting->value(PORT_KEY).toString().toUShort();
    if (port == 0) {
        port = PORT_DEFAULT;
        configSetting->setValue(PORT_KEY, port);
    }
    port = PORT_DEFAULT;

    //按钮添加图标
    ui->btnAdd->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    ui->btnAddDir->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    ui->btnSend->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
    ui->btnDisconnect->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->btnFlushAddrList->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

    initSignalHandle();
}

MainWindow::~MainWindow() {
    //开启防火墙
    // setFirewall(FirewallMode::ENABLE);
    configSetting->setValue(THEME_KEY, theme);
    configSetting->setValue(PORT_KEY, port);
    qDebug() << "program end";
    delete ui;
}

void MainWindow::toConnect(const QString &ip, const quint16 &port) {
    statusLabel->setText(QStringLiteral("正在连接..."));

    //创建工作线程
    Worker *worker = new Worker(ip, port);

    //连接结果
    connect(worker, &Worker::connectResult, this, [=](QString result) {
        statusLabel->setText(result);
        if (result.at(1) == 'E') {
            worker->deleteLater();
            return;
        }
        ui->btnDisconnect->setEnabled(true);
        ui->btnAdd->setEnabled(true);
        ui->btnAddDir->setEnabled(true);
        isConnect = true;
        scanTimer->start(500);
    });

    //连接断开
    connect(worker, &Worker::overConnect, this, [=]() {
        worker->deleteLater();
        initVariable();
    });

    //断开连接
    connect(this, &MainWindow::goToDisconnect, worker, &Worker::disconnectHost);

    //发送文件
    connect(this, &MainWindow::goToSendFile, worker, &Worker::sendFile);

    //更新进度
    connect(worker, &Worker::updateSendProgress, ui->progressBar, &QProgressBar::setValue);
    connect(worker, &Worker::updateSendProgress, this, [this]() {
        if (!speedTimer->isActive())
            speedTimer->start(1000);
    });

    //更新速度
    connect(speedTimer, &QTimer::timeout, worker, [this, worker]() { speedLabel->setText(worker->getSpeed()); });

    //全部文件发送完成
    connect(worker, &Worker::sendFinish, this, [this]() {
        isStartSend = false;
        speedTimer->stop();
        speedLabel->setText(QStringLiteral("0.00B/s"));
        QMessageBox::information(this, QStringLiteral("信息"), QStringLiteral("全部文件发送完成"));
    });

    worker->connectHost();
}

void MainWindow::initSignalHandle() {
    //主题切换
    connect(ui->theme, &QMenu::triggered, this, [&](QAction *action) {
        QString themeText = QString("%1.css").arg(action->text());
        TitleBar::msetStyleSheet(QString(":/qss/%1").arg(themeText));
        theme = themeText;
    });

    //加载使用说明
    helpDialog = new MDialog(this);
    helpDialog->setWindowTitle(QStringLiteral("使用说明"));
    helpDialog->setText(loadHelpText());
    connect(ui->actionHelp, &QAction::triggered, helpDialog, &MDialog::show);

    //发送广播，加载/刷新局域网地址列表
    broadcastSock = new MUdpBroadcast(port, this);
    connect(broadcastSock, &MUdpBroadcast::addressChange, this, [&](QStringList addrList) {
        ui->listWidgetAddr->clear();
        ui->listWidgetAddr->addItems(std::move(addrList));
    });
    //刷新列表
    connect(ui->btnFlushAddrList, &QPushButton::clicked, this, [&]() { broadcastSock->sendBroadcast(); });
    broadcastSock->sendBroadcast();

    //双击连接
    connect(ui->listWidgetAddr, &QListWidget::itemDoubleClicked, this, [&](QListWidgetItem *item) {
        QString ip = item->text();
        toConnect(ip, port);
    });
    //地址列表右键菜单
    connect(ui->listWidgetAddr, &QListWidget::customContextMenuRequested, this, [&]() {
        QScopedPointer<QMenu> popMenu(new QMenu);
        QScopedPointer<QAction> delConnect(new QAction(QStringLiteral("断开连接")));
        //添加"断开连接"
        popMenu->addAction(delConnect.data());
        //断开连接
        connect(delConnect.data(), &QAction::triggered, this, &MainWindow::goToDisconnect);
        popMenu->exec(QCursor::pos());
    });

    //打开日志目录
    connect(ui->actionLogDir, &QAction::triggered, this, [&]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(MLogManager::getLogFileDir()));
    });

    //断开连接
    connect(ui->btnDisconnect, &QPushButton::clicked, this, &MainWindow::goToDisconnect);

    //添加文件
    connect(ui->btnAdd, &QPushButton::clicked, this, [&]() {
        QStringList list = QFileDialog::getOpenFileNames(this, QStringLiteral("添加文件"));
        if (!list.isEmpty()) {
            fileList.append(list);
            fileList.removeDuplicates();
            ui->listWidget->clear();
            ui->listWidget->addItems(fileList);
        }
    });

    //添加文件目录
    connect(ui->btnAddDir, &QPushButton::clicked, this, [&]() {
        //选择文件夹
        QString openDir = QFileDialog::getExistingDirectory(this, QStringLiteral("添加文件夹"));
        if (!openDir.isEmpty()) {
            //遍历文件夹
            QDir dir(openDir);
            //dir.setFilter(filter);
            dir.setFilter(QDir::Files);
            QStringList list = dir.entryList();
            //添加到fileList中
            for (int i = 0, size = list.size(); i < size; i++)
                fileList.append(QString("%1/%2").arg(openDir).arg(list.at(i)));
            fileList.removeDuplicates();
            ui->listWidget->clear();
            ui->listWidget->addItems(fileList);
            ui->btnSend->setEnabled(true);
        }
    });

    //发送文件
    connect(ui->btnSend, &QPushButton::clicked, this, [&]() {
        isStartSend = true;
        emit goToSendFile(fileList);
    });

    //定时检测QListWidget是否为空
    connect(scanTimer, &QTimer::timeout, this, [&]() {
        if (ui->listWidget->count() > 0 && isConnect && !isStartSend)
            ui->btnSend->setEnabled(true);
        else
            ui->btnSend->setEnabled(false);
    });

    //文件列表右键菜单
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, [&]() {
        QScopedPointer<QMenu> popMenu(new QMenu);
        QScopedPointer<QAction> delThis(new QAction(QStringLiteral("删除此项")));
        QScopedPointer<QAction> delAll(new QAction(QStringLiteral("删除所有项")));
        //若有选中项，则添加"删除此项"选项
        QList<QListWidgetItem *> itemList = ui->listWidget->selectedItems();
        if (!itemList.isEmpty())
            popMenu->addAction(delThis.data());
        //添加"删除所有项"
        popMenu->addAction(delAll.data());
        //删除选中的项
        connect(delThis.data(), &QAction::triggered, this, [&]() {
            foreach (QListWidgetItem *item, itemList) {
                fileList.removeOne(item->text());
                ui->listWidget->takeItem(ui->listWidget->row(item));
            }
        });
        //删除所有项
        connect(delAll.data(), &QAction::triggered, this, [&]() {
            fileList.clear();
            ui->listWidget->clear();
        });
        popMenu->exec(QCursor::pos());
    });
}

void MainWindow::setFirewall(FirewallMode mode) {
#ifdef Q_OS_WIN
    QStringList argvList{QStringLiteral("on"), QStringLiteral("off")};
    QString argv = QString("netsh advfirewall set allprofiles state %1\r\n").arg(argvList.at(mode));
    QProcess proc;
    proc.start(QStringLiteral("cmd.exe"), QStringList{QStringLiteral("/c"), argv});
    proc.waitForStarted();
    proc.waitForFinished();
#else

#endif
}

QString MainWindow::loadHelpText() {
    QFile helpFile(HELP_FILE);
    if (!helpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("open file %s error:%s", qUtf8Printable(QString("help.txt")), qUtf8Printable(helpFile.errorString()));
        return QString();
    }
    QString helpText(helpFile.readAll());
    helpFile.close();
    return helpText;
}

void MainWindow::initVariable() {
    statusLabel->setText(QStringLiteral("未连接"));
    speedLabel->setText(QStringLiteral("0.00B/s"));
    ui->btnDisconnect->setEnabled(false);
    ui->btnAdd->setEnabled(false);
    ui->btnAddDir->setEnabled(false);
    ui->btnSend->setEnabled(false);
    isConnect = false;
    isStartSend = false;
    scanTimer->stop();
    speedTimer->stop();
}
