#include "serverresponsebuilder.h"

ServerResponseBuilder::ServerResponseBuilder()
{

}

ServerResponseBuilder::~ServerResponseBuilder()
{

}

QByteArray ServerResponseBuilder::buildBasicOkResponse(bool ok, qint8 flags){
    QByteArray result;
    QDataStream in(&result, QIODevice::WriteOnly);
    in << SERVER_RESPONSE_ID_OK;
    in << (bool) ok;
    in << (qint8) flags;
    return result;
}

QByteArray ServerResponseBuilder::buildFileDownloadResponse(bool ok, quint64 filesize){
    QByteArray result;
    QDataStream in(&result, QIODevice::WriteOnly);
    in << SERVER_RESPONSE_ID_DOWNLOAD;
    in << (bool) ok;
    in << (quint64) filesize;
    return result;
}

QByteArray ServerResponseBuilder::buildChangedFilesResponse(bool ok, int nChanges, quint64 revisionNumber, qint8 flags){
    QByteArray result;
    QDataStream in(&result, QIODevice::WriteOnly);
    in << SERVER_RESPONSE_ID_CHANGED_FILES;
    in << (bool) ok;
    in << (int) nChanges;
    in << (quint64) revisionNumber;
    in << (qint8) flags;
    return result;
}

