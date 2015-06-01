#ifndef SERVERRESPONSEBUILDER_H
#define SERVERRESPONSEBUILDER_H

#include <QtGlobal>
#include <QDataStream>

#define SERVER_RESPONSE_ID_OK (quint8)0x01
#define SERVER_RESPONSE_ID_DOWNLOAD (quint8)0x02
#define SERVER_RESPONSE_ID_CHANGED_FILES (quint8)0x03

class ServerResponseBuilder
{
public:
    ServerResponseBuilder();
    ~ServerResponseBuilder();

    static QByteArray buildBasicOkResponse(bool ok, qint8 = 0);
    static QByteArray buildFileDownloadResponse(bool ok, quint64 filesize = 0);
    static QByteArray buildChangedFilesResponse(bool ok, int nChanges = 0, quint64 revisionNumber = 0, qint8 flags = 0);
};

#endif // SERVERRESPONSEBUILDER_H
