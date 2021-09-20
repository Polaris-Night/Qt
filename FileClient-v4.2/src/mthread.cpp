#include "mthread.h"
#include <QDebug>

MThread::MThread(QObject *parent) : QThread(parent)
{

}

MThread::~MThread()
{

}

void MThread::run()
{
    connect(this, &MThread::finished, this, &MThread::deleteLater);
    exec();
}
