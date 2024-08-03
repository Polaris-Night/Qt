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
#include "worker.h"

/***********************************主线程****************************************/
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

private slots:
    void updateSendButtonState();

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

    Ui::MainWindow *ui{};

    MUdpBroadcast *m_broadcastSocket{};
    QSettings *m_config{};   // 配置
    QTimer *m_speedTimer{};  // 速度定时器
    QLabel *m_speedLabel{};  // 速度标签
    Worker *m_worker{};      // 文件处理器
    MDialog *m_helpDialog{}; // 帮助对话框
    quint16 m_port{};        // 端口
    QString theme;           // 主题
    QStringList m_fileList;  // 文件列表

signals:
    void goToSendFile(QStringList fileList); //发送文件信号
};
#endif // MAINWINDOW_H
