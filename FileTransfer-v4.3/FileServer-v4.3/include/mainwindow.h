#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QListWidgetItem>
#include <QTextBrowser>
#include <QProcess>
#include <QSettings>

#include "mserver.h"
#include "mclock.h"
#include "mdialog.h"
#include "mudpbroadcast.h"

/************************************主线程***************************************/
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @brief setSaveDir
     * @details 设置保存目录
     * @param dir 保存目录
     */
    void setSaveDir(const QString &dir);

    /**
     * @brief initVariable
     * @details 初始化变量
     */
    void initVariable();

    /**
     * @brief loadHelpText
     * @details 加载帮助文本
     * @return 帮助文本内容
     */
    QString loadHelpText();

private:
    /**
     * @brief The FirewallMode enum
     * @details 防火墙规则枚举类型
     * @param ENABLE  开启
     * @param DISABLE 关闭
     */
    enum FirewallMode {
        ENABLE,
        DISABLE
    };

    /**
     * @brief setFirewall
     * @details 设置防火墙规则
     * @param mode 模式--关闭/开启
     */
    void setFirewall(FirewallMode mode);

    Ui::MainWindow *ui;

    MUdpBroadcast *broadcastSock;
    QSettings *configSetting;//配置文件
    MServer *server;//服务端监听socket
    quint16 port;//监听端口
    QString theme;//主题
    MDialog *helpDialog;//帮助
    MClock clock;//时钟
    QTimer *mtimer;//定时器
    QLabel *ipLabel;//本机地址标签

private slots:
    /**
     * @brief readyConnect
     * @param socket
     */
    void readyConnect(qintptr socket);//连接客户端

    /**
     * @brief updateSpeedAndTime
     * @details 更新速度及时间
     */
    void updateSpeedAndTime();

signals:
    void goToDisconnectThis(QListWidgetItem *delItem);//断开选中项目连接信号
    void goToDisconnect();//主动断开连接信号
    void goToDisconnectAll();//断开所有连接信号
};

class MListWidgetItem : public QObject, public QListWidgetItem
{
    Q_OBJECT
public:
    explicit MListWidgetItem(QListWidget *wparent = nullptr, int type = Type, QObject *oparent = nullptr)
    : QObject(oparent), QListWidgetItem(wparent, type)
    {}

public slots:
    void setMText(const QString &text)
    {
        setText(text);
    }
};

#endif // MAINWINDOW_H
