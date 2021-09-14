#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mthread.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("文件分割/合并");
    splitLabel = new QLabel(this);
    mergeLabel = new QLabel(this);
    ui->statusbar->addWidget(splitLabel);
    ui->statusbar->addWidget(mergeLabel);

    //加载样式表
    QFile qssFile(":/qss/css/dark.css");
    if(qssFile.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(qssFile.readAll());
        qApp->setStyleSheet(qss);
        qssFile.close();
    }

    /****************************文件分割********************************/
    saveDir = QDir().absolutePath();
    ui->lineEditSplitPath->setText(saveDir);

    //选择文件
    connect(ui->btnSplitSelect, &QPushButton::clicked, this, [=](){
        QString openFileName = QFileDialog::getOpenFileName(this, "选择", saveDir);
        if (!openFileName.isEmpty()) {
            splitFileName = openFileName;
            ui->lineEditSplitFile->setText(splitFileName);
        }
    });

    //浏览
    connect(ui->btnSplitView, &QPushButton::clicked, this, [=](){
        QDesktopServices::openUrl(QUrl::fromLocalFile(ui->lineEditSplitPath->text()));
    });

    //修改路径
    connect(ui->btnSplitMode, &QPushButton::clicked, this, [=](){
        QString modDir = QFileDialog::getExistingDirectory();
        if (!modDir.isEmpty()) {
            saveDir = modDir;
            ui->lineEditSplitPath->setText(saveDir);
        }
    });

    //开始分割
    connect(ui->btnSplitStart, &QPushButton::clicked, this, [=](){
        if (splitFileName.isEmpty())
            return;
        splitLabel->setText("正在分割文件...");
        //创建线程分割文件
        MThread *thread = new MThread(0, splitFileName, QStringList(), saveDir);
        connect(thread, &MThread::finished, this, [=](){
            thread->deleteLater();
            splitLabel->setText(QString());
            ui->textBrowser->append(QString("[完成]%1").arg(splitFileName));
        });
        thread->start();
    });

    /****************************文件合并********************************/
    //选择文件
    connect(ui->btnMergeSelect, &QPushButton::clicked, this, [=](){
        QStringList openFileNames = QFileDialog::getOpenFileNames(this, "选择", saveDir);
        if (!openFileNames.isEmpty()) {
            mergeFileList.clear();
            mergeFileList = openFileNames;
            ui->listWidget->clear();
            ui->listWidget->addItems(mergeFileList);
        }
    });

    //开始合并
    connect(ui->btnMergeStart, &QPushButton::clicked, this, [=](){
        if (mergeFileList.isEmpty())
            return;
        mergeLabel->setText("正在合并文件...");
        //解析源文件名
        QFileInfo info(mergeFileList.at(0));
        QString name = info.fileName();
        QStringList list = name.split(".");
        name.clear();
        for (int i = 0; i < list.size()-1; i++) {
            if (i != 0)
                name.append(".");
            name.append(list.at(i));
        }
        //创建线程合并文件
        MThread *thread = new MThread(1, name, mergeFileList, saveDir);
        connect(thread, &MThread::finished, this, [=](){
            thread->deleteLater();
            mergeLabel->setText(QString());
            ui->listWidget->addItem(QString("[完成]%1").arg(name));
        });
        thread->start();
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

