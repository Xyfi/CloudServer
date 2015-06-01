#include "server.h"

Server::Server(QObject *parent) : QTcpServer(parent)
{
    database = new ServerDatabase();
    database->init();
    pool = new QThreadPool(this);
    pool->setMaxThreadCount(10);
}

Server::~Server()
{
    database->close();
    delete database;
    delete pool;
}

void Server::start(){
    if(!listen(QHostAddress::Any, PORT)){
        qDebug() << "Listen failed.";
        return;
    }
    qDebug("Server started...");
}

void Server::incomingConnection(qintptr handle)
{
    ConnectionHandler *handler = new ConnectionHandler(handle, database);
    pool->start(handler);
    qDebug() << "New connection";
}
