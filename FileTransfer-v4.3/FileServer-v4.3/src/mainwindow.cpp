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
#include <QThreadPool>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mlogmanager.h"
#include "mhostaddress.h"
#include "titlebar.h"

#define HELP_FILE QStringLiteral("help.txt")

#define CONFIG_FILE QStringLiteral("config.ini")
#define DIR_KEY QStringLiteral("default/dir")
#define THEME_KEY QStringLiteral("default/theme")
#define PORT_KEY QStringLiteral("default/port")

#define PORT_DEFAULT 50050
#define THEME_DEFAULT QStringLiteral("flatwhite.css")
#define DIR_DEFAULT QStringLiteral("./output")

/************************************主线程***************************************/
QString saveDir; //全局保存目录

namespace {

const Qt::ItemDataRole role = Qt::UserRole; // 使用UserRole作为数据索引

} // namespace

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow) {
    ui->setupUi(this);

    MLogManager::spaceLine();
    qDebug() << "program start";

    //关闭防火墙
    // setFirewall(FirewallMode::DISABLE);

    m_config = new QSettings(CONFIG_FILE, QSettings::IniFormat, this);
    m_server = new MServer(this);
    m_timer = new QTimer(this);
    m_ipLabel = new QLabel(this);
    m_worker = new Worker(this);

    initConfig();
    initVariable();
    initSignalHandle();
}

MainWindow::~MainWindow() {
    // setFirewall(FirewallMode::ENABLE);
    m_config->setValue(DIR_KEY, saveDir);
    m_config->setValue(THEME_KEY, theme);
    m_config->setValue(PORT_KEY, m_port);
    qDebug() << "program end";
    delete ui;
}

void MainWindow::setSaveDir(const QString &dir) {
    saveDir = dir;
    ui->lineEditPath->setText(dir);
}

void MainWindow::initVariable() {
    //加载配置
    ui->statusbar->addWidget(ui->speedLabel);
    ui->statusbar->addWidget(ui->timeLabel);
    ui->statusbar->addPermanentWidget(m_ipLabel);
    //加载使用说明
    m_helpDialog = new MDialog(this);
    m_helpDialog->setWindowTitle(QStringLiteral("使用说明"));
    m_helpDialog->setText(loadHelpText());
}

void MainWindow::initConfig() { //加载保存路径，为空则默认为当前目录下的output目录
    QString dir = m_config->value(DIR_KEY).toString();
    QDir defaultDir;
    if (!defaultDir.exists(DIR_DEFAULT))
        defaultDir.mkdir(DIR_DEFAULT);
    if (dir.isEmpty()) {
        dir = QString("%1/%2").arg(defaultDir.absolutePath()).arg(defaultDir.relativeFilePath(DIR_DEFAULT));
        m_config->setValue(DIR_KEY, dir);
    }
    setSaveDir(dir);
    //加载样式表，为空则默认为flatwhite.css
    theme = m_config->value(THEME_KEY).toString();
    if (theme.isEmpty()) {
        theme = THEME_DEFAULT;
        m_config->setValue(THEME_KEY, theme);
    }
    TitleBar::msetStyleSheet(QString(":/qss/%1").arg(theme));
    //加载监听端口，为0则默认为50050
    m_port = m_config->value(PORT_KEY).toString().toUShort();
    if (m_port == 0) {
        m_port = PORT_DEFAULT;
        m_config->setValue(PORT_KEY, m_port);
    }
    m_server->listen(QHostAddress::Any, m_port);
    ui->portLabel->setText(QString::number(m_port));

    //发送广播
    m_broadcastSocket = new MUdpBroadcast(m_port, this);
    m_broadcastSocket->sendBroadcast();

    //按钮添加图标
    ui->btnView->setIcon(style()->standardIcon(QStyle::SP_DirHomeIcon));
    ui->btnMod->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
}

void MainWindow::initSignalHandle() {
    //主题切换
    connect(ui->theme, &QMenu::triggered, this, [=](QAction *action) {
        QString themeText = QString("%1.css").arg(action->text());
        theme = themeText;
        TitleBar::msetStyleSheet(QString(":/qss/%1").arg(theme));
    });

    //打开使用说明窗口
    connect(ui->actionHelp, &QAction::triggered, m_helpDialog, &MDialog::show);

    //获取本机活动IP
    m_ipLabel->setText(QString("本机地址[%1]").arg(MHostAddress::getHostAddressTable().join(" | ")));
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
        QString modPath =
            QFileDialog::getExistingDirectory(this, QStringLiteral("选择文件夹"), ui->lineEditPath->text());
        if (!modPath.isEmpty()) {
            m_config->setValue(DIR_KEY, modPath);
            setSaveDir(modPath);
        }
    });

    //有新连接
    connect(m_server, &MServer::haveNewConnect, this, &MainWindow::readyConnect);

    //更新速度及时间
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateSpeedAndTime);

    //QLiswWidget右键菜单
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, [=]() {
        QScopedPointer<QMenu> popMenu(new QMenu);
        QScopedPointer<QAction> delThis(new QAction(QStringLiteral("断开此连接")));
        QScopedPointer<QAction> delAll(new QAction(QStringLiteral("断开所有连接")));
        //若有选中项，则添加"断开此连接"选项
        QList<QListWidgetItem *> itemList = ui->listWidget->selectedItems();
        if (!itemList.isEmpty())
            popMenu->addAction(delThis.data());
        //添加"断开所有连接"选项
        popMenu->addAction(delAll.data());
        //断开选中的连接
        connect(delThis.data(), &QAction::triggered, this, [=]() {
            for (auto *item : itemList) {
                m_worker->disconnectClient(item->data(::role).toLongLong());
            }
        });
        //断开所有连接
        connect(delAll.data(), &QAction::triggered, this, [=]() {
            for (int i = 0, count = ui->listWidget->count(); i < count; i++) {
                auto *item = ui->listWidget->item(i);
                m_worker->disconnectClient(item->data(::role).toLongLong());
            }
        });
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

    connect(m_worker, &Worker::receiveProgressChanged, this, [this](int progress) {
        ui->progressBarRecv->setValue(progress);
        if (!m_timer->isActive()) {
            m_timer->start(1000);
            m_clock.start();
        }
    });
    connect(m_worker, &Worker::mergeProgressChanged, ui->progressBarTotal, &QProgressBar::setValue);
    connect(m_worker, &Worker::receiveFinished, this, [this]() {
        m_timer->stop();
        m_clock.clear();
        m_worker->getSpeed();
        updateSpeedAndTime();
    });
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

void MainWindow::setFirewall(FirewallMode mode) {
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

void MainWindow::updateSpeedAndTime() {
    ui->timeLabel->setText(m_clock.getClock(QStringLiteral("hh:mm:ss")));
    ui->speedLabel->setText(m_worker->getSpeed());
}

void MainWindow::readyConnect(qintptr socket) {
    MListWidgetItem *item = new MListWidgetItem(ui->listWidget);
    connect(m_worker, &Worker::clientConnected, item, [item](QString hostPort, qintptr fd) {
        item->setMText(hostPort);
        item->setData(::role, fd);
    });
    connect(m_worker, &Worker::clientDisconnected, item, [item](QString, qintptr fd) {
        if (item->data(::role).toLongLong() != fd) {
            return;
        }
        item->deleteLater();
    });
    m_worker->setSaveDir(saveDir);
    m_worker->appendClient(socket);
}
