#ifndef MTHREAD_H
#define MTHREAD_H

#include <QThread>

class MThread : public QThread
{
    Q_OBJECT
public:
    explicit MThread(QObject *parent = nullptr);
    ~MThread();

protected:
    void run() override;

signals:

};

#endif // MTHREAD_H
