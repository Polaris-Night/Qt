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

    static void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void spaceLine();

    static QMutex logMutex;
    static QFile logFile;
signals:

};

#endif // MLOGMANAGER_H
