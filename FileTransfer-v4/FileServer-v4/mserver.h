#ifndef MSERVER_H
#define MSERVER_H

#include <QTcpServer>

class MServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MServer(QObject *parent = nullptr);

protected:
    virtual void incomingConnection(qintptr socketDescriptor);

signals:
    void haveNewConnect(qintptr socket);
};

#endif // MSERVER_H
