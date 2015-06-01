#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <QRunnable>
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "clientrequestparser.h"
#include "serverresponsebuilder.h"
#include "serverdatabase.h"

#define DEFAULT_TIMEOUT 3000
#define BUFFER_SIZE 1024 * 64

class ConnectionHandler : public QRunnable
{
public:
    ConnectionHandler(qintptr handle, ServerDatabase *database);
    ~ConnectionHandler();
protected:
    void run();
private:
    ServerDatabase *database;
    QTcpSocket* socket;
    qintptr sockid;
    int userId, machineId;

    bool initSocket();

    bool handleAuthentication();

    bool handleMessage();

    bool receiveFile(FileUploadRequest request);
    bool sendFile(FileDownloadRequest request);
    bool deleteFile(FileDeletionRequest request);

    bool getFileChanges(FileChangesRequest request);

    QString generateFilePath(QString directory, QString filename);

};

#endif // CONNECTIONHANDLER_H
