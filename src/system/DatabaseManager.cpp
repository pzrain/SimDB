#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "DatabaseManager.h"

DatabaseManager::DatabaseManager() {
    BASE_PATH = "database/";
    databaseStroeFileId = -1;
    databaseUsedName = "";
    fileManager = new FileManager();
    bufPageManager = new BufPageManager(fileManager);
    tableManager = nullptr;
}

inline bool DatabaseManager::checkName(string name) {
    size_t length = name.length();
    if(length == 0 || length > DB_MAX_NAME_LEN) {
        printf("[Error] valid database name !\n");
        return false;
    }
    return true;
}

inline bool DatabaseManager::checkExist(string path) {
    if (!access(path.c_str(), F_OK))
        return true; // already exit
    return false;
}

int DatabaseManager::readMetaData(int fileId, DBMeta* meta) {
    int index;
    BufType loadData = bufPageManager->allocPage(databaseStroeFileId, 0, index, true);
    memcpy((uint8_t*)meta, (uint8_t*)loadData, sizeof(DBMeta));
    return 0;
}

int DatabaseManager::writeMetaData(int fileId, DBMeta* meta) {
    int index;
    BufType storeData = bufPageManager->getPage(databaseStroeFileId, 0, index);
    memcpy((uint8_t*)storeData, (uint8_t*)meta, sizeof(DBMeta));
    bufPageManager->markDirty(index);
    return 0;
}

int DatabaseManager::createDatabase(string name) {
    if(!checkName(name))
        return -1;

    string path = BASE_PATH + name + '/';
    if(checkExist(path)) {
        printf("[Error] database already exist !\n");
        return -1;
    }
    if(!mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH)) {
        //                     7         7         4
        // database meta info
        path = path + ".DBstore";
        if(!checkExist(path)) {
            bufPageManager->fileManager->createFile(path.c_str());
        }
        return 0;
    }

    printf("[Error] fail to create the database %s\n", name);
    return -1;
}

int DatabaseManager::dropDatabase(string name) {
    if(!checkName(name))
        return -1;
    
    string path = BASE_PATH + name + '/';
    if(!checkExist(path)) {
        printf("[Error] database does not exist !\n");
        return -1;
    }

    system(("rm -rf " + path).c_str());
    return 0;
}

int DatabaseManager::switchDatabase(string name) {
    if (databaseStroeFileId != -1) {
        writeMetaData(databaseStroeFileId, metaData);
        bufPageManager->close();
        databaseUsedName = "";
        databaseStroeFileId = -1;
    }

    if(!checkName(name))
        return -1;

    string path = BASE_PATH + name + '/';

    if(!checkExist(path)) {
        printf("[Error] database does not exist !\n");
        return -1;
    }

    databaseUsedName = name;
    tableManager = new TableManager(databaseUsedName, bufPageManager);

    path = path + ".DBstore";
    if(!checkExist(path)) {
        printf("[Error] database data lose\n");
        return -1;
    }
    bufPageManager->fileManager->openFile(path.c_str(), databaseStroeFileId);
    readMetaData(databaseStroeFileId, metaData);

    for(int i = 0; i < metaData->tableNum; i++) {
        tableManager->openTable(metaData->tableNames[i]);
    }
    
    return 0;
}