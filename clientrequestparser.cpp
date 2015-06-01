#include "clientrequestparser.h"

ClientRequestParser::ClientRequestParser()
{

}

ClientRequestParser::~ClientRequestParser()
{

}

AuthenticationRequest ClientRequestParser::parseAuthenticationRequest(QByteArray request){
    AuthenticationRequest result;
    QDataStream out(request);
    quint8 reqid;
    out >> reqid;
    if(!reqid == REQID_AUTHENTICATION){
        result.parseSuccess = false;
    } else {
        result.parseSuccess = true;
        out >> result.email;
        out >> result.password;
        out >> result.machineId;
    }
    return result;
}

FileUploadRequest ClientRequestParser::parseFileUploadRequest(QByteArray request){
    FileUploadRequest result;
    QDataStream out(request);
    quint8 reqid;
    out >> reqid;
    if(!(reqid == REQID_FILE_UPLOAD)){
        result.parseSuccess = false;
    } else {
        result.parseSuccess = true;
        out >> result.filesize;
        out >> result.directory;
        out >> result.filename;
    }
    return result;
}

FileDownloadRequest ClientRequestParser::parseFileDownloadRequest(QByteArray request){
    FileDownloadRequest result;
    QDataStream out(request);
    quint8 reqid;
    out >> reqid;
    if(!(reqid == REQID_FILE_DOWNLOAD)){
        result.parseSuccess = false;
    } else {
        result.parseSuccess = true;
        out >> result.directory;
        out >> result.filename;
    }
    return result;
}

FileDeletionRequest ClientRequestParser::parseFileDeletionRequest(QByteArray request){
    FileDeletionRequest result;
    QDataStream out(request);
    quint8 reqid;
    out >> reqid;
    if(!(reqid == REQID_FILE_DELETION)){
        result.parseSuccess = false;
    } else {
        result.parseSuccess = true;
        out >> result.directory;
        out >> result.filename;
    }
    return result;
}

FileChangesRequest ClientRequestParser::parseFileChangesRequest(QByteArray request){
    FileChangesRequest result;
    QDataStream out(request);
    quint8 reqid;
    out >> reqid;
    if(!(reqid == REQID_FILE_CHANGES)){
        result.parseSuccess = false;
    } else {
        result.parseSuccess = true;
        out >> result.machineId;
        out >> result.revisionNumber;
    }
    return result;
}

quint8 ClientRequestParser::getRequestId(QByteArray request){
    return (quint8) request.at(0);
}

bool ClientRequestParser::validate(QByteArray request){
    if(request.size() < 1){
        return false;
    }
    return true;
}

