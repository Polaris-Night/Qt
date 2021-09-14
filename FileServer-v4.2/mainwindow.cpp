#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mlogmanager.h"
#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QMenu>
#include <QList>
#include <QFile>
#include <QMutex>
#include <QAction>
#include <QProcess>
#include <QFileDialog>
#include <QDesktopServices>


/************************************主线程***************************************/
QMutex mutex;//全局互斥锁
qint64 speedSize = 0;//单位时间接收总大小
QString saveDir;//全局保存目录

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("文件传输-v4.2");
    this->setWindowIcon(QIcon("://transfer.png"));
    statusLabel = new QLabel(this);
    ui->statusbar->addWidget(statusLabel);

    //设置默认保存路径
    QDir defaultDir;
    if (!defaultDir.exists("./output"))
        defaultDir.mkdir("./output");
    MWork::rwLock.lockForWrite();//加写锁
    saveDir = QString("%1/output").arg(defaultDir.absolutePath());
    ui->lineEditPath->setText(saveDir);
    MWork::rwLock.unlock();//解写锁

    MLogManager::spaceLine();
    qDebug() << "program start";

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

    //创建监听
    server = new MServer(this);

    //点击监听
    connect(ui->btnListen, &QPushButton::clicked, this, [=](){
        quint16 port = ui->lineEditPort->text().toUShort();
        server->close();
        server->listen(QHostAddress::Any, port);
        statusLabel->setText(QString("正在监听[%1]").arg(port));
        qDebug() << QString("start to listen %1").arg(port);
    });

    //停止监听
    connect(ui->btnStop, &QPushButton::clicked, this, [=](){
        server->close();
        statusLabel->setText(QString());
        qDebug() << QString("stop to listen %1");
    });

    //浏览保存目录
    connect(ui->btnView, &QPushButton::clicked, this, [=](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(ui->lineEditPath->text()));
    });

    //修改保存目录
    connect(ui->btnMod, &QPushButton::clicked, this, [=](){
        QString modPath = QFileDialog::getExistingDirectory(this, "选择文件夹", ui->lineEditPath->text());
        if (!modPath.isEmpty()) {
            MWork::rwLock.lockForWrite();//加写锁
            saveDir = modPath;
            MWork::rwLock.unlock();//解写锁
            ui->lineEditPath->setText(modPath);
        }
    });

    //有新连接
    connect(server, &MServer::haveNewConnect, this, &MainWindow::readyConnect);

    //更新速度
    connect(&mtimer, &QTimer::timeout, this, [=](){
        mutex.lock();//加锁
        double speed = speedSize/1.0;//B
        speedSize = 0;
        mutex.unlock();//解锁
        ui->speedLabel->setText(speedToString(speed));
    });

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
    if (!MergeThread::lockMap.isEmpty()) {
        QList<QReadWriteLock*> temp = MergeThread::lockMap.values();
        for (int i = 0, size = temp.size(); i < size; i++)
            delete temp[i];
    }
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

QString MainWindow::speedToString(double &speed)
{
    if (speed < 1024) {
        //小于1KB/s
    } else if (speed < 1024*1024) {
        //小于1MB/s
        speed /= 1024;
    } else {
        //大于1MB/s
        speed /= 1024*1024;
    }
    return QString("%1MB/s").arg(QString::number(speed, 'f', 2));
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

    //更新全部文件总大小
    connect(work, &MWork::updateTotalSize, this, [=](qint64 size){
        MergeThread::rwLock.lockForWrite();
        MergeThread::totalSize += size;
        MergeThread::rwLock.unlock();
    });

    //更新进度条
    connect(work, &MWork::updateProgress, this, [=](int progress){
        ui->progressBarRecv->setValue(progress);
        if (!mtimer.isActive() && progress == 0) {
            mutex.lock();//加锁
            speedSize = 0;
            mutex.unlock();//解锁
            ui->progressBarTotal->setValue(0);
            mtimer.start(1000);
        } else if (progress == 100) {
            ui->speedLabel->setText("0B/s");
            mtimer.stop();
        }
    });

    //单个文件接收完成
    connect(work, &MWork::fileFinished, this, [=](FileMsg msg){
        ui->textBrowser->append(QString("[完成]%1.00%2").arg(msg.fileName).arg(msg.block+1));
        //每个文件创建一个读写锁
        if (!MergeThread::lockMap.contains(msg.fileName)) {
            QReadWriteLock *lock = new QReadWriteLock;
            MergeThread::lockMap.insert(msg.fileName, lock);
        }
        //创建线程合并文件
        MergeThread *thread = new MergeThread(saveDir, msg);
        //更新总进度条
        connect(thread, &MergeThread::updateTotalProgress, this, [=](int totalProgress){
            ui->progressBarTotal->setValue(totalProgress);
        });
        //启动线程
        thread->start();
    });

    //右键点击断开所有连接
    connect(this, &MainWindow::goToDisconnectAll, work, &MWork::toDisconnectAll);
}

/**********************************合并文件线程**************************************/
qint64 MergeThread::totalSize = 0;//全部文件总大小
qint64 MergeThread::totalFinishedSize = 0;//已完成总大小
int MergeThread::totalProgress = 0;//总进度值
int MergeThread::totalTempProgress = 0;//临时总进度值
QReadWriteLock MergeThread::rwLock;//读写锁
QMap<QString, QReadWriteLock*> MergeThread::lockMap;//读写锁列表

MergeThread::MergeThread(const QString saveDir, const FileMsg &msg, QObject *parent) : QThread(parent)
{
    this->saveDir = saveDir;
    this->msg = msg;
}

MergeThread::~MergeThread()
{

}

void MergeThread::calculateProgress(const qint64 &len)
{
    rwLock.lockForWrite();//加写锁
    totalFinishedSize += len;
    totalTempProgress = static_cast<double>(totalFinishedSize) / static_cast<double>(totalSize) * 100;
    if (totalTempProgress != totalProgress) {
        totalProgress = totalTempProgress;
        emit updateTotalProgress(totalProgress);
        if (totalProgress == 100) {
            totalProgress = totalTempProgress = 0;
            totalSize = totalFinishedSize = 0;
        }
    }
    rwLock.unlock();//解写锁
}

void MergeThread::run()
{
    connect(this, &MergeThread::finished, this, &MergeThread::deleteLater);

    QFile blockFile(QString("%1/%2.00%3").arg(saveDir).arg(msg.fileName).arg(msg.block+1));
    QFile sourceFile(QString("%1/%2").arg(saveDir).arg(msg.fileName));
    if (!blockFile.open(QIODevice::ReadOnly)) {
        qWarning("open file %s error:%s", qUtf8Printable(QString("%1.00%2").arg(msg.fileName).arg(msg.block+1)), qUtf8Printable(blockFile.errorString()));
        return;
    }

    char buffer[4*1024];
    size_t bufferSize = sizeof(buffer);
    qint64 len = 0;
    qint64 offset;
    //计算偏移量
    offset = (msg.block == 3) ? (msg.fileSize - msg.blockSize) : (msg.blockSize * msg.block);

    //读分块文件并写入源文件
    lockMap[msg.fileName]->lockForWrite();//加写锁
    if (!sourceFile.open(QIODevice::Append | QIODevice::WriteOnly)) {
        qWarning("block %s open file %s error:%s", qUtf8Printable(QString::number(msg.block)), qUtf8Printable(msg.fileName), qUtf8Printable(sourceFile.errorString()));
        //打开失败，使用QProcess处理后再次打开
        QProcess proc;
        proc.start(sourceFile.fileName(), QStringList(sourceFile.fileName()));
        proc.close();
        if (!sourceFile.open(QIODevice::Append | QIODevice::WriteOnly)) {
            lockMap[msg.fileName]->unlock();//解写锁
            qWarning("open file %s error:%s", qUtf8Printable(msg.fileName), qUtf8Printable(sourceFile.errorString()));
            blockFile.close();
            return;
        }
    }
    sourceFile.seek(offset);
    do {
        memset(buffer, 0, bufferSize);
        len = blockFile.read(buffer, bufferSize);
        len = sourceFile.write(buffer, len);
        //计算更新总进度
        calculateProgress(len);
    } while (len > 0);
    sourceFile.close();
    lockMap[msg.fileName]->unlock();//解写锁
    qDebug() << QString("merge block %1 to file %2 finished").arg(msg.block).arg(msg.fileName);

    //关闭并移除分块文件
    blockFile.close();
    blockFile.remove();
}
