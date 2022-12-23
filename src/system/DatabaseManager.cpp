#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "DatabaseManager.h"

DatabaseManager::DatabaseManager() {
    BASE_PATH = "database/";
    databaseStroeFileId = -1;
    databaseUsedName = "";
    databaseUsed = false;
    MyBitMap::initConst();
    fileManager = new FileManager();
    bufPageManager = new BufPageManager(fileManager);
    metaData = new DBMeta();
    tableManager = nullptr;
}

DatabaseManager::~DatabaseManager() {
    if (databaseUsed) {
        writeMetaData(databaseStroeFileId, metaData);
        for(int i = 0; i < metaData->tableNum; i++) {
            tableManager->saveChangeToFile(metaData->tableNames[i]);
        }
        databaseUsedName = "";
        databaseStroeFileId = -1;
        databaseUsed = false;
    }
    delete metaData;
    if (tableManager != nullptr) {
        delete tableManager;
    }
    delete bufPageManager;
    delete fileManager;
}

inline bool DatabaseManager::checkDatabaseName(string name) {
    size_t length = name.length();
    if(length == 0 || length > DB_MAX_NAME_LEN) {
        printf("[ERROR] invalid database name !\n");
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
        printf("[ERROR] use a database first\n");
        return -1;
    }
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(metaData->tableNames[i], name.c_str()) == 0)
            return i;
    }
    return -1;
}

int DatabaseManager::readMetaData(int fileId, DBMeta* meta) {
    int index;
    BufType loadData = bufPageManager->getPage(fileId, 0, index);
    memcpy((uint8_t*)meta, (uint8_t*)loadData, sizeof(DBMeta));
    return 0;
}

int DatabaseManager::writeMetaData(int fileId, DBMeta* meta) {
    int index;
    BufType storeData;
    storeData = bufPageManager->getPage(fileId, 0, index);
    memcpy((uint8_t*)storeData, (uint8_t*)meta, sizeof(DBMeta));
    bufPageManager->markDirty(index);
    // bufPageManager->writeBack(index);
    return 0;
}

int DatabaseManager::createDatabase(string name) {
    if(!checkDatabaseName(name))
        return -1;
    string path = BASE_PATH + name + "/";
    if(checkFileExist(path)) {
        printf("[ERROR] database already exist !\n");
        return -1;
    }
    if(!mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH)) {
        //                     7         7         4
        // init database meta data
        path = path + ".DBstore";
        if(!checkFileExist(path)) { 
            int fileId;
            bufPageManager->fileManager->createFile(path.c_str());
            bufPageManager->fileManager->openFile(path.c_str(), fileId);
            DBMeta* initMeta = new DBMeta();
            writeMetaData(fileId, initMeta);
        }
        return 0;
    }

    printf("[ERROR] fail to create the database %s\n", name.c_str());
    return -1;
}

int DatabaseManager::dropDatabase(string name) {
    if(!checkDatabaseName(name))
        return -1;
    
    if (databaseUsed && name == databaseUsedName) {
        printf("[ERROR] can not drop currently using database.\n");
        return -1;
    }

    string path = BASE_PATH + name + '/';
    if(!checkFileExist(path)) {
        printf("[ERROR] database does not exist !\n");
        return -1;
    }
    string command = "rm -rf ";
    system((command + path).c_str());
    return 0;
}

int DatabaseManager::switchDatabase(string name) {
    if (name == databaseUsedName) {
        printf("[INFO] databse %s is already in use.\n", name.c_str());
        return 0;
    }
    if (databaseUsed) {
        writeMetaData(databaseStroeFileId, metaData);
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
        printf("[ERROR] database does not exist !\n");
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
        printf("[ERROR] database data lose\n");
        return -1;
    }
    bufPageManager->fileManager->openFile(path.c_str(), databaseStroeFileId);
    readMetaData(databaseStroeFileId, metaData);
    for(int i = 0; i < metaData->tableNum; i++) {
        tableManager->openTable(metaData->tableNames[i]);
    }
    vector<string> indexTableNames;
    vector<vector<string>> indexColNames;
    for (int i = 0; i < metaData->tableNum; i++) {
        indexTableNames.push_back(metaData->tableNames[i]);
        indexColNames.push_back(vector<string>());
        for (int j = 0; j < metaData->indexNum[i]; j++) {
            indexColNames[i].push_back(metaData->indexNames[i][j]);
        }
    }
    tableManager->initIndex(indexTableNames, indexColNames);
    printf("Database changed.\n");
    return 0;
}

int DatabaseManager::showDatabases() {
    printf("===== Show Databases =====\n");
    int cnt = 0;
    for (const auto & entry : std::filesystem::directory_iterator(BASE_PATH)) {
        std::string str(entry.path().c_str());
        printf("%s\n", str.substr(BASE_PATH.length(), str.length() - BASE_PATH.length()).c_str());
        cnt += 1;
    }
    printf("===== %d databases in total =====\n", cnt);
    return 0;
}

string DatabaseManager::getDatabaseName() {
    return databaseUsedName == "" ? "simDB" : databaseUsedName;
}

int DatabaseManager::listTablesOfDatabase() {
    if (!databaseUsed) {
        printf("[ERROR] use a database first!\n");
        return -1;
    }
    printf("============%s=============\n", databaseUsedName.c_str());
    for(int i = 0; i < metaData->tableNum; i++) {
        printf("table%d: %-64s\n", i, metaData->tableNames[i]);
    }
    printf("=============end================\n");
    return 0;
}

int DatabaseManager::createTable(string name, char colName[][COL_MAX_NAME_LEN], TB_COL_TYPE* colType, int* colLen, int colNum) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first!\n");
        return -1;
    }

    TableEntry* tableEntrys = new TableEntry[colNum];
    for(int i = 0; i < colNum; i++) {
        tableEntrys[i] = TableEntry(colName[i], colType[i]);
        tableEntrys[i].colLen = colLen[i];
    }
    if(tableManager->creatTable(name, tableEntrys, colNum) != 0) {
        printf("report error when create table in database manager\n");
        return -1;
    }
    
    strcpy(metaData->tableNames[metaData->tableNum], name.c_str());
    metaData->colNum[metaData->tableNum] = colNum;
    metaData->foreignKeyNum[metaData->tableNum] = 0;
    for(int i = 0; i < TAB_MAX_COL_NUM; i++) {
        metaData->isPrimaryKey[metaData->tableNum][i] = false;
        metaData->isUniqueKey[metaData->tableNum][i] = false;
    }
    metaData->tableNum++;
    delete[] tableEntrys;
    return 0;
}

int DatabaseManager::listTableInfo(string name) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first!\n");
        return -1;
    }

    return tableManager->listTableInfo(name, metaData);
}

int DatabaseManager::dropTable(string name) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first!\n");
        return -1;
    }

    int tableToDrop = searchTableByName(name);
    if(tableToDrop == -1) {
        printf("[ERROR] no such table %s!\n", name.c_str());
        return -1;
    }
    if(tableManager->dropTable(name) == 0) {
        strcpy(metaData->tableNames[tableToDrop], metaData->tableNames[metaData->tableNum-1]);
        metaData->colNum[tableToDrop] = metaData->colNum[metaData->tableNum-1];
        for(int i = 0; i < TAB_MAX_COL_NUM; i++) { // note: this should be TAB_MAX_COL_NUM
            metaData->isPrimaryKey[tableToDrop][i] = metaData->isPrimaryKey[metaData->tableNum-1][i];
            metaData->isUniqueKey[tableToDrop][i] = metaData->isUniqueKey[metaData->tableNum-1][i];
        }
        for (int i = 0; i < metaData->foreignKeyNum[metaData->tableNum-1]; i++) {
            int refTableNum = metaData->foreignKeyRefTable[tableToDrop][i];
            int refColNum = metaData->foreignKeyRefColumn[tableToDrop][i];
            int refKeyIndex = metaData->foreignToRef[tableToDrop][i];
            metaData->refKeyColumn[refTableNum][refKeyIndex] = metaData->refKeyColumn[refTableNum][metaData->refKeyNum[refTableNum]-1];
            metaData->refKeyRefTable[refTableNum][refKeyIndex] = metaData->refKeyRefTable[refTableNum][metaData->refKeyNum[refTableNum]-1];
            metaData->refKeyRefColumn[refTableNum][refKeyIndex] = metaData->refKeyRefColumn[refTableNum][metaData->refKeyNum[refTableNum]-1];
            metaData->refKeyNum[refTableNum]--;

            metaData->foreignKeyOnCol[tableToDrop][i] = metaData->foreignKeyOnCol[metaData->tableNum-1][i];
            strcpy(metaData->foreignKeyNames[tableToDrop][i], metaData->foreignKeyNames[metaData->tableNum-1][i]);
            metaData->foreignKeyColumn[tableToDrop][i] = metaData->foreignKeyColumn[metaData->tableNum-1][i];
            metaData->foreignKeyRefTable[tableToDrop][i] = metaData->foreignKeyRefTable[metaData->tableNum-1][i];
            metaData->foreignKeyRefColumn[tableToDrop][i] = metaData->foreignKeyRefColumn[metaData->tableNum-1][i];
        }
        metaData->tableNum--;
        return 0;
    }
    return -1;
}

int DatabaseManager::renameTable(string oldName, string newName) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    if(tableManager->renameTable(oldName, newName) != 0) {
        printf("[ERROR] report error when rename a table\n");
        return -1;
    }
    for (int i = 0; i < metaData->tableNum; i++) {
        if (strcmp(metaData->tableNames[i], newName.c_str()) == 0) {
            printf("[ERROR] a table named %s already exists.\n", newName.c_str());
            return -1;
        }
    }
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(metaData->tableNames[i], oldName.c_str()) == 0){
            strcpy(metaData->tableNames[i], newName.c_str());
            return 0;
        }
    }
    printf("[ERROR] report error when edit meta data!\n");
    return -1;
}

int DatabaseManager::createIndex(string tableName, string colName) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    int res = tableManager->createIndex(tableName, colName);
    if (res > -1) {
        int tableNum = -1;
        for(int i = 0; i < metaData->tableNum; i++) {
            if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0) {
                tableNum = i;
                break;
            }
        }
        strcpy(metaData->indexNames[tableNum][metaData->indexNum[tableNum]], colName.c_str());
        metaData->mannuallyCreateIndex[tableNum][metaData->indexNum[tableNum]] = true;
        metaData->indexNum[tableNum]++;
    }
    return res;
}

int DatabaseManager::dropIndex(string tableName, string colName) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    int res = tableManager->dropIndex(tableName, colName);
    if (res > -1) {
        int tableNum = -1;
        for(int i = 0; i < metaData->tableNum; i++) {
            if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0) {
                tableNum = i;
                break;
            }
        }
        int indexNum = -1;
        for (int i = 0; i < metaData->indexNum[tableNum]; i++) {
            if (strcmp(metaData->indexNames[tableNum][i], colName.c_str()) == 0) {
                indexNum = i;
                break;
            }
        }
        strcpy(metaData->indexNames[tableNum][indexNum], metaData->indexNames[tableNum][metaData->indexNum[tableNum]-1]);
        metaData->mannuallyCreateIndex[tableNum][indexNum] = metaData->mannuallyCreateIndex[tableNum][metaData->indexNum[tableNum]-1];
        metaData->indexNum[tableNum]--;
    }
    return res;
}

int DatabaseManager::hasIndex(string tableName, string colName) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    return tableManager->hasIndex(tableName, colName);
}

int DatabaseManager::showIndex() {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    return tableManager->showIndex();
}

void addOrDropIndex(string tableName, string colName, DBMeta* metaData, bool add) {
    int tableNum = -1;
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    if (tableNum == -1) {
        return;
    }
    int indexNum = -1;
    for (int i = 0; i < metaData->indexNum[tableNum]; i++) {
        if (strcmp(colName.c_str(), metaData->indexNames[tableNum][i]) == 0) {
            indexNum = i;
            break;
        }
    }
    if (add && indexNum == -1) {
        strcpy(metaData->indexNames[tableNum][metaData->indexNum[tableNum]], colName.c_str());
        metaData->mannuallyCreateIndex[tableNum][metaData->indexNum[tableNum]] = false;
        metaData->indexNum[tableNum]++;
    } else if (!add && indexNum != -1) {
        strcpy(metaData->indexNames[tableNum][indexNum], metaData->indexNames[tableNum][metaData->indexNum[tableNum]-1]);
        metaData->mannuallyCreateIndex[tableNum][indexNum] = metaData->mannuallyCreateIndex[tableNum][metaData->indexNum[tableNum]-1];
        metaData->indexNum[tableNum]--;
    }
}

int DatabaseManager::createPrimaryKey(string tableName, vector<string> colNames, int colNum) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
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
            printf("[ERROR] error in adding %d:%s primary key\n", i, colNames[i].c_str());
            return -1;
        }
        addOrDropIndex(tableName, colNames[i], metaData, true);
        metaData->isPrimaryKey[tableNum][colIndex] = true;
        // metaData->isUniqueKey[tableNum][colIndex] = false; // make sure the two never co-exist
    }
    return 0;
}

int DatabaseManager::dropPrimaryKey(string tableName, vector<string> colNames, int colNum) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
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
        int indexDropped = -1;
        int colIndex = tableManager->dropPrimaryKey(tableName, colNames[i], metaData, indexDropped);
        if(colIndex == -1) {
            printf("[ERROR] error in dropping %d primary key error.\n", i);
            return -1;
        }
        if (indexDropped == 1) {
            addOrDropIndex(tableName, colNames[i], metaData, false);
        }
        metaData->isPrimaryKey[tableNum][colIndex] = false;
    }
    return 0;
}

int DatabaseManager::createForeignKey(string tableName, string foreignKeyName, string colName, string refTableName, string refTableCol) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
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
    for(int i = 0; i < metaData->foreignKeyNum[tableNum]; i++) {
        if(strcmp(foreignKeyName.c_str(), metaData->foreignKeyNames[tableNum][i]) == 0) {
            printf("[ERROR] foreign key already exists\n");
            return -1;
        }
    }
    int refIndex;
    int colIndex = tableManager->createForeignKey(tableName, foreignKeyName, colName, refTableName, refTableCol, refIndex);
    if(colIndex == -1) {
        printf("[ERROR] error in creating foreign key.\n");
        return -1;
    }
    addOrDropIndex(tableName, colName, metaData, true);
    addOrDropIndex(refTableName, refTableCol, metaData, true);
    metaData->foreignKeyOnCol[tableNum][colIndex]++;
    metaData->foreignKeyOnCol[refTableNum][refIndex]++;

    strcpy(metaData->foreignKeyNames[tableNum][metaData->foreignKeyNum[tableNum]], foreignKeyName.c_str());
    metaData->foreignKeyColumn[tableNum][metaData->foreignKeyNum[tableNum]] = colIndex; // colIndex will stay unchanged, so this is valid
    metaData->foreignKeyRefTable[tableNum][metaData->foreignKeyNum[tableNum]] = refTableNum;
    metaData->foreignKeyRefColumn[tableNum][metaData->foreignKeyNum[tableNum]] = refIndex;
    metaData->foreignToRef[tableNum][metaData->foreignKeyNum[tableNum]] = metaData->refKeyNum[refTableNum];
    metaData->foreignKeyNum[tableNum]++;

    metaData->refKeyColumn[refTableNum][metaData->refKeyNum[refTableNum]] = refIndex;
    metaData->refKeyRefTable[refTableNum][metaData->refKeyNum[refTableNum]] = tableNum;
    metaData->refKeyRefColumn[refTableNum][metaData->refKeyNum[refTableNum]] = colIndex;
    metaData->refKeyNum[refTableNum]++;

    return 0;
}

int DatabaseManager::dropForeignKey(string tableName, string foreignKeyName) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    int tableNum = -1;
    for(int i = 0; i < metaData->tableNum; i++) {
        if(strcmp(tableName.c_str(), metaData->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    if(tableNum == -1){
        // printf("meta data error when drop foreign key\n");
        printf("[ERROR] specified table does not exist.\n");
        return -1;
    }
    int foreignKeyIndex = -1;
    for(int i = 0; i < metaData->foreignKeyNum[tableNum]; i++) {
        if(strcmp(metaData->foreignKeyNames[tableNum][i], foreignKeyName.c_str()) == 0) {
            foreignKeyIndex = i;
            break;
        }
    }
    if(foreignKeyIndex == -1) {
        // printf("meta data error when drop foreign key\n");
        printf("[ERROR] specified foreign key does not exist.\n");
    }
    char colName[64], refColName[64];
    int refTableNum = metaData->foreignKeyRefTable[tableNum][foreignKeyIndex];
    int refColNum = metaData->foreignKeyRefColumn[tableNum][foreignKeyIndex];
    int refKeyIndex = metaData->foreignToRef[tableNum][foreignKeyIndex];
    int indexDropped = -1, refIndexDropped = -1;
    int ret = tableManager->dropForeignKey(tableName, metaData->foreignKeyColumn[tableNum][foreignKeyIndex], metaData, metaData->tableNames[refTableNum], refColNum, indexDropped, refIndexDropped, colName, refColName);
    if(ret == -1)
        return -1;
    if (indexDropped == 1) {
        addOrDropIndex(tableName, colName, metaData, false);
    }
    if (refIndexDropped == 1) {
        addOrDropIndex(metaData->tableNames[refTableNum], refColName, metaData, false);
    }
    metaData->foreignKeyOnCol[tableNum][metaData->foreignKeyColumn[tableNum][foreignKeyIndex]]--;
    metaData->foreignKeyOnCol[metaData->foreignKeyRefTable[tableNum][foreignKeyIndex]][metaData->foreignKeyRefColumn[tableNum][foreignKeyIndex]]--;

    metaData->refKeyColumn[refTableNum][refKeyIndex] = metaData->refKeyColumn[refTableNum][metaData->refKeyNum[refTableNum]-1];
    metaData->refKeyRefTable[refTableNum][refKeyIndex] = metaData->refKeyRefTable[refTableNum][metaData->refKeyNum[refTableNum]-1];
    metaData->refKeyRefColumn[refTableNum][refKeyIndex] = metaData->refKeyRefColumn[refTableNum][metaData->refKeyNum[refTableNum]-1];
    metaData->refKeyNum[refTableNum]--;

    strcpy(metaData->foreignKeyNames[tableNum][foreignKeyIndex], metaData->foreignKeyNames[tableNum][metaData->foreignKeyNum[tableNum]-1]);
    metaData->foreignKeyColumn[tableNum][foreignKeyIndex] = metaData->foreignKeyColumn[tableNum][metaData->foreignKeyNum[tableNum]-1];
    metaData->foreignKeyRefTable[tableNum][foreignKeyIndex] = metaData->foreignKeyRefTable[tableNum][metaData->foreignKeyNum[tableNum]-1];
    metaData->foreignKeyRefColumn[tableNum][foreignKeyIndex] = metaData->foreignKeyRefColumn[tableNum][metaData->foreignKeyNum[tableNum]-1];
    metaData->foreignToRef[tableNum][foreignKeyIndex] = metaData->foreignToRef[tableNum][metaData->foreignKeyNum[tableNum]-1];
    metaData->foreignKeyNum[tableNum]--;

    return 0;
}

int DatabaseManager::createUniqueKey(string tableName, vector<string> colNames, int colNum) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
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
            printf("[ERROR] error in adding %d unique key error\n", i);
            return -1;
        }
        addOrDropIndex(tableName, colNames[i], metaData, true);
        metaData->isUniqueKey[tableNum][colIndex] = true;
    }
    return 0;
}

int DatabaseManager::dropUniqueKey(string tableName, vector<string> colNames, int colNum) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
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
        int indexDropped = -1;
        int colIndex = tableManager->dropUniqueKey(tableName, colNames[i], metaData, indexDropped);
        if(colIndex == -1) {
            printf("[ERROR] error in dropping %d unique key error.\n", i);
            return -1;
        }
        if (indexDropped == 1) {
            addOrDropIndex(tableName, colNames[i], metaData, false);
        }
        metaData->isUniqueKey[tableNum][colIndex] = false;
    }
    return 0;
}

int DatabaseManager::selectRecords(DBSelect* dbSelect) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    return tableManager->selectRecords(dbSelect);
}

int DatabaseManager::updateRecords(string tableName, DBUpdate* dbUpdate) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    return tableManager->updateRecords(tableName, dbUpdate, metaData);
}

int DatabaseManager::insertRecords(string tableName, DBInsert* dbInsert) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    return tableManager->insertRecords(tableName, dbInsert, metaData);
}

int DatabaseManager::dropRecords(string tableName, DBDelete* dbDelete) {
    if(!databaseUsed) {
        printf("[ERROR] use a database first\n");
        return -1;
    }
    return tableManager->dropRecords(tableName, dbDelete, metaData);
}
