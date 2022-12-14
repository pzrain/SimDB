#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "DatabaseManager.h"

DBMeta::DBMeta() {
    tableNum = 0;
    foreignKeyNum = 0;
    memset(isPrimaryKey, false, sizeof(isPrimaryKey));
    memset(isUniqueKey, false, sizeof(isUniqueKey));
}

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

DatabaseManager::~DatabaseManager() {
    delete metaData;
    delete fileManager;
    delete bufPageManager;
    delete tableManager;
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
    memcpy((uint8_t*)meta, (uint8_t*)loadData, sizeof(DBMeta));
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
            initMeta->foreignKeyNum = 0;
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
    metaData->colNum[metaData->tableNum] = colNum;
    metaData->foreignKeyNum = 0;
    for(int i = 0; i < colNum; i++) {
        metaData->isPrimaryKey[metaData->tableNum][i] = false;
    }
    metaData->tableNum++;
    delete[] tableEntrys;
    return 0;
}

int DatabaseManager::listTableInfo(string name) {
    if(!databaseUsed) {
        printf("[Error] use a database first!\n");
        return -1;
    }

    return tableManager->listTableInfo(name);
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
        metaData->colNum[tableToDrop] = metaData->colNum[metaData->tableNum-1];
        // TODO delete foreign key
        for(int i = 0; i < metaData->colNum[metaData->tableNum-1]; i++) {
            metaData->isPrimaryKey[tableToDrop][i] = metaData->isPrimaryKey[metaData->tableNum-1][i];
        }
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

int DatabaseManager::createIndex(string tableName, string colName) {
    return tableManager->createIndex(tableName, colName);
}

int DatabaseManager::dropIndex(string tableName, string colName) {
    return tableManager->dropIndex(tableName, colName);
}

int DatabaseManager::hasIndex(string tableName, string colName) {
    return tableManager->hasIndex(tableName, colName);
}

int DatabaseManager::createPrimaryKey(string tableName, vector<string> colNames, int colNum) {
    int tableNum = -1;
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    if(tableNum == -1){
        fprintf(stderr, "meta data error when add primary key\n");
        return -1;
    }
    for(int i = 0; i < colNum; i++) {
        int colIndex = tableManager->createPrimaryKey(tableName, colNames[i]);
        if(colIndex == -1) {
            printf("[Error] error in adding %d primary key error\n", i);
            return -1;
        }
        metaData->isPrimaryKey[tableNum][colIndex] = true;
        metaData->isUniqueKey[tableNum][colIndex] = false; // make sure the two never co-exist
    }
    return 0;
}

int DatabaseManager::dropPrimaryKey(string tableName, vector<string> colNames, int colNum) {
    int tableNum = -1;
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    if(tableNum == -1){
        fprintf(stderr, "meta data error when drop primary key\n");
        return -1;
    }
    for(int i = 0; i < colNum; i++) {
        int colIndex = tableManager->createPrimaryKey(tableName, colNames[i]);
        if(colIndex == -1) {
            printf("[Error] error in dropping %d primary key error.\n", i);
            return -1;
        }
        metaData->isPrimaryKey[tableNum][colIndex] = false;
    }
    return 0;
}

int DatabaseManager::createForeignKey(string tableName, string foreignKeyName, string colName, string refTableName, string refTableCol) {
    int tableNum = -1, refTableNum = -1;
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0)
            tableNum = i;
        if(strcmp(refTableName.c_str(), metaData->tableNames[i]) == 0)
            refTableNum = i;
    }
    if(tableNum == -1 || refTableNum == -1){
        fprintf(stderr, "meta data error when add foreign key\n");
        return -1;
    }
    /*
        Note: can different tables have the same foreign key name ?
    */
    for(int i = 0; i < metaData->foreignKeyNum; i++) {
        if(strcmp(foreignKeyName.c_str(), metaData->foreignKeyNames[i]) == 0) {
            printf("[Error] foreign key already exists\n");
            return -1;
        }
    }

    int colIndex = tableManager->createForeignKey(tableName, foreignKeyName, colName, refTableName, refTableCol);
    if(colIndex == -1) {
        printf("[Error] error in creating foreign key.\n");
        return -1;
    }

    strcpy(metaData->foreignKeyNames[metaData->foreignKeyNum], foreignKeyName.c_str());
    metaData->foreignKeyColumn[metaData->foreignKeyNum] = colIndex; // colIndex will stay unchanged, so this is valid
    metaData->foreignKeyNum++;
    
    return 0;
}

int DatabaseManager::dropForeignKey(string tableName, string foreignKeyName) {
    int tableNum = -1;
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    if(tableNum == -1){
        // printf("meta data error when drop foreign key\n");
        printf("[Error] specified table does not exist.\n");
        return -1;
    }
    int foreignKeyIndex = -1;
    for(int i = 0; i < metaData->foreignKeyNum; i++) {
        if(strcmp(metaData->foreignKeyNames[i], foreignKeyName.c_str()) == 0) {
            foreignKeyIndex = i;
            break;
        }
    }
    if(foreignKeyIndex == -1) {
        // printf("meta data error when drop foreign key\n");
        printf("[Error] specified column does not exist.\n");
    }

    int ret = tableManager->dropForeignKey(tableName, metaData->foreignKeyColumn[foreignKeyIndex]);
    if(ret == -1)
        return -1;
    strcpy(metaData->foreignKeyNames[foreignKeyIndex], metaData->foreignKeyNames[metaData->foreignKeyNum-1]);
    metaData->foreignKeyColumn[foreignKeyIndex] = metaData->foreignKeyColumn[metaData->foreignKeyNum-1];
    metaData->foreignKeyNum--;
    return 0;
}

int DatabaseManager::createUniqueKey(string tableName, vector<string> colNames, int colNum) {
    int tableNum = -1;
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    if(tableNum == -1){
        fprintf(stderr, "meta data error when add unique key\n");
        return -1;
    }
    for(int i = 0; i < colNum; i++) {
        int colIndex = tableManager->createUniqueKey(tableName, colNames[i]);
        if(colIndex == -1) {
            printf("[Error] error in adding %d unique key error\n", i);
            return -1;
        }
        metaData->isUniqueKey[tableNum][colIndex] = true;
    }
    return 0;
}

int DatabaseManager::dropUniqueKey(string tableName, vector<string> colNames, int colNum) {
    int tableNum = -1;
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    if(tableNum == -1){
        fprintf(stderr, "meta data error when drop unique key\n");
        return -1;
    }
    for(int i = 0; i < colNum; i++) {
        int colIndex = tableManager->createUniqueKey(tableName, colNames[i]);
        if(colIndex == -1) {
            printf("[Error] error in dropping %d unique key error.\n", i);
            return -1;
        }
        metaData->isUniqueKey[tableNum][colIndex] = false;
    }
    return 0;
}
