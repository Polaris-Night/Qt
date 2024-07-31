#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QStringList>
#include <QVector>
#include <QThread>
#include <QMutex>
#include <QTimer>

#include "mprogress.h"
#include "mspeed.h"

/**********************************工作线程类***************************************/
class Worker : public QObject {
    Q_OBJECT
public:
    explicit Worker(const QString &ip, const quint16 &port, QObject *parent = nullptr);
    ~Worker();

    void setIp(const QString &ip);
    void setPort(quint16 port);
    QString getIp() const;
    quint16 getPort() const;

    QString getSpeed();

public slots:
    void connectHost();
    void disconnectHost();
    void sendFile(const QStringList &fileList);

private slots:
    /**
     * @brief _connect
     * @details 连接服务端
     */
    void _connect();

    /**
     * @brief _disconnect
     * @details 断开与服务端的连接
     */
    void _disconnect();

    /**
     * @brief _send
     * @details 发送文件
     * @param fileList 文件列表
     */
    void _send(const QStringList &fileList);

signals:
    void connectResult(QString);  //连接结果信号
    void overConnect();           //结束连接信号
    void updateSendProgress(int); //更新进度条信号, 0~100
    void sendFinish();            //全部文件发送完成信号

private:
    QThread *m_thread{};    //线程
    QTcpSocket *m_socket{}; //socket
    QString m_ip;           //ip
    quint16 m_port{};       //端口
    MProgress m_progress;   // 进度计算器
    MSpeed m_speed;
    QTimer *m_speedTimer{};
};
#endif // MWORK_H
