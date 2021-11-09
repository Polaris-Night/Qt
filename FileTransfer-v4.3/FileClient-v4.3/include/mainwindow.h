#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QStringList>
#include <QTextBrowser>
#include <QSettings>

#include "mdialog.h"
#include "mudpbroadcast.h"

/***********************************主线程****************************************/
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
     * @brief loadHelpText
     * @details 加载帮助文档
     * @return 帮助文档内容
     */
    QString loadHelpText();

    /**
     * @brief initVariable
     * @details 初始化变量值
     */
    void initVariable();

public slots:
    /**
     * @brief toConnect
     * @details 连接服务端
     */
    void toConnect(const QString &ip, const quint16 &port);

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
    QSettings *configSetting;//配置
    quint16 port;//端口
    QString theme;//主题
    QTimer *speedTimer;//速度定时器
    QTimer *scanTimer;//文件扫描定时器
    MDialog *helpDialog;//帮助对话框
    QStringList fileList;//文件列表
    bool isConnect;//连接标志
    bool isStartSend;//开始发送标志
    QLabel *statusLabel;//状态标签
    QLabel *speedLabel;//速度标签

signals:
    void goToDisconnect();//连接信号
    void goToSendFile(QStringList fileList);//发送文件信号
};
#endif // MAINWINDOW_H
