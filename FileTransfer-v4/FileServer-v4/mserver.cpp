#include "mserver.h"

MServer::MServer(QObject *parent) : QTcpServer(parent)
{

}

void MServer::incomingConnection(qintptr socketDescriptor)
{
    emit haveNewConnect(socketDescriptor);
}
