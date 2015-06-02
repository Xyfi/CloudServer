#include "connectionhandler.h"

ConnectionHandler::ConnectionHandler(qintptr sockid, ServerDatabase *database) :
    database(database)
{
    this->sockid = sockid;
    setAutoDelete(true);
}

ConnectionHandler::~ConnectionHandler()
{
    delete socket;
}

void ConnectionHandler::run()
{
    if(!initSocket()){
        qDebug() << "Initializing socket failed.";
        return;
    }
    if(!handleAuthentication()){
        qDebug() << "Authentication failed.";
        return;
    }
    if(!handleMessage()){
        qDebug() << "Handling message failed";
    }
    socket->close();
}

bool ConnectionHandler::initSocket(){
    socket = new QTcpSocket;
    return socket->setSocketDescriptor(sockid);
}

bool ConnectionHandler::handleAuthentication(){
    socket->waitForReadyRead(DEFAULT_TIMEOUT);
    AuthenticationRequest userData =
            ClientRequestParser::parseAuthenticationRequest(socket->readAll());
    bool authSuccess;
    QString password;
    if(!userData.parseSuccess){
        authSuccess = false;
    } else if (!database->getUserDetails(userData.email, userData.machineId, &password, &userId)){
        authSuccess = false;
    } else if (!userData.password.compare(password) == 0) {
        authSuccess = false;
    } else {
        authSuccess = true;
        machineId = userData.machineId;
    }
    socket->write(ServerResponseBuilder::buildBasicOkResponse(authSuccess));
    socket->waitForBytesWritten(DEFAULT_TIMEOUT);
    return authSuccess;
}

bool ConnectionHandler::handleMessage(){
    socket->waitForReadyRead(DEFAULT_TIMEOUT);
    QByteArray request = socket->readAll();
    if(!ClientRequestParser::validate(request)){
        socket->close();
        return false;
    }
    quint8 request_id = ClientRequestParser::getRequestId(request);
    switch(request_id){
    case REQID_FILE_UPLOAD:
        qDebug() << "Client request to upload a file.";
        return receiveFile(ClientRequestParser::parseFileUploadRequest(request));
        break;
    case REQID_FILE_DOWNLOAD:
        qDebug() << "Client request to download a file.";
        return sendFile(ClientRequestParser::parseFileDownloadRequest(request));
        break;
    case REQID_FILE_DELETION:
        qDebug() << "Client request to delete a file.";
        return deleteFile(ClientRequestParser::parseFileDeletionRequest(request));
        break;
    case REQID_FILE_CHANGES:
        qDebug() << "Client requests file changes.";
        return getFileChanges(ClientRequestParser::parseFileChangesRequest(request));
        break;
    default:
        // Invalid message type
        return false;
        break;
    }
}

bool ConnectionHandler::receiveFile(FileUploadRequest request){
    // Send response
    if(!request.parseSuccess){
        qDebug() << "[receiveFile] Parsing failed.";
        socket->write(ServerResponseBuilder::buildBasicOkResponse(false));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        return false;
    } else if (!database->lockFile(request.directory, request.filename, userId)) {
        qDebug() << "[receiveFile] Locking file failed.";
        socket->write(ServerResponseBuilder::buildBasicOkResponse(false));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        return false;
    }
    socket->write(ServerResponseBuilder::buildBasicOkResponse(true));
    socket->waitForBytesWritten(DEFAULT_TIMEOUT);

    // Create file & folder(s)
    QString filePath = generateFilePath(request.directory, request.filename);
    QFile file(filePath);
    QFileInfo info(file);
    QDir().mkpath(info.path());

    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        qDebug() << "Error while opening file.";
        return false;
    }
    qDebug() << "Starting download" << info.filePath() << request.filesize << "bytes.";

    // Receive file

    char data[BUFFER_SIZE];
    quint64 receivedTotal = 0;
    quint64 received;
    while(receivedTotal < request.filesize){
        if(!socket->bytesAvailable()){
            socket->waitForReadyRead(DEFAULT_TIMEOUT);
        }
        received = socket->read(data, BUFFER_SIZE);
        receivedTotal += received;
        file.write(data, received);
    }
    file.close();
    database->updateAndUnlockFile(request.directory, request.filename, userId, machineId);

    // Send response;
    socket->write(ServerResponseBuilder::buildBasicOkResponse(true));
    socket->waitForBytesWritten(DEFAULT_TIMEOUT);
    qDebug() << "Response written";
    return true;
}

bool ConnectionHandler::sendFile(FileDownloadRequest request){
    if(!request.parseSuccess){
        qDebug() << "[sendFile] Parsing failed.";
        socket->write(ServerResponseBuilder::buildBasicOkResponse(false));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        return false;
    }
    QString filepath = generateFilePath(request.directory, request.filename);
    QFile file(filepath);
    if(!file.exists()){
        qDebug() << "[sendFile] Requested file not found";
        socket->write(ServerResponseBuilder::buildFileDownloadResponse(false));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        socket->close();
        return false;
    }
    QFileInfo info(file);
    socket->write(ServerResponseBuilder::buildFileDownloadResponse(true, info.size()));
    socket->waitForBytesWritten(DEFAULT_TIMEOUT);
    file.open(QIODevice::ReadOnly);
    char data[BUFFER_SIZE];
    quint64 sendTotal = 0;
    quint64 send;
    while((send = file.read(data, BUFFER_SIZE)) > 0){
        sendTotal += send;
        socket->write(data, send);
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
    }
    return true;
}

bool ConnectionHandler::deleteFile(FileDeletionRequest request){
    // Send response
    if(!request.parseSuccess){
        qDebug() << "[deleteFile] Parsing failed.";
        socket->write(ServerResponseBuilder::buildBasicOkResponse(false));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        return false;
    } else if (!database->lockFile(request.directory, request.filename, userId)) {
        qDebug() << "[deleteFile] Locking file failed.";
        socket->write(ServerResponseBuilder::buildBasicOkResponse(false));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        return false;
    }
    QString filepath = generateFilePath(request.directory, request.filename);
    QFile file(filepath);
    if(!file.exists()){
        qDebug() << "[deleteFile] File requested for deletion not found.";
        socket->write(ServerResponseBuilder::buildBasicOkResponse(true));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        socket->close();
        return true;
    }
    if(!file.remove()){
        qDebug() << "[deleteFile] Deleting file failed.";
        socket->write(ServerResponseBuilder::buildBasicOkResponse(false));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        socket->close();
        return false;
    }
    socket->write(ServerResponseBuilder::buildBasicOkResponse(true));
    socket->waitForBytesWritten(DEFAULT_TIMEOUT);
    socket->close();
    database->updateAndUnlockFile(request.directory, request.filename, userId, machineId, true);
    return true;
}

bool ConnectionHandler::getFileChanges(FileChangesRequest request){
    if(!request.parseSuccess){
        qDebug() << "[getFileChanges] Parsing failed.";
        socket->write(ServerResponseBuilder::buildChangedFilesResponse(false));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        return false;
    }
    ChangesList list = database->getChangesList(machineId, request.revisionNumber);
    socket->write(ServerResponseBuilder::buildChangedFilesResponse(true, list.nChanges, list.revisionNumber));
    socket->waitForBytesWritten(DEFAULT_TIMEOUT);
    while(list.changes.size() > 0){
        qDebug() << socket->write(list.changes.mid(0, BUFFER_SIZE));
        socket->waitForBytesWritten(DEFAULT_TIMEOUT);
        list.changes.remove(0, BUFFER_SIZE);
    }
    return true;
}

QString ConnectionHandler::generateFilePath(QString directory, QString filename){
    QString filepath = QString::number(userId) + "/";
    if(!(directory.compare(".") == 0)){
        filepath.append(directory + "/");
    }
    filepath.append(filename);
    return filepath;
}

