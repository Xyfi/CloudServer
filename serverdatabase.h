#ifndef SERVERDATABASE_H
#define SERVERDATABASE_H

#include <QObject>
#include <QtSql>
#include <QFile>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

#define DB_NAME "server.db"
#define DB_TYPE "QSQLITE"

#define TABLE_USERS "CREATE TABLE Users("\
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"\
    "email TEXT NOT NULL UNIQUE,"\
    "password TEXT NOT NULL"\
")"

#define TABLE_USER_MACHINES "CREATE TABLE UserMachines("\
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"\
    "userId INTEGER NOT NULL,"\
    "FOREIGN KEY(userId) REFERENCES Users(id) ON DELETE NO ACTION ON UPDATE NO ACTION"\
")"

#define TABLE_FILES "CREATE TABLE Files("\
    "directory TEXT NOT NULL,"\
    "filename TEXT NOT NULL,"\
    "lastUpdatedBy INTEGER,"\
    "ownedBy INTEGER NOT NULL,"\
    "locked NUMBER NOT NULL DEFAULT 0,"\
    "revisionNumber NUMBER DEFAULT 0,"\
    "deleted NUMBER DEFAULT 0,"\
    "PRIMARY KEY(directory, filename),"\
    "FOREIGN KEY(lastUpdatedBy) REFERENCES UserMachines(id) ON DELETE NO ACTION ON UPDATE NO ACTION,"\
    "FOREIGN KEY(ownedby) REFERENCES Users(id) ON DELETE NO ACTION ON UPDATE NO ACTION"\
")"

typedef struct{
    quint64 revisionNumber;
    int nChanges;
    QByteArray changes;
} ChangesList;

class ServerDatabase : public QObject
{
    Q_OBJECT
public:
    explicit ServerDatabase(QObject *parent = 0);
    bool init();
    void close(){database.close();}

    bool getUserDetails(QString email, int machineId, QString* password, int* userId);

    bool lockFile(QString directory, QString filename, int userId);
    void updateAndUnlockFile(QString directory, QString filename, int userId, int machineId, bool deleted = false);

    ChangesList getChangesList(int machineId, int sinceRevisionNumber);
private:
    QMutex fileLocker;
    QSqlDatabase database;

    void registerMachine(int userId, int machineId);
    bool isFileLocked(QString directory, QString filename);
    void createTables();

signals:

public slots:
};

#endif // SERVERDATABASE_H
