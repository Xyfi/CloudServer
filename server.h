#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QThreadPool>

#include "connectionhandler.h"
#include "serverdatabase.h"

#define PORT 1337

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    void start();
    ~Server();
protected:
    void incomingConnection(qintptr handle);
private:
    ServerDatabase *database;
    QThreadPool*    pool;


signals:

public slots:

};

#endif // SERVER_H
