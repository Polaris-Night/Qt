#ifndef MCLOCK_H
#define MCLOCK_H

#include <QObject>
#include <QTime>
#include <QTimer>

class MClock : public QObject {
    Q_OBJECT
public:
    explicit MClock(QObject *parent = nullptr);
    ~MClock();

    /**
     * @brief getClock
     * @details 获取计时时间
     * @param format 时间格式
     * @return 时间
     */
    QString getClock(const QString &format) const;

public slots:
    /**
     * @brief start
     * @details 开始计时
     */
    void start();

    /**
     * @brief stop
     * @details 停止计时
     */
    void stop();

    /**
     * @brief clear
     * @details 停止计时并清零
     */
    void clear();

private:
    void timeOut();

    int hour;
    int minute;
    int second;
    QTime time;
    QTimer *timer;
};

#endif // MCLOCK_H
