#include "mlogmanager.h"
#include <QDateTime>
#include <QTextStream>
#include <QDebug>
#include <QDir>

QMutex MLogManager::logMutex;
QFile  MLogManager::logFile("./log/clientLog.txt");

MLogManager::MLogManager(QObject *parent) : QObject(parent)
{

}

void MLogManager::outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    //输出类型
    QString typeText;
    //获取当前时间
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString currentDate = QString("%1").arg(currentDateTime);
    //打印所在文件和行
    QString contextInfo = QString("File:(%1) Line(%2)").arg(context.file).arg(context.line);
    //判断日志存储目录是否存在，不存在则创建
    if (!QDir().exists("./log"))
        QDir().mkdir("./log");
    logMutex.lock();//加锁
    switch (type) {
        case QtDebugMsg:
            typeText = QString("Debug:");
            break;
        case QtInfoMsg:
            typeText = QString("Info:");
            break;
        case QtWarningMsg:
            typeText = QString("Warning:");
            break;
        case QtCriticalMsg:
            typeText = QString("Critical:");
            break;
        case QtFatalMsg:
            typeText = QString("Fatal:");
            break;
    }
    QString message = QString("[%1] [%2 %3] [%4]").arg(currentDate).arg(typeText).arg(contextInfo).arg(msg);
    if (!logFile.open(QIODevice::Append | QIODevice::WriteOnly)) {
        logMutex.unlock();
        return;
    }
    QTextStream textStream(&logFile);
    textStream << message << "\r\n";
    logFile.flush();
    logFile.close();
    logMutex.unlock();//解锁
}

void MLogManager::spaceLine()
{
    qDebug() << "=======================================================";
}
