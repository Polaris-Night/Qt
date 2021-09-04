#ifndef MTHREAD_H
#define MTHREAD_H

#include <QThread>
#include <QFile>
#include <QFileInfo>

class MThread : public QThread
{
    Q_OBJECT
public:
    explicit MThread(QObject *parent = nullptr);
    explicit MThread(int flag, const QString &name, const QStringList &list, const QString &dir, QObject *parent = nullptr);

    void run() override;

private:
    QString fileName;
    QStringList fileList;

    QString saveDir;
    int flag;
signals:

};

#endif // MTHREAD_H
