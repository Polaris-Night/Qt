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
#include "worker.h"

/************************************主线程***************************************/
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
} // namespace Ui
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
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
     * @brief loadHelpText
     * @details 加载帮助文本
     * @return 帮助文本内容
     */
    QString loadHelpText();

    /**
     * @brief initVariable
     * @details 初始化变量值
     */
    void initVariable();

    /**
     * @brief initConfig
     * @details 加载配置
     */
    void initConfig();

    /**
     * @brief initSignalHandle
     * @details 初始化信号处理
     */
    void initSignalHandle();

private:
    /**
     * @brief The FirewallMode enum
     * @details 防火墙规则枚举类型
     * @param ENABLE  开启
     * @param DISABLE 关闭
     */
    enum FirewallMode { ENABLE, DISABLE };

    /**
     * @brief setFirewall
     * @details 设置防火墙规则
     * @param mode 模式--关闭/开启
     */
    void setFirewall(FirewallMode mode);

    Ui::MainWindow *ui;

    MUdpBroadcast *m_broadcastSocket{};
    QSettings *m_config{};   //配置文件
    MServer *m_server{};     //服务端监听socket
    quint16 m_port;          //监听端口
    QString theme;           //主题
    MDialog *m_helpDialog{}; //帮助
    MClock m_clock;          //时钟
    QTimer *m_timer{};       //定时器
    QLabel *m_ipLabel{};     //本机地址标签
    Worker *m_worker{};

private slots:
    /**
     * @brief readyConnect
     * @param socket
     */
    void readyConnect(qintptr socket); //连接客户端

    /**
     * @brief updateSpeedAndTime
     * @details 更新速度及时间
     */
    void updateSpeedAndTime();
};

class MListWidgetItem : public QObject, public QListWidgetItem {
    Q_OBJECT
public:
    explicit MListWidgetItem(QListWidget *wparent = nullptr, int type = Type, QObject *oparent = nullptr)
    : QObject(oparent)
    , QListWidgetItem(wparent, type) { }

public slots:
    void setMText(const QString &text) { setText(text); }
};

#endif // MAINWINDOW_H
