#include "mspeed.h"

MSpeed::MSpeed(QObject *parent)
    : QObject(parent)
    , speedSize(0), unitList({"B/s", "KB/s", "MB/s", "GB/s"})
{
}

MSpeed::~MSpeed()
{
}

void MSpeed::addSize(const qint64 &size)
{
    speedSize += size;
}

QString MSpeed::getSpeedSize()
{
    double speed = static_cast<double>(speedSize);//B
    speedSize = 0;
    int index;
    if (speed < UNIT_KB) {
        //B
        index = 0;
    } else if (speed < UNIT_MB) {
        //KB
        index = 1;
        speed /= UNIT_KB;
    } else if (speed < UNIT_GB){
        //MB
        index = 2;
        speed /= UNIT_MB;
    } else {
        //GB
        index = 3;
        speed /= UNIT_GB;
    }
    return QString("%1%2").arg(QString::number(speed, 'f', 2)).arg(unitList.at(index));
}

void MSpeed::clear()
{
    speedSize = 0;
}
