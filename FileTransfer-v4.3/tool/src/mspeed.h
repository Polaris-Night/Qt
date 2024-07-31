#ifndef MSPEED_H
#define MSPEED_H

#include <QString>
#include <QStringList>

class MSpeed {
public:
    /**
     * @brief addSize
     * @details 增加大小
     * @param size 增加大小值
     */
    void addSize(const qint64 &size);

    /**
     * @brief getSpeedSize
     * @details 获取速度
     * @return 带单位的速度字符串
     */
    QString getSpeedSize();

    void clear();

private:
    qint64 m_speedSize{};
    QStringList unitList{"B/s", "KB/s", "MB/s", "GB/s"};
};

#endif // MSPEED_H
