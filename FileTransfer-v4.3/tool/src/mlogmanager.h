#ifndef MLOGMANAGER_H
#define MLOGMANAGER_H

#include <QObject>
#include <QMutex>
#include <QFile>

class MLogManager : public QObject
{
    Q_OBJECT
public:
    explicit MLogManager(QObject *parent = nullptr);

    /*
     * qInstallMessageHandler(MLogManager::outputMessage);
    */

    static void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    /**
     * @brief setLogFile
     * @details 设置日志文件
     * @param fileName 日志文件名
     * @param filePath 日志文件路径
     */
    static void setLogFile(const QString &fileName = "log.txt", const QString &fileDir = "./log");

    /**
     * @brief getLogFilePath
     * @details 获取日志文件目录
     * @return 日志文件目录
     */
    static QString getLogFileDir();

    /**
     * @brief getLogFileName
     * @details 获取日志文件名
     * @return 日志文件名
     */
    static QString getLogFileName();

    /**
     * @brief spaceLine
     * @details 打印分隔行
     */
    static void spaceLine();

private:
    static QMutex logMutex;
    static QString logFileDir;
    static QString logFileName;
    static QFile logFile;
signals:

};

#endif // MLOGMANAGER_H
