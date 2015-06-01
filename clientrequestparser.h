#ifndef CLIENTREQUESTPARSER_H
#define CLIENTREQUESTPARSER_H

#include <QtGlobal>
#include <QString>
#include <QDataStream>
#include <QByteArray>

#define REQID_AUTHENTICATION    (quint8)0x01
#define REQID_FILE_UPLOAD       (quint8)0x02
#define REQID_FILE_DOWNLOAD     (quint8)0x03
#define REQID_FILE_DELETION     (quint8)0x04
#define REQID_FILE_CHANGES      (quint8)0x05

typedef struct {
    bool parseSuccess;
    QString email;
    QString password;
    int machineId;
} AuthenticationRequest;

typedef struct {
    bool parseSuccess;
    quint64 filesize;
    QString directory;
    QString filename;
} FileUploadRequest;

typedef struct {
    bool parseSuccess;
    QString directory;
    QString filename;
} FileDownloadRequest;

typedef struct {
    bool parseSuccess;
    QString directory;
    QString filename;
} FileDeletionRequest;

typedef struct {
    bool parseSuccess;
    int machineId;
    int revisionNumber;
} FileChangesRequest;

class ClientRequestParser
{
public:
    ClientRequestParser();
    ~ClientRequestParser();

    static AuthenticationRequest parseAuthenticationRequest(QByteArray request);
    static FileUploadRequest parseFileUploadRequest(QByteArray request);
    static FileDownloadRequest parseFileDownloadRequest(QByteArray request);
    static FileDeletionRequest parseFileDeletionRequest(QByteArray request);
    static FileChangesRequest parseFileChangesRequest(QByteArray request);

    static quint8 getRequestId(QByteArray request);
    static bool validate(QByteArray request);
private:

};

#endif // CLIENTREQUESTPARSER_H
