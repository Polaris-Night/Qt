#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QScopedPointer>
#include <QProcess>
#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"

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
    m_config = new QSettings(CONFIG_FILE, QSettings::IniFormat, this);
    m_speedTimer = new QTimer(this);
    m_speedLabel = new QLabel(this);
    m_worker = new Worker(this);
    m_helpDialog = new MDialog(this);

    initConfig();
    initVariable();
    initSignalHandle();
}

MainWindow::~MainWindow() {
    //开启防火墙
    // setFirewall(FirewallMode::ENABLE);
    m_config->setValue(THEME_KEY, theme);
    m_config->setValue(PORT_KEY, m_port);
    qDebug() << "program end";
    delete ui;
}

void MainWindow::updateSendButtonState() {
    ui->btnSend->setEnabled((m_worker->getState() == Worker::Inactive) && ui->listWidget->count() > 0);
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
    m_speedLabel->setText(QStringLiteral("0.00B/s"));
    m_speedLabel->setMinimumSize(100, 0);
    m_speedLabel->setAlignment(Qt::AlignCenter);
    m_speedLabel->setFont(QFont(QStringLiteral("微软雅黑"), 9));
    ui->statusbar->addWidget(m_speedLabel);
    updateSendButtonState();
}

void MainWindow::initConfig() {
    //加载样式表，为空则默认为flatwhite.css
    theme = m_config->value(THEME_KEY).toString();
    if (theme.isEmpty()) {
        theme = THEME_DEFAULT;
        m_config->setValue(THEME_KEY, theme);
    }
    TitleBar::msetStyleSheet(QString(":/qss/%1").arg(theme));
    //加载端口，为0则默认为50050
    m_port = m_config->value(PORT_KEY).toString().toUShort();
    if (m_port == 0) {
        m_port = PORT_DEFAULT;
        m_config->setValue(PORT_KEY, m_port);
    }
    m_port = PORT_DEFAULT;
    m_broadcastSocket = new MUdpBroadcast(m_port, this);

    //按钮添加图标
    ui->btnAdd->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    ui->btnAddDir->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    ui->btnSend->setIcon(style()->standardIcon(QStyle::SP_CommandLink));
    ui->btnFlushAddrList->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
}

void MainWindow::initSignalHandle() {
    //主题切换
    connect(ui->theme, &QMenu::triggered, this, [&](QAction *action) {
        QString themeText = QString("%1.css").arg(action->text());
        TitleBar::msetStyleSheet(QString(":/qss/%1").arg(themeText));
        theme = themeText;
    });

    //加载使用说明
    m_helpDialog->setWindowTitle(QStringLiteral("使用说明"));
    m_helpDialog->setText(loadHelpText());
    connect(ui->actionHelp, &QAction::triggered, m_helpDialog, &MDialog::show);

    //发送广播，加载/刷新局域网地址列表
    connect(m_broadcastSocket, &MUdpBroadcast::addressChange, this, [&](QStringList addrList) {
        ui->hostBox->clear();
        ui->hostBox->addItems(std::move(addrList));
    });
    //刷新列表
    connect(ui->btnFlushAddrList, &QPushButton::clicked, this, [&]() { m_broadcastSocket->sendBroadcast(); });
    m_broadcastSocket->sendBroadcast();

    //打开日志目录
    connect(ui->actionLogDir, &QAction::triggered, this, [&]() {
        QDesktopServices::openUrl(QUrl::fromLocalFile(MLogManager::getLogFileDir()));
    });

    //添加文件
    connect(ui->btnAdd, &QPushButton::clicked, this, [&]() {
        QStringList list = QFileDialog::getOpenFileNames(this, QStringLiteral("添加文件"));
        if (!list.isEmpty()) {
            m_fileList.append(list);
            m_fileList.removeDuplicates();
            ui->listWidget->clear();
            ui->listWidget->addItems(m_fileList);
        }
        updateSendButtonState();
    });

    //添加文件目录
    connect(ui->btnAddDir, &QPushButton::clicked, this, [&]() {
        //选择文件夹
        QString openDir = QFileDialog::getExistingDirectory(this, QStringLiteral("添加文件夹"));
        if (!openDir.isEmpty()) {
            //遍历文件夹
            QDir dir(openDir);
            dir.setFilter(QDir::Files);
            QStringList list = dir.entryList();
            //添加到fileList中
            for (auto &item : list) {
                m_fileList.append(QString("%1/%2").arg(openDir).arg(item));
            }
            m_fileList.removeDuplicates();
            ui->listWidget->clear();
            ui->listWidget->addItems(m_fileList);
        }
        updateSendButtonState();
    });

    //发送文件
    connect(ui->btnSend, &QPushButton::clicked, this, [&]() {
        QString host = ui->hostBox->currentText();
        if (host.isEmpty()) {
            return;
        }
        m_worker->setIp(host);
        m_worker->setPort(m_port);
        m_worker->sendFile(m_fileList);
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
        connect(delThis.data(), &QAction::triggered, this, [=]() {
            for (auto *item : itemList) {
                m_fileList.removeOne(item->text());
                ui->listWidget->takeItem(ui->listWidget->row(item));
            }
            updateSendButtonState();
        });
        //删除所有项
        connect(delAll.data(), &QAction::triggered, this, [&]() {
            m_fileList.clear();
            ui->listWidget->clear();
            updateSendButtonState();
        });
        popMenu->exec(QCursor::pos());
    });

    // worker
    // 发送状态变化
    connect(m_worker, &Worker::stateChanged, this, [this](Worker::State state) {
        if (state == Worker::Sending) {
            m_speedTimer->start(1000);
        } else {
            m_speedTimer->stop();
        }
        updateSendButtonState();
    });
    // 速度更新
    connect(m_speedTimer, &QTimer::timeout, m_worker, [this]() { m_speedLabel->setText(m_worker->getSpeed()); });
    // 进度更新
    connect(m_worker, &Worker::sendProgressChanged, ui->progressBar, &QProgressBar::setValue);
    // 发送完成
    connect(m_worker, &Worker::sendFinish, this, [this]() {
        QMessageBox::information(this, QStringLiteral("信息"), QStringLiteral("全部文件发送完成"));
    });
}
