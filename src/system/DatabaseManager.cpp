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
    databaseUsed = false;
    fileManager = new FileManager();
    bufPageManager = new BufPageManager(fileManager);
    metaData = new DBMeta();
    tableManager = nullptr;
}

inline bool DatabaseManager::checkDatabaseName(string name) {
    size_t length = name.length();
    if(length == 0 || length > DB_MAX_NAME_LEN) {
        printf("[Error] invalid database name !\n");
        return false;
    }
    return true;
}

inline bool DatabaseManager::checkFileExist(string path) {
    if (!access(path.c_str(), F_OK))
        return true; // already exit
    return false;
}

inline int DatabaseManager::searchTableByName(string name) {
    if(!databaseUsed) {
        printf("use a database first\n");
        return false;
    }
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(metaData->tableNames[i], name.c_str()) == 0)
            return i;
    }
    return -1;
}

int DatabaseManager::readMetaData(int fileId, DBMeta* meta) {
    int index;
    BufType loadData = bufPageManager->getPage(databaseStroeFileId, 0, index);
    memcpy((uint8_t*)meta, (uint8_t*)loadData, sizeof(int));
    return 0;
}

int DatabaseManager::writeMetaData(int fileId, DBMeta* meta) {
    int index;
    BufType storeData = bufPageManager->getPage(databaseStroeFileId, 0, index);
    memcpy((uint8_t*)storeData, (uint8_t*)meta, sizeof(DBMeta));
    bufPageManager->markDirty(index);
    bufPageManager->writeBack(index);
    return 0;
}

int DatabaseManager::createDatabase(string name) {
    if(!checkDatabaseName(name))
        return -1;
    string path = BASE_PATH + name + "/";
    if(checkFileExist(path)) {
        printf("[Error] database already exist !\n");
        return -1;
    }
    if(!mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH)) {
        //                     7         7         4
        // init database meta data
        path = path + ".DBstore";
        if(!checkFileExist(path)) { 
            bufPageManager->fileManager->createFile(path.c_str());
            bufPageManager->fileManager->openFile(path.c_str(), databaseStroeFileId);
            DBMeta* initMeta = new DBMeta;
            initMeta->tableNum = 0;
            writeMetaData(databaseStroeFileId, initMeta);
            databaseStroeFileId = -1;
        }
        return 0;
    }

    printf("[Error] fail to create the database %s\n", name.c_str());
    return -1;
}

int DatabaseManager::dropDatabase(string name) {
    if(!checkDatabaseName(name))
        return -1;
    
    string path = BASE_PATH + name + '/';
    if(!checkFileExist(path)) {
        printf("[Error] database does not exist !\n");
        return -1;
    }
    string command = "rm -rf ";
    system((command + path).c_str());
    return 0;
}

int DatabaseManager::switchDatabase(string name) {
    if (databaseUsed) {
        writeMetaData(databaseStroeFileId, metaData);
        bufPageManager->close();
        for(int i = 0; i < metaData->tableNum; i++) {
            tableManager->saveChangeToFile(metaData->tableNames[i]);
        }
        databaseUsedName = "";
        databaseStroeFileId = -1;
        databaseUsed = false;
    }

    if(!checkDatabaseName(name))
        return -1;

    string path = BASE_PATH + name + '/';

    if(!checkFileExist(path)) {
        printf("[Error] database does not exist !\n");
        return -1;
    }

    databaseUsedName = name;
    databaseUsed = true;
    // 内存泄漏的问题需要验证下
    if(tableManager != nullptr) {
        delete tableManager;
        tableManager = nullptr;
    }
    TableManager* newtmp = new TableManager(databaseUsedName, bufPageManager);
    tableManager = newtmp;

    path = path + ".DBstore";
    if(!checkFileExist(path)) {
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

int DatabaseManager::listTablesOfDatabase(string name) {
    printf("============%s=============\n", name.c_str());
    for(int i = 0; i < metaData->tableNum; i++) {
        printf("table%d: %-64s\n", i, metaData->tableNames[i]);
    }
    printf("=============end================\n");
}

int DatabaseManager::createTable(string name, char colName[][COL_MAX_NAME_LEN], TB_COL_TYPE* colType, int* colLen, int colNum) {
    if(!databaseUsed) {
        printf("[Error] use a database first!\n");
        return -1;
    }

    TableEntry* tableEntrys = new TableEntry[colNum];
    for(int i = 0; i < colNum; i++) {
        tableEntrys[i] = TableEntry(colName[i], colType[i]);
        if(colType[i] == COL_VARCHAR)
            tableEntrys[i].colLen = colLen[i];
    }

    if(tableManager->creatTable(name, tableEntrys, colNum) != 0) {
        printf("report error when create table in database manager");
        return -1;
    }
    
    strcpy(metaData->tableNames[metaData->tableNum], name.c_str());
    metaData->tableNum++;
    delete[] tableEntrys;
    return 0;

}

int DatabaseManager::dropTable(string name) {
    if(!databaseUsed) {
        printf("[Error] use a database first!\n");
        return -1;
    }

    int tableToDrop = searchTableByName(name);
    if(tableToDrop != -1) {
        printf("[Error] database is empty !\n");
        return -1;
    }
    if(tableManager->dropTable(name) == 0) {
        strcpy(metaData->tableNames[tableToDrop], metaData->tableNames[metaData->tableNum-1]);
        metaData->tableNum--;
        return 0;
    }
}

int DatabaseManager::renameTable(string oldName, string newName) {
    if(tableManager->renameTable(oldName, newName) != 0) {
        printf("[Error] report error when rename a table\n");
        return -1;
    }
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(metaData->tableNames[i], oldName.c_str()) == 0){
            strcpy(metaData->tableNames[i], newName.c_str());
            return 0;
        }
    }
    printf("[Error] report error when edit meta data!\n");
    return -1;
}