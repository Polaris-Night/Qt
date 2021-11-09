#ifndef MTCPSERVER_H
#define MTCPSERVER_H

#include <QTcpServer>

class MTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MTcpServer(QObject *parent = nullptr);

protected:
    //重写
    virtual void incomingConnection(qintptr socketDescriptor);


signals:
    void haveNewConnect(qintptr socket);

};

#endif // MTCPSERVER_H
