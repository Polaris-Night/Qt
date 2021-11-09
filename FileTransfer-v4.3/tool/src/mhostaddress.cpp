#include "mhostaddress.h"

#include <QProcess>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QList>

MHostAddress::MHostAddress(QObject *parent)
    : QObject(parent)
{
}

#ifdef Q_OS_WIN
QString MHostAddress::getActiveHostAddress()
{
    QString ip;
    QProcess cmdProc;
    QString cmdStr = QString("ipconfig");
    cmdProc.start("cmd.exe", QStringList() << "/c" << cmdStr);
    cmdProc.waitForStarted();
    cmdProc.waitForFinished();
    QString result = cmdProc.readAll();
    QString pattern("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    QRegExp rx(pattern);

    int pos = 0;
    while((pos = rx.indexIn(result, pos)) != -1){
        QString temp = rx.cap(0);
        //跳过子网掩码 eg:255.255.255.0
        if(-1 == temp.indexOf("255")) {
            //ip不为空且存在网关，则返回
            if (!ip.isEmpty() && -1 != temp.indexOf(ip.midRef(0, ip.lastIndexOf("."))))
                break;
            ip = temp;
        }
        pos += rx.matchedLength();
    }
    return ip;
}
#else
QString MHostAddress::getActiveHostAddress() {return QString();}
#endif

QStringList MHostAddress::getHostAddressTable()
{
    QList<QHostAddress> allAddress = QNetworkInterface::allAddresses();
    QStringList ipTable;
    foreach (QHostAddress address, allAddress) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol
                && !address.isLoopback())
            ipTable.append(address.toString());
    }
    return ipTable;
}

#ifdef Q_OS_WIN
QStringList MHostAddress::getArpAddress()
{
    //ipv4正则表达式
    QString reg("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    //cmd命令
    QString cmdStr = QString("arp -a -N %1\r\n").arg(getActiveHostAddress());
    //cmd进程
    QProcess cmdProc;
    //"/c"表示执行完命令后退出
    cmdProc.start("cmd.exe", QStringList() << "/c" << cmdStr);
    cmdProc.waitForStarted();
    cmdProc.waitForFinished();
    //读取数据
    QString result = cmdProc.readAll();
    return getRegString(result, reg);
}
#else
QStringList MHostAddress::getArpAddress() {return QStringList();}
#endif

QStringList MHostAddress::getRegString(const QString &str, const QString &reg)
{
    //匹配字符串列表
    QStringList matchList;
    //正则表达式
    QRegExp rx(reg);
    //匹配起始位置
    int matchPos = 0;
    //匹配
    while ((matchPos = rx.indexIn(str, matchPos)) != -1) {
        //获取匹配长度
        int matchLen = rx.matchedLength();
        //截取匹配的字符串到列表
        matchList.append(str.mid(matchPos,matchLen));
        //起始位置偏移
        matchPos += matchLen;
    }
    return matchList;
}
