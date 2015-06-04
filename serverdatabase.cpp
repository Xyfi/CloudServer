#include "serverdatabase.h"

ServerDatabase::ServerDatabase(QObject *parent) : QObject(parent)
{

}

// PUBLIC FUNCTIONS

bool ServerDatabase::init(){
    if(!QFile(DB_NAME).exists()){
        // Create and open database
        database = QSqlDatabase::addDatabase(DB_TYPE);
        database.setDatabaseName(DB_NAME);
        if(!database.open()){
            qDebug() << "Opening new database failed!";
            return false;
        }
        database.exec("pragma foreign_keys = on");
        createTables();
        return true;
    } else {
        database = QSqlDatabase::addDatabase(DB_TYPE);
        database.setDatabaseName(DB_NAME);
        if(!database.open()){
            qDebug() << "Opening existing database failed!";
            return false;
        }
        database.exec("pragma foreign_keys = on");
    }
    return true;
}

bool ServerDatabase::getUserDetails(QString email, int machineId, QString *password, int* userId){
    QString queryString = "SELECT password, id FROM Users WHERE email = :email";
    QSqlQuery query(database);
    if(!query.prepare(queryString)){
        qDebug() << "[getUserDetails] Prepare failed";
        return false;
    }
    query.bindValue(":email", email);
    if(!query.exec()){
        qDebug() << "[getUserDetails] Exec failed";
        return false;
    }
    if(!query.next()){
        qDebug() << "[getUserDetails] Next failed" << email;
        return false;
    }
    *password = query.value(0).toString();
    *userId = query.value(1).toInt();

    registerMachine(*userId, machineId);

    return true;
}

bool ServerDatabase::lockFile(QString directory, QString filename, int userId){
    QMutexLocker lock(&fileLocker);
    if(isFileLocked(directory, filename)){
        qDebug() << "[ServerDatabase::lockFile] Allready locked";
        return false;
    }
    QString queryString = "INSERT OR REPLACE INTO Files(directory, filename, locked, ownedBy, deleted) VALUES(:directory, :filename, :locked, :ownedBy, 0)";
    QSqlQuery query(database);
    if(!query.prepare(queryString)){
        qDebug() << "[ServerDatabase::lockFile] Prepare failed" << query.lastError().text();
        return false;
    }
    query.bindValue(":directory", directory);
    query.bindValue(":filename", filename);
    query.bindValue(":locked", 1);
    query.bindValue(":ownedBy", userId);
    if(!query.exec()){
        qDebug() << "[ServerDatabase::lockFile] Exec failed" << query.lastError().text();
        return false;
    }
    return true;
}

void ServerDatabase::updateAndUnlockFile(QString directory, QString filename, int userId, int machineId, bool deleted){
    QMutexLocker lock(&fileLocker);
    QString queryString = "UPDATE Files SET locked = 0, lastUpdatedBy = :machineId, deleted = :deleted, revisionNumber = (Select MAX(revisionNumber) + 1 FROM Files WHERE ownedBy = :userId) WHERE ownedBy = :userId AND directory = :directory AND filename = :filename";
    QSqlQuery query(database);
    if(!query.prepare(queryString)){
        qDebug() << "[updateAndUnlockFile] Prepare failed.";
        qDebug() << query.lastError().text();
        return;
    }
    query.bindValue(":machineId", machineId);
    query.bindValue(":userId", userId);
    query.bindValue(":deleted", deleted);
    query.bindValue(":directory", directory);
    query.bindValue(":filename", filename);
    if(!query.exec()){
        qDebug() << "[updateAndUnlockFile] Exec failed.";
        qDebug() << query.lastError().text();
        return;
    }
}

ChangesList ServerDatabase::getChangesList(int machineId){
    QString queryString = "SELECT directory, filename, deleted, revisionNumber FROM Files WHERE revisionNumber > (SELECT lastRevisionNumber FROM UserMachines WHERE machineId = :machineId) AND lastUpdatedBy != :machineId ORDER BY revisionNumber ASC";
    QSqlQuery query(database);
    ChangesList list;
    if(!query.prepare(queryString)){
        qDebug() << "[getChangesList] Prepare failed.";
        qDebug() << query.lastError().text();
        return list;
    }
    query.bindValue(":machineId", machineId);
    if(!query.exec()){
        qDebug() << "[getChangesList] Exec failed.";
        qDebug() << query.lastError().text();
        return list;
    }
    QByteArray fileChanges;
    QDataStream in(&fileChanges, QIODevice::WriteOnly);
    int counter = 0;
    while(query.next()){
        counter++;
        in << query.value(0).toString();
        in << query.value(1).toString();
        in << query.value(2).toBool();
        list.revisionNumber = query.value(3).toLongLong();
        qDebug() << "Files: " << counter;
    }
    list.nChanges = counter;
    list.changes = fileChanges;
    updateRevisionNumber(machineId);
    return list;
}

// PRIVATE FUNCTIONS

void ServerDatabase::updateRevisionNumber(int machineId){
    QString queryString = "UPDATE UserMachines SET lastRevisionNumber = (SELECT MAX(revisionNumber) FROM Files WHERE userId = (SELECT userId FROM UserMachines WHERE id = :machineId)) WHERE id = :machineId";
    QSqlQuery query(database);
    if(!query.prepare(queryString)){
        qDebug() << "[ServerDatabase::updateRevisionNumber] Prepare failed.";
        qDebug() << query.lastError().text();
    }
    query.bindValue(":machineId", machineId);
    if(!query.exec()){
        qDebug() << "[ServerDatabase::updateRevisionNumber] Exec failed.";
        qDebug() << query.lastError().text();
    }
}

void ServerDatabase::registerMachine(int userId, int machineId){
    qDebug() << "Registering machine";
    QString queryString = "INSERT OR IGNORE INTO UserMachines(id, userId) VALUES(:machineId, :userId)";
    QSqlQuery query(database);
    if(!query.prepare(queryString)){
        return;
    }
    query.bindValue(":machineId", machineId);
    query.bindValue(":userId", userId);
    if(!query.exec()){
        return;
    }
}

bool ServerDatabase::isFileLocked(QString directory, QString filename){
    QString queryString = "SELECT locked FROM Files WHERE directory = :directory AND filename = :filename";
    QSqlQuery query(database);
    if(!query.prepare(queryString)){
        qDebug() << "Error preparing query";
        return true;
    }
    query.bindValue(":directory", directory);
    query.bindValue(":filename", filename);
    if(!query.exec()){
        qDebug() << "Error executing query";
        return true;
    }
    if(!query.next()){
        return false;
    }
    if(query.value(0).toInt() != 0){
        qDebug() << "[ServerDatabase::isFileLocked] File is locked.";
        return true;
    }
    return false;
}

void ServerDatabase::createTables(){
    if(!database.tables().contains("Users")){
        QSqlQuery query(database);
        bool s = query.exec(TABLE_USERS);
        if(!s){
            qDebug() << "Query failed" << query.lastError().text();
        } else {
            qDebug() << "Table Users added";
        }
    }
    if(!database.tables().contains("UserMachines")){
        QSqlQuery query(database);
        bool s = query.exec(TABLE_USER_MACHINES);
        if(!s){
            qDebug() << "Query failed" << query.lastError().text();
        } else {
            qDebug() << "Table UserMachines added";
        }
    }
    if(!database.tables().contains("Files")){
        QSqlQuery query(database);
        bool s = query.exec(TABLE_FILES);
        if(!s){
            qDebug() << "Query failed" << query.lastError().text();
        } else {
            qDebug() << "Table Files added";
        }
    }
}

