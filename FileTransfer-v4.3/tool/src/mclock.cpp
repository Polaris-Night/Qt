#include "mclock.h"

MClock::MClock(QObject *parent)
    : QObject(parent), hour(0), minute(0), second(0)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MClock::timeOut);
    time.setHMS(hour, minute, second);
}

MClock::~MClock()
{
    timer->stop();
    timer->deleteLater();
}

QString MClock::getClock(const QString &format) const
{
    return time.toString(format);
}

void MClock::start()
{
    timer->start(1000);
}

void MClock::stop()
{
    timer->stop();
}

void MClock::clear()
{
    timer->stop();
    hour = minute = second = 0;
    time.setHMS(hour, minute, second);
}

void MClock::timeOut()
{
    if ((++second) == 60) {
        second = 0;
        if ((++minute) == 60) {
            minute = 0;
            ++hour;
        }
    }
    time.setHMS(hour, minute, second);
}
