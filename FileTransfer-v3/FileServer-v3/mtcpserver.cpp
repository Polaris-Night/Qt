#include "mtcpserver.h"

MTcpServer::MTcpServer(QObject *parent) : QTcpServer(parent)
{

}

void MTcpServer::incomingConnection(qintptr socketDescriptor)
{
    emit haveNewConnect(socketDescriptor);
}
