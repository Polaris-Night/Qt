#include "mspeed.h"

#define UNIT_KB 1024
#define UNIT_MB 1048576
#define UNIT_GB 1073741824

void MSpeed::addSize(const qint64 &size) { m_speedSize += size; }

QString MSpeed::getSpeedSize() {
    double speed = static_cast<double>(m_speedSize); //B
    m_speedSize = 0;
    int index;
    if (speed < UNIT_KB) {
        //B
        index = 0;
    } else if (speed < UNIT_MB) {
        //KB
        index = 1;
        speed /= UNIT_KB;
    } else if (speed < UNIT_GB) {
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

void MSpeed::clear() { m_speedSize = 0; }
