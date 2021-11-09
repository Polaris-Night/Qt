#ifndef MHOSTADDRESS_H
#define MHOSTADDRESS_H

#include <QObject>

class MHostAddress : public QObject
{
    Q_OBJECT
public:
    explicit MHostAddress(QObject *parent = nullptr);

    /**
     * @brief getActiveHostAddress
     * @details 获取本机活动IP
     * @return 本机活动IP
     */
    static QString getActiveHostAddress();

    /**
     * @brief getHostAddressTable
     * @details 获取本机IP列表
     * @return 本机IP列表
     */
    static QStringList getHostAddressTable();

    /**
     * @brief getArpAddress
     * @details 获取局域网地址列表
     * @return 局域网地址列表
     */
    static QStringList getArpAddress();

    /**
     * @brief getRegString
     * @details 由正则表达式获取字符串中所有匹配的字符串
     * @param str 字符串
     * @param reg 正则表达式
     * @return 匹配的字符串列表
     */
    static QStringList getRegString(const QString &str, const QString &reg);

signals:

};

#endif // MHOSTADDRESS_H
