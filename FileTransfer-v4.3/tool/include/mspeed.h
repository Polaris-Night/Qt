#ifndef MSPEED_H
#define MSPEED_H

#include <QObject>

#define UNIT_KB 1024
#define UNIT_MB 1048576
#define UNIT_GB 1073741824

class MSpeed : public QObject
{
    Q_OBJECT
public:
    explicit MSpeed(QObject *parent = nullptr);
    ~MSpeed();

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

public slots:
    void clear();

private:
    qint64 speedSize;
    QStringList unitList;
};

#endif // MSPEED_H
