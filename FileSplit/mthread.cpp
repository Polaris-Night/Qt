#include "mthread.h"
#include <QDebug>

MThread::MThread(QObject *parent): QThread(parent)
{
    this->flag = -1;
}

MThread::MThread(const int flag, const QString &name, const QStringList &list, const QString &dir, QObject *parent) : QThread(parent)
{
    this->flag = flag;
    this->fileName = name;
    this->fileList = list;
    this->saveDir = dir;
}

void MThread::run()
{
    if (flag == 0) {
        //分割文件
        //创建文件对象并获取文件信息
        QFile file(fileName);
        QFileInfo info(file);
        qint64 fileSize = info.size();
        qint64 splitSize[4];
        splitSize[0] = splitSize[1] = splitSize[2] = fileSize / 3;
        splitSize[3] = fileSize - splitSize[0]*3;
        //打开源文件
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << file.errorString();
            return;
        }
        //开始分割为4块文件
        for (int i = 0; i < 4; i++) {
            //创建分块文件
            QFile tempFile(QString("%1/%2.0%3").arg(saveDir).arg(info.fileName()).arg(i+1));
            if (!tempFile.open(QIODevice::WriteOnly)) {
                qDebug() << tempFile.errorString();
                return;
            }
            //预设文件大小，防止存储空间不足
            tempFile.resize(splitSize[i]);
            qint64 len = 0;
            char buffer[4*1024];
            //读取源文件并写入分块文件
            do {
                len = file.read(buffer, sizeof(buffer));
                len = tempFile.write(buffer, len);
                splitSize[i] -= len;
            } while (len > 0 && splitSize[i] > 0);
            //关闭分块文件
            tempFile.close();
        }
        //关闭源文件
        file.close();
    } else if (flag == 1) {
        //合并文件
        QFile file;
        //创建源文件
        QFile sourceFile(QString("%1/%2").arg(saveDir).arg(fileName));
        if (!sourceFile.open(QIODevice::WriteOnly)) {
            qDebug() << sourceFile.errorString();
            return;
        }
        //开始合并
        for (int i = 0; i < 4; i++) {
            //打开分块文件
            file.setFileName(fileList.at(i));
            if (!file.open(QIODevice::ReadOnly)) {
                qDebug() << file.errorString();
                return;
            }
            qint64 len = 0;
            char buffer[4*1024];
            //读取分块文件并写入源文件
            do {
                len = file.read(buffer, sizeof(buffer));
                len = sourceFile.write(buffer, len);
            } while (len > 0);
            //关闭分块文件
            file.close();
        }
        //关闭源文件
        sourceFile.close();
    }
}

