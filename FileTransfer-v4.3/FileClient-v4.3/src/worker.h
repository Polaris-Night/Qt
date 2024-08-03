#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QStringList>

#include "mprogress.h"
#include "mspeed.h"

/**********************************工作线程类***************************************/
class Worker : public QObject {
    Q_OBJECT
public:
    enum State {
        Inactive = 0, // 空闲
        Sending,      // 发送中
    };

    explicit Worker(QObject *parent = nullptr);
    Worker(const QString &ip, const quint16 &port, QObject *parent = nullptr);
    ~Worker();

    void setIp(const QString &ip);
    void setPort(quint16 port);
    QString getIp() const;
    quint16 getPort() const;
    State getState() const;

    QString getSpeed();

public slots:
    void sendFile(const QStringList &fileList);

signals:
    void stateChanged(State);      // 状态变更信号
    void sendProgressChanged(int); // 更新进度条信号, 0~100
    void sendFinish();             // 全部文件发送完成信号

private slots:
    /**
     * @brief _send
     * @details 发送文件
     * @param fileList 文件列表
     */
    void _send(const QStringList &fileList);

    void setState(State state);

private:
    QString m_ip;                   // ip
    quint16 m_port{};               // 端口
    State m_state{State::Inactive}; // 发送状态
    MProgress m_progress;           // 进度计算器
    MSpeed m_speed;                 // 速度计算器
};
#endif // MWORK_H
