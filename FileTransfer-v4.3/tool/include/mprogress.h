#ifndef MPROGRESS_H
#define MPROGRESS_H

#include <QObject>

class MProgress : public QObject
{
    Q_OBJECT
public:
    explicit MProgress(QObject *parent = nullptr);
    MProgress(const qint64 &size, QObject *parent = nullptr);
    MProgress(const MProgress &prog, QObject *parent = nullptr);
    ~MProgress();

    /**
     * @brief setTotalSize
     * @details 设置总大小
     * @param size 总大小
     */
    void setTotalSize(const qint64 &size);

    /**
     * @brief setCurrentSize
     * @details 设置当前大小
     * @param size 当前大小
     */
    void setCurrentSize(const qint64 &size);

    /**
     * @brief addSize
     * @details 增加大小
     * @param size 增加大小值
     */
    void addSize(const qint64 &size);

    /**
     * @brief getProgress
     * @details 获取进度值
     * @return 进度值
     */
    int getProgress() const;

    /**
     * @brief getTotalSize
     * @details 获取总大小
     * @return 总大小
     */
    qint64 getTotalSize() const;

    /**
     * @brief getCurrentSize
     * @details 获取当前大小
     * @return 当前大小
     */
    qint64 getCurrentSize() const;

public slots:
    void clear();

private:
    qint64 totalSize;
    qint64 currentSize;
};

#endif // MPROGRESS_H
