#include "TableManager.h"
#include "assert.h"
#include <map>
#include <regex>
#include <algorithm>
#include <unistd.h>
#include <cstring>

TableManager::TableManager(string databaseName_,  BufPageManager* bufPageManager_) {
    databaseName = databaseName_;
    recordManager = new RecordManager(bufPageManager_, databaseName.c_str());
    indexManager = new IndexManager(bufPageManager_, databaseName.c_str());
}

TableManager::~TableManager() {
    delete recordManager;
    delete indexManager;
    // delete fileHandler;
}

int TableManager::checkColExist(TableHeader* tableHeader, const char* colName) {
    return tableHeader->getCol(colName);
}

inline bool TableManager::checkTableName(string name) {
    size_t length = name.length();
    if(length == 0 || length > TAB_MAX_NAME_LEN) {
        printf("[ERROR] invalid table name ! \n");
        return false;
    }
    return true;
}

inline bool TableManager::checkTableExist(string path) {
    if (!access(path.c_str(), F_OK))
        return true; // already exit
    return false;
}

int TableManager::creatTable(string tableName, TableEntry* tableEntrys, int colNum) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(checkTableExist(path)) {
        printf("[ERROR] table named %s already exist!\n", tableName.c_str());
        return -1;
    }
    if(recordManager->createFile(tableName.c_str()) != 0){
        fprintf(stderr, "report error when create file in table manager\n");
        return -1;
    }
    fileHandler = recordManager->openFile(tableName.c_str());
    if(fileHandler == nullptr) {
        fprintf(stderr, "report error when open file in tablemanager\n");
        return -1;
    }

    fileHandler->operateTable(TB_INIT, nullptr, tableEntrys, colNum);
    return 0;
}

int TableManager::openTable(string name) {
    if(!checkTableName(name))
        return -1;
    string path = "database/" + databaseName + '/' + name +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }

    if((fileHandler = recordManager->openFile(name.c_str())) == nullptr) {
        printf("[ERROR] error in opening table %s.\n", name.c_str());
        return -1;
    }

    return 0;
}

int TableManager::dropTable(string name) {
    if(!checkTableName(name))
        return -1;
    string path = "database/" + databaseName + '/' + name +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
        
    if(recordManager->removeFile(name.c_str()) != 0) {
        printf("[ERROR] fail to drop the table named %s\n", name.c_str());
        return -1;
    }
    indexManager->removeIndex(name.c_str());

    return 0;

}

int TableManager::listTableInfo(string name, DBMeta* dbMeta) {
    if(!checkTableName(name))
        return -1;
    string path = "database/" + databaseName + '/' + name +".db";
    if(!checkTableExist(path))
        return -1;
    fileHandler = recordManager->findTable(name.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int tableNum = -1;
    for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
        if (strcmp(name.c_str(), dbMeta->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    if (tableNum == -1) {
        return -1;
    }
    printf("==========  Begin Table Info  ==========\n");
    tableHeader->printInfo();

    FileHandler* refFileHandler;
    TableHeader* refTableHeader;
    char colName[64], refColName[64];
    for (int i = 0; i < dbMeta->foreignKeyNum[tableNum]; i++) {
        printf("FOREIGN KEY ");
        tableHeader->getCol(dbMeta->foreignKeyColumn[tableNum][i], colName);
        printf("%s (%s)", dbMeta->foreignKeyNames[tableNum][i], colName);
        printf(" REFERENCE ");
        refFileHandler = recordManager->findTable(dbMeta->tableNames[dbMeta->foreignKeyRefTable[tableNum][i]]);
        refTableHeader = refFileHandler->getTableHeader();
        refTableHeader->getCol(dbMeta->foreignKeyRefColumn[tableNum][i], refColName);
        printf("%s.%s;\n", dbMeta->tableNames[dbMeta->foreignKeyRefTable[tableNum][i]], refColName);
    }

    indexManager->showIndex(name.c_str());
    printf("==========   End  Table Info  ==========\n");
    // printf("====================begin %s====================\n", name.c_str());
    // for(int i = 0; i < tableHeader->colNum; i++) {
    //     printf("%s|", tableHeader->entrys[i].colName);
    // }
    // printf("\n===============================================\n");
    // for(int i = 0; i < tableHeader->colNum; i++) {
    //     switch (tableHeader->entrys[i].colType)
    //     {
    //     case 0:
    //         printf("NULL|");
    //         break;
    //     case 1:
    //         printf("INT|");
    //         break;
    //     case 2:
    //         printf("VARCHAR(%d)|", tableHeader->entrys[i].colLen);
    //         break;
    //     case 3:
    //         printf("FLOAT|");
    //         break;
    //     default:
    //         break;
    //     }
    // }
    // printf("\n=====================end======================\n");
    return 0;
}

int TableManager::renameTable(string oldName, string newName) {
    if(!checkTableName(newName))
        return -1;

    string oldPath = "database/" + databaseName + '/' + oldName +".db";
    if(!checkTableExist(oldPath)) {
        printf("[ERROR] table named %s does not exist !\n", oldName.c_str());
        return -1;
    }

    fileHandler = recordManager->findTable(oldName.c_str());
    if(fileHandler == nullptr) {
        printf("[ERROR] can not find the table named %s !\n", oldName.c_str());
        return -1;
    }
    fileHandler->renameTable(newName.c_str());
    recordManager->renameTable(oldName.c_str(), newName.c_str());
    if(recordManager->closeFile(fileHandler) != 0) {
        printf("[ERROR] can not close the file before rename it !\n");
        return-1;
    }

    string newPath = "database/" + databaseName + '/' + newName +".db";
    int ret = rename(oldPath.c_str(), newPath.c_str());
    if(ret != 0) {
        printf("[ERROR] can not rename the table !\n");
        return -1;
    }
    
    if((fileHandler = recordManager->openFile(newName.c_str())) == nullptr) {
        printf("[ERROR] can not open the file after rename it !\n");
        return -1;
    }
    indexManager->renameIndex(oldName.c_str(), newName.c_str());
    return 0;
}

int TableManager::saveChangeToFile(const char* tableName) {
    fileHandler = recordManager->findTable(tableName);
    if (fileHandler == nullptr) {
        return -1;
    }
    recordManager->closeFile(fileHandler);
    return 0;
}

void TableManager::initIndex(vector<string> tableNames, vector<vector<string>> colNames) {
    vector<vector<uint16_t>> indexLens;
    vector<vector<uint8_t>> colTypes;
    for (int i = 0; i < tableNames.size(); i++) {
        indexLens.push_back(vector<uint16_t>());
        colTypes.push_back(vector<uint8_t>());
        for (int j = 0; j < colNames[i].size(); j++) {
            fileHandler = recordManager->findTable(tableNames[i].c_str());
            TableHeader* tableHeader = fileHandler->getTableHeader();
            int index = checkColExist(tableHeader, colNames[i][j].c_str());
            if (index >= 0) {
                uint8_t colType = tableHeader->entrys[index].colType;
                uint16_t indexLen = tableHeader->entrys[index].colLen;
                indexLens[i].push_back(indexLen);
                colTypes[i].push_back(colType);
            }
        }
    }
    indexManager->initIndex(tableNames, colNames, indexLens, colTypes);
}

int TableManager::createIndex(string tableName, string colName) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }

    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    uint8_t colType;
    uint16_t indexLen;
    int index = checkColExist(tableHeader, colName.c_str());
    if (index >= 0) {
        colType = tableHeader->entrys[index].colType;
        indexLen = tableHeader->entrys[index].colLen;
    } else { // column not found
        printf("[ERROR] specified column does not exist.\n");
        return -1;
    }

    if (indexManager->hasIndex(tableName.c_str(), colName.c_str())) {
        printf("[INFO] the index has already been created.\n");
        return -1;
    }

    int res = _createAndAddIndex(tableName, colName, indexLen, colType, index);
    return res;    
}

int TableManager::dropIndex(string tableName, string colName, DBMeta* dbMeta) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    // check column exists? maybe do not have to
    if (!indexManager->hasIndex(tableName.c_str(), colName.c_str())) {
        printf("[INFO] No index on %s.%s\n", tableName.c_str(), colName.c_str());
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int index = checkColExist(tableHeader, colName.c_str());
    if (tableHeader->entrys[index].uniqueConstraint || tableHeader->entrys[index].primaryKeyConstraint || dbMeta->foreignKeyOnCol[tableNum][index] > 0) {
        printf("[ERROR] can not drop this index due to constraints.\n");
        return -1;
    }
    return indexManager->removeIndex(tableName.c_str(), colName.c_str());
}

bool TableManager::hasIndex(string tableName, string colName) {
    return indexManager->hasIndex(tableName.c_str(), colName.c_str());
}

int TableManager::showIndex() {
    printf("===== Show Indexes =====\n");
    int cnt = indexManager->showIndex();
    printf("==== %d indexes in total. =====\n", cnt);
    return 0;
}

int TableManager::_createAndAddIndex(string tableName, string colName, uint16_t indexLen, uint8_t colType, int index) {
    int res = indexManager->createIndex(tableName.c_str(), colName.c_str(), indexLen, colType);
    // add pre-existing records to the newly-added index
    std::vector<Record*> records;
    std::vector<RecordId*> recordIds;
    fileHandler->getAllRecordsAccordingToFields(records, recordIds, (1 << index));
    std::vector<void*> insertDatas;
    std::vector<int> insertVals;
    std::vector<int> pageIds, slotIds;
    for (int i = 0; i < records.size(); i++) {
        insertDatas.push_back((void*)(((uint8_t*)records[i]->data) + sizeof(int16_t)));
        pageIds.push_back(recordIds[i]->getPageId());
        slotIds.push_back(recordIds[i]->getSlotId());
    }
    insertVals.resize(records.size());
    fileHandler->transform(insertVals, pageIds, slotIds);
    indexManager->insert(tableName.c_str(), colName.c_str(), insertDatas, insertVals);
    for (int i = 0; i < records.size(); i++) {
        delete records[i];
        delete recordIds[i];
    }
    return res;
}

int TableManager::createPrimaryKey(string tableName, string colName) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int index = checkColExist(tableHeader, colName.c_str());

    // make sure there is only one primary key
    if (tableHeader->hasPrimaryKey()) {
        printf("[ERROR] can not add primary key to this table.\n");
        return -1;
    }

    uint16_t indexLen;
    uint8_t colType;
    if (index >= 0) {
        tableHeader->entrys[index].primaryKeyConstraint = true;
        // tableHeader->entrys[index].uniqueConstraint = false; // make sure primaryKeyConstraint and uniqueConstrain don't co-exist
        indexLen = tableHeader->entrys[index].colLen;
        colType = tableHeader->entrys[index].colType;
        fileHandler->saveTableHeader();
    } else {
        printf("[ERROR] specified column dose not exit.\n");
        return -1;
    }
    if (indexManager->hasIndex(tableName.c_str(), colName.c_str())) {
        printf("[INFO] the index has already been created.\n");
        return index;
    }
    // add pre-existing records to the newly-added index
    // however when primary key is enabled as the table is created
    // there should be no record to add

    _createAndAddIndex(tableName, colName, indexLen, colType, index);
    return index;
}

int TableManager::dropPrimaryKey(string tableName, int colId, DBMeta* dbMeta, int& indexDropped, char* pkColName) {
    indexDropped = 0;
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int index = colId;
    char colName[64];
    tableHeader->getCol(index, colName);
    memcpy(pkColName, colName, strlen(colName));
    if (index >= 0) {
        if (tableHeader->entrys[index].primaryKeyConstraint == false) {
            printf("[INFO] there is no primary key constraint on column %s\n", colName);
            return -1;
        }
        tableHeader->entrys[index].primaryKeyConstraint = false;
        fileHandler->saveTableHeader();
    } else {
        printf("[ERROR] specified column does not exist.\n");
        return -1;
    }

    int tableNum = -1, indexNum = -1;
    for(int i = 0; i < dbMeta->tableNum; i++) {
        if(strcmp(tableName.c_str(), dbMeta->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    for (int i = 0; i < dbMeta->indexNum[tableNum]; i++) {
        if (strcmp(dbMeta->indexNames[tableNum][i], colName) == 0) {
            indexNum = i;
            break;
        }
    }

    /* 
        if the index has been mannually set up, then it should not be removed
        when the primary key is dropped
     */
    if (!dbMeta->mannuallyCreateIndex[tableNum][indexNum] && !tableHeader->entrys[index].uniqueConstraint && dbMeta->foreignKeyOnCol[tableNum][index] == 0)  {
        printf("[INFO] automatically remove Index on %s.%s.\n", tableName.c_str(), colName);
        indexManager->removeIndex(tableName.c_str(), colName);
        indexDropped = 1;
    }

    return index;
}

int TableManager::createForeignKey(string tableName, string foreignKeyName, string colName, string refTableName, string refTableCol, int& refIndex) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    if(!checkTableName(refTableName))
        return -1;
    string tablePath = "database/" + databaseName + '/' + refTableName +".db";
    if(!checkTableExist(tablePath)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    if (tableName == refTableName) {
        printf("[ERROR] can not build foreign key within one table.\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int index = checkColExist(tableHeader, colName.c_str());
    FileHandler* refFileHandler = recordManager->findTable(refTableName.c_str());
    TableHeader* refTableHeader = refFileHandler->getTableHeader();
    if (refFileHandler == nullptr) {
        fprintf(stderr, "refFileHandler is nullptr in createForeignKey.\n");
        return -1;
    }
    refIndex = checkColExist(refFileHandler->getTableHeader(), refTableCol.c_str());
    if (index >= 0 && refIndex >= 0) {
        if (refTableHeader->entrys[refIndex].primaryKeyConstraint == false) {
            printf("[ERROR] can not build foreign key on ref table's non-primary-key field.\n");
            return -1;
        }
        /* 
            According to course doc, foreign column doesn't have to be parimary key nor have index
            Updata on 2022/12/25: according to the Wechat Group, foreign key can only be created on primary key (￣、￣)
         */
        if (tableHeader->getTableEntryDesc().getCol(index)->colType != refTableHeader->getTableEntryDesc().getCol(refIndex)->colType) {
            printf("[ERROR] can not build foreign key between column of different type.\n");
            return -1;
        }
        if (!indexManager->hasIndex(tableName.c_str(), colName.c_str())) {
            printf("[INFO] automatically build index for local table.\n");
            _createAndAddIndex(tableName, colName, tableHeader->entrys[index].colLen, tableHeader->entrys[index].colType, index);
        }
        if (!indexManager->hasIndex(refTableName.c_str(), refTableCol.c_str())) {
            printf("[INFO] automatically build index for foreign table.\n");
            _createAndAddIndex(refTableName, refTableCol, refTableHeader->entrys[refIndex].colLen, refTableHeader->entrys[refIndex].colType, refIndex);
        }
        if (tableHeader->entrys[index].foreignKeyConstraint == MAX_FOREIGN_KEY_FOR_COL) {
            printf("[ERROR] can not add more foreign keys for column %s.\n", colName.c_str());
            return -1;
        }
        strcpy(tableHeader->entrys[index].foreignKeyTableName[tableHeader->entrys[index].foreignKeyConstraint], refTableName.c_str());
        strcpy(tableHeader->entrys[index].foreignKeyColName[tableHeader->entrys[index].foreignKeyConstraint], refTableCol.c_str());
        tableHeader->entrys[index].foreignKeyConstraint++;
        fileHandler->saveTableHeader();
    } else {
        printf("[ERROR] specified column does not exist.\n");
        return -1;
    }
    return index;
}

int TableManager::dropForeignKey(string tableName, uint8_t colIndex, DBMeta* dbMeta, string refTableName, int refColIndex, int& indexDropped, int& refIndexDropped, char* resColName, char* resRefColName) {
    indexDropped = 0;
    refIndexDropped = 0;
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int foreignKeyIndex = -1;
    FileHandler* refFileHandler = recordManager->findTable(refTableName.c_str());
    char refColName[64];
    refFileHandler->getTableHeader()->getCol(refColIndex, refColName);
    for (int i = 0; i < tableHeader->entrys[colIndex].foreignKeyConstraint; i++) {
        if (strcmp(tableHeader->entrys[colIndex].foreignKeyTableName[i], refTableName.c_str()) == 0 && strcmp(tableHeader->entrys[colIndex].foreignKeyColName[i], refColName) == 0) {
            foreignKeyIndex = i;
            break;
        }
    }
    if(foreignKeyIndex == -1) {
        printf("[ERROR] this table dose not have this foreign key\n");
        return -1;
    }
    
    // remove the index created on foreign table
    int refIndex = checkColExist(refFileHandler->getTableHeader(), refColName);
    if (indexManager->hasIndex(tableName.c_str(), tableHeader->entrys[colIndex].colName)) {
        int tableNum = -1, indexNum = -1;
        for(int i = 0; i < dbMeta->tableNum; i++) {
            if(strcmp(tableName.c_str(), dbMeta->tableNames[i]) == 0) {
                tableNum = i;
                break;
            }
        }
        for (int i = 0; i < dbMeta->indexNum[tableNum]; i++) {
            if (strcmp(dbMeta->indexNames[tableNum][i], tableHeader->entrys[colIndex].colName) == 0) {
                indexNum = i;
                break;
            }
        }
        if (!dbMeta->mannuallyCreateIndex[tableNum][indexNum] && !dbMeta->isPrimaryKey[tableNum][colIndex] && !dbMeta->isUniqueKey[tableNum][colIndex]) {
            if (dbMeta->foreignKeyOnCol[tableNum][colIndex] == 1) {
                printf("[INFO] automatically remove Index on %s.%s.\n", tableName.c_str(), tableHeader->entrys[colIndex].colName);
                indexManager->removeIndex(tableName.c_str(), tableHeader->entrys[colIndex].colName);
                indexDropped = 1;
                strcpy(resColName, tableHeader->entrys[colIndex].colName);
            }
        }
    }
    if (indexManager->hasIndex(refTableName.c_str(), refColName)) {
        int tableNum = -1, indexNum = -1;
        for(int i = 0; i < dbMeta->tableNum; i++) {
            if(strcmp(refTableName.c_str(), dbMeta->tableNames[i]) == 0) {
                tableNum = i;
                break;
            }
        }
        for (int i = 0; i < dbMeta->indexNum[tableNum]; i++) {
            if (strcmp(dbMeta->indexNames[tableNum][i], refColName) == 0) {
                indexNum = i;
                break;
            }
        }
        if (!dbMeta->mannuallyCreateIndex[tableNum][indexNum] && !dbMeta->isPrimaryKey[tableNum][refIndex] && !dbMeta->isUniqueKey[tableNum][refIndex]) {
            if (dbMeta->foreignKeyOnCol[tableNum][refIndex] == 1) {
                printf("[INFO] automatically remove Index on %s.%s.\n", refTableName.c_str(), refColName);
                indexManager->removeIndex(refTableName.c_str(), refColName);
                refIndexDropped = 1;
                strcpy(resRefColName, refColName);
            }
        }
    }

    strcpy(tableHeader->entrys[colIndex].foreignKeyTableName[foreignKeyIndex], tableHeader->entrys[colIndex].foreignKeyTableName[tableHeader->entrys[colIndex].foreignKeyConstraint-1]);
    strcpy(tableHeader->entrys[colIndex].foreignKeyColName[foreignKeyIndex], tableHeader->entrys[colIndex].foreignKeyColName[tableHeader->entrys[colIndex].foreignKeyConstraint-1]);
    tableHeader->entrys[colIndex].foreignKeyConstraint--;
    fileHandler->saveTableHeader();
    return 0;
}

int TableManager::createUniqueKey(string tableName, string colName) {
    int res = createIndex(tableName, colName);
    // the index is established. Next should build unique constraint
    if (res >= 0) {
        fileHandler = recordManager->findTable(tableName.c_str());
        TableHeader* tableHeader = fileHandler->getTableHeader();
        int index = checkColExist(tableHeader, colName.c_str());
        if (tableHeader->entrys[index].uniqueConstraint == true) {
            printf("[INFO] the unique constraint of %s has already been created.\n", colName.c_str());
            return index;
        } /* else if (tableHeader->entrys[index].primaryKeyConstraint == true) {
            printf("[ERROR] can not create unique constraint on primary key.\n");
            return -1;
        } */
        tableHeader->entrys[index].uniqueConstraint = true;
        fileHandler->saveTableHeader();
        return index;
    }
    return res;
}

int TableManager::dropUniqueKey(string tableName, string colName, DBMeta* dbMeta, int& indexDropped) {
    indexDropped = 0;
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int index = checkColExist(tableHeader, colName.c_str());
    if (index >= 0) {
        if (tableHeader->entrys[index].uniqueConstraint == false) {
            printf("[INFO] specified column dose not have unique constraint.\n");
            return -1;
        }
        tableHeader->entrys[index].uniqueConstraint = false;
    } else {
        printf("[ERROR] specified column does not exist.\n");
        return -1;
    }

    if (!indexManager->hasIndex(tableName.c_str(), colName.c_str())) {
        fprintf(stderr, "indexManager has no index on uniquekey.\n");
        return -1;
    }

    int tableNum = -1, indexNum = -1;
    for(int i = 0; i < dbMeta->tableNum; i++) {
        if(strcmp(tableName.c_str(), dbMeta->tableNames[i]) == 0) {
            tableNum = i;
            break;
        }
    }
    for (int i = 0; i < dbMeta->indexNum[tableNum]; i++) {
        if (strcmp(dbMeta->indexNames[tableNum][i], colName.c_str()) == 0) {
            indexNum = i;
            break;
        }
    }

    if (!dbMeta->mannuallyCreateIndex[tableNum][indexNum] && !tableHeader->entrys[index].primaryKeyConstraint && dbMeta->foreignKeyOnCol[tableNum][index] == 0) {
        printf("[INFO] automatically remove Index on %s.%s.\n", tableName.c_str(), colName.c_str());
        indexManager->removeIndex(tableName.c_str(), colName.c_str());
        indexDropped = 1;
    }
    fileHandler->saveTableHeader();
    return index;
}

int TableManager::_checkFormat(FileHandler* fileHandlers[], TableHeader* tableHeaders[], vector<string> &selectTables, vector<DBExpItem*> &waitChecked) {
    int tableSize = selectTables.size();
    for (int i = 0; i < waitChecked.size(); i++) {
        if (waitChecked[i]->expTable != "") {
            int j;
            for (j = 0; j < tableSize; j++) {
                if (!strcmp(waitChecked[i]->expTable.c_str(), selectTables[j].c_str())) {
                    break;
                }
            }
            if (j == tableSize) {
                printf("[ERROR] no such table in %s.%s\n", waitChecked[i]->expTable.c_str(), waitChecked[i]->expCol.c_str());
                return -1;
            }
            int index = checkColExist(tableHeaders[j], waitChecked[i]->expCol.c_str());
            if (index < 0) {
                printf("[ERROR] no such column %s in table%s.\n", waitChecked[i]->expCol.c_str(), waitChecked[i]->expTable.c_str());
                return -1;
            }
        } else {
            int hitTable = 0;
            for (int j = 0; j < tableSize; j++) {
                if (checkColExist(tableHeaders[j], waitChecked[i]->expCol.c_str()) >= 0) {
                    hitTable++;
                    waitChecked[i]->expTable = selectTables[j];
                }
            }
            if (hitTable == 0) {
                printf("[ERROR] no table has such column %s.\n", waitChecked[i]->expCol.c_str());
                return -1;
            } else if (hitTable >= 2) {
                printf("[ERROR] ambiguous column %s.\n", waitChecked[i]->expCol.c_str());
                return -1;
            }
        }
    }
    return 0;
}

inline bool compareEqual(RecordDataNode* cur, RecordDataNode* pre) {
    if (!cur || !pre || cur->nodeType != pre->nodeType || cur->len != pre->len) {
        return false;
    }
    switch (cur->nodeType)
    {
        case COL_INT:
            return *(int*)(cur->content.intContent) == *(int*)(pre->content.intContent);
            break;
        case COL_FLOAT:
            return *(float*)(cur->content.floatContent) == *(float*)(pre->content.floatContent);
            break;
        case COL_VARCHAR:
            return (strcmp((char*)(cur->content.charContent), (char*)(pre->content.charContent)) == 0);
            break;
        default:
            return true;
            break;
    }
    return false;
}

inline void* transformType(RecordDataNode* recordDataNode) {
    void* res = nullptr;
    switch (recordDataNode->nodeType) {
        case COL_INT:
            res = recordDataNode->content.intContent;
            break;
        case COL_FLOAT:
            res = recordDataNode->content.floatContent;
            break;
        case COL_VARCHAR:
            res = recordDataNode->content.charContent;
            break;
        default:
            recordDataNode->len = 0;
            break;
    }
    return res;
}

inline void writeBackData(void* data, RecordDataNode* recordDataNode) {
    switch (recordDataNode->nodeType) {
        case COL_INT:
            recordDataNode->content.intContent = (int*)data;
            break;
        case COL_FLOAT:
            recordDataNode->content.floatContent = (float*)data;
            break;
        case COL_VARCHAR:
            recordDataNode->content.charContent = (char*)data;
            break;
        default:
            break;
    }
}

int getMapIndex(RecordDataNode* cur, void* ma, int &cnt) {
    switch (cur->nodeType) {
        case COL_INT:
            if (((map<int, int>*)ma)->count(*cur->content.intContent) == 0) {
                (*(map<int, int>*)ma)[*cur->content.intContent] = ++cnt;
                return cnt;
            }
            return (*(map<int, int>*)ma)[*cur->content.intContent];
        case COL_FLOAT:
            if (((map<float, int>*)ma)->count(*cur->content.floatContent) == 0) {
                (*(map<float, int>*)ma)[*cur->content.floatContent] = ++cnt;
                return cnt;
            }
            return (*(map<float, int>*)ma)[*cur->content.floatContent];
        case COL_VARCHAR:
            if (((map<string, int>*)ma)->count((string)(cur->content.charContent)) == 0) {
                (*(map<string, int>*)ma)[(string)cur->content.charContent] = ++cnt;
                return cnt;
            }
            return (*(map<string, int>*)ma)[(string)cur->content.charContent];
    }
    return -1;
}

int getMapIndex(void* cur, uint8_t colType, void* ma, int &cnt) {
    switch (colType) {
        case COL_INT:
            if (((map<int, int>*)ma)->count(*(int*)cur) == 0) {
                (*(map<int, int>*)ma)[*(int*)cur] = ++cnt;
                return cnt;
            }
            return (*(map<int, int>*)ma)[*(int*)cur];
        case COL_FLOAT:
            if (((map<float, int>*)ma)->count(*(float*)cur) == 0) {
                (*(map<float, int>*)ma)[*(float*)cur] = ++cnt;
                return cnt;
            }
            return (*(map<float, int>*)ma)[*(float*)cur];
        case COL_VARCHAR:
            if (((map<string, int>*)ma)->count((string)((char*)cur)) == 0) {
                (*(map<string, int>*)ma)[(string)((char*)cur)] = ++cnt;
                return cnt;
            }
            return (*(map<string, int>*)ma)[(string)((char*)cur)];
    }
    return -1;
}

int TableManager::_iterateWhere(vector<string> selectTables, vector<DBExpression> expressions, vector<RecordId*>& resRecordIds) {
    int cur = 0;
    std::vector<RecordId*> res[2];
    FileHandler* fileHandlers[MAX_SELECT_TABLE];
    TableHeader* tableHeaders[MAX_SELECT_TABLE];
    std::vector<Record*> records[MAX_SELECT_TABLE];
    std::vector<RecordId*> recordIds[MAX_SELECT_TABLE];
    std::vector<Record*> optimizeRecords;
    std::vector<RecordId*> optimizeRecordIds, opRecordIds;
    int tableNum = selectTables.size();
    for (int i = 0; i < tableNum; i++) {
        fileHandlers[i] = recordManager->findTable(selectTables[i].c_str());
        tableHeaders[i] = fileHandlers[i]->getTableHeader();
        fileHandlers[i]->getAllRecords(records[i], recordIds[i]);
    }
    bool initialOptimize = false;
    if (expressions.size() > 0 && ((expressions[0].op >= EQU_TYPE && expressions[0].op <= LTE_TYPE && expressions[0].rType >= DB_INT && expressions[0].rType <= DB_CHAR) || (expressions[0].rType == DB_NST) || (expressions[0].rType == DB_LIST))) {
        DBExpItem* lItem = (DBExpItem*)expressions[0].lVal;
        if (hasIndex(lItem->expTable, lItem->expCol)) {
            int fileId = (lItem->expTable == selectTables[0]) ? 0 : 1;
            int colId = tableHeaders[fileId]->getCol(lItem->expCol.c_str());
            TableEntryDesc tableEntryDesc = tableHeaders[fileId]->getTableEntryDesc();
            TableEntryDescNode* tableEntryDescNode = tableEntryDesc.getCol(colId);
            std::vector<int> val, tempRes;
            bool flag = false; // flag is used to integrate nesty selection and normal selection when op is an comparison operator
            void* nstSearchData = expressions[0].rVal;
            vector<RecordData> tempRecordData;
            vector<string> tempColNames;
            if (expressions[0].rType == DB_LIST) {
                if (expressions[0].op != IN_TYPE) {
                    printf("[ERROR] operation type of DB_LIST must be IN_TYPE.\n");
                    return -1;
                }
                void* ma = nullptr;
                switch (tableEntryDescNode->colType) { // build map
                    case COL_INT: ma = new map<int, int>; break;
                    case COL_FLOAT: ma = new map<float, int>; break;
                    case COL_VARCHAR: ma = new map<string, int>; break;
                    default: return -1; break;
                }
                int cnt = 0;
                std::vector<void*>* valueList = (std::vector<void*>*)expressions[0].rVal;
                for (int i = 0; i < expressions[0].valueListType.size(); i++) {
                    if (expressions[0].valueListType[i] != (DB_LIST_TYPE)tableEntryDescNode->colType) {
                        continue;
                    }
                    int lastCnt = cnt;
                    getMapIndex((*valueList)[i], tableEntryDescNode->colType, ma, cnt);
                    if (cnt == lastCnt) {
                        continue;
                    }
                    std::vector<int> tempVal;
                    indexManager->search(lItem->expTable.c_str(), lItem->expCol.c_str(), (*valueList)[i], tempVal);
                    for (int j = 0; j < tempVal.size(); j++) {
                        val.push_back(tempVal[j]);
                    }
                }
                switch (tableEntryDescNode->colType) { // build map
                    case COL_INT: delete (map<int, int>*)ma; break;
                    case COL_FLOAT: delete (map<float, float>*)ma; break;
                    case COL_VARCHAR: delete (map<string, int>*)ma; break;
                    default: break;
                }
            }
            if (expressions[0].rType == DB_NST) {
                if (expressions[0].op > LTE_TYPE && expressions[0].op != IN_TYPE) {
                    printf("[ERROR] op %d is not supported for nesty selection.\n", expressions[0].op);
                    return -1;
                }
                if (((DBSelect*)(expressions[0].rVal))->selectItems.size() > 1) {
                    printf("[ERROR] nesty selection in tuple form is not supported.\n");
                    return -1;
                }
                if (_selectRecords((DBSelect*)expressions[0].rVal, tempRecordData, tempColNames) == -1) {
                    return -1;
                }
                if (tempRecordData.size() > 0 && tempRecordData[0].head->nodeType != tableEntryDescNode->colType) {
                    printf("[ERROR] incompatible type between data and result of nesty selection.\n");
                    return -1;
                }
                if (expressions[0].op == IN_TYPE) {
                    void* ma = nullptr;
                    switch (tableEntryDescNode->colType) { // build map
                        case COL_INT: ma = new map<int, int>; break;
                        case COL_FLOAT: ma = new map<float, int>; break;
                        case COL_VARCHAR: ma = new map<string, int>; break;
                        default: return -1; break;
                    }
                    int cnt = 0;
                    for (int i = 0; i < tempRecordData.size(); i++) {
                        void* rData = transformType(tempRecordData[i].head);
                        int lastCnt = cnt;
                        getMapIndex(tempRecordData[i].head, ma, cnt);
                        if (cnt == lastCnt) { // duplicate item
                            continue;
                        }
                        std::vector<int> tempVal;
                        indexManager->search(lItem->expTable.c_str(), lItem->expCol.c_str(), rData, tempVal);
                        for (int j = 0; j < tempVal.size(); j++) {
                            val.push_back(tempVal[j]);
                        }
                    }
                    switch (tableEntryDescNode->colType) { // build map
                        case COL_INT: delete (map<int, int>*)ma; break;
                        case COL_FLOAT: delete (map<float, float>*)ma; break;
                        case COL_VARCHAR: delete (map<string, int>*)ma; break;
                        default: break;
                    }
                } else {
                    if (tempRecordData.size() > 1) {
                        printf("[ERROR] the result of child select contains more than one record.\n");
                        return -1;
                    }
                    flag = true;
                    nstSearchData = transformType(tempRecordData[0].head);
                }
            }
            if (flag || (expressions[0].rType >= DB_INT && expressions[0].rType <= DB_CHAR)) {
                if (!flag && tableEntryDescNode->colType != expressions[0].rType) {
                    printf("[ERROR] incompatible type for %s.%s and %d.\n", lItem->expTable.c_str(), lItem->expCol.c_str(), expressions[0].rType);
                    return -1;
                }
                switch (expressions[0].op) {
                    case EQU_TYPE:
                        indexManager->search(lItem->expTable.c_str(), lItem->expCol.c_str(), nstSearchData, val);
                        break;
                    case NEQ_TYPE:
                        indexManager->searchBetween(lItem->expTable.c_str(), lItem->expCol.c_str(), nullptr, nstSearchData, tempRes, true, false);
                        indexManager->searchBetween(lItem->expTable.c_str(), lItem->expCol.c_str(), nstSearchData, nullptr, val, false, true);
                        for (int k = 0; k < tempRes.size(); k++) {
                            val.push_back(tempRes[k]);
                        }
                        break;
                    case LT_TYPE:
                        indexManager->searchBetween(lItem->expTable.c_str(), lItem->expCol.c_str(), nullptr, nstSearchData, val, true, false);
                        break;
                    case LTE_TYPE:
                        indexManager->searchBetween(lItem->expTable.c_str(), lItem->expCol.c_str(), nullptr, nstSearchData, val, true, true);
                        break;
                    case GT_TYPE:
                        indexManager->searchBetween(lItem->expTable.c_str(), lItem->expCol.c_str(), nstSearchData, nullptr, val, false, true);
                        break;
                    case GTE_TYPE:
                        indexManager->searchBetween(lItem->expTable.c_str(), lItem->expCol.c_str(), nstSearchData, nullptr, val, true, true);
                        break;
                    default:
                        fprintf(stderr, "encouter error op type %d in search.\n", expressions[0].op);
                        return -1;
                        break;
                }
            }
            for (int i = 0; i < val.size(); i++) {
                int pageId, slotId;
                fileHandlers[fileId]->transformR(val[i], pageId, slotId);
                RecordId* recordId = new RecordId(pageId, slotId);
                if (tableNum == 1) {
                    res[cur].push_back(recordId);
                } else if (tableNum == 2) {
                    for (auto recordIdJ : recordIds[fileId ^ 1]) {
                        if (fileId == 0) {
                            res[cur].push_back(recordId);
                            res[cur].push_back(recordIdJ);
                        } else {
                            res[cur].push_back(recordIdJ);
                            res[cur].push_back(recordId);
                        }
                    }
                } else {
                    return -1;
                }
            }
            initialOptimize = true;
        }
    }
    if (expressions.size() > 0 && expressions[0].rType == DB_ITEM) {
        DBExpItem* lItem = (DBExpItem*)expressions[0].lVal;
        DBExpItem* rItem = (DBExpItem*)expressions[0].rVal;
        int lfileId = (lItem->expTable == selectTables[0]) ? 0 : 1;
        int rfileId = (rItem->expTable == selectTables[0]) ? 0 : 1;
        if (lfileId != rfileId && hasIndex(rItem->expTable, rItem->expCol)) {
            int lcolId = tableHeaders[lfileId]->getCol(lItem->expCol.c_str());
            int rcolId = tableHeaders[rfileId]->getCol(rItem->expCol.c_str());
            TableEntryDesc ltableEntryDesc = tableHeaders[lfileId]->getTableEntryDesc();
            TableEntryDesc rtableEntryDesc = tableHeaders[rfileId]->getTableEntryDesc();
            TableEntryDescNode* ltableEntryDescNode = ltableEntryDesc.getCol(lcolId);
            TableEntryDescNode* rtableEntryDescNode = rtableEntryDesc.getCol(rcolId);
            if (ltableEntryDescNode->colType != rtableEntryDescNode->colType) {
                printf("[ERROR] %s.%s and %s.%s don't have compatible types.\n", lItem->expTable.c_str(), lItem->expCol.c_str(), rItem->expTable.c_str(), rItem->expCol.c_str());
                return -1;
            }
            fileHandlers[lfileId]->getAllRecordsAccordingToFields(optimizeRecords, optimizeRecordIds, 1 << lcolId);
            void* searchData;
            for (int i = 0; i < recordIds[rfileId].size(); i++) {
                opRecordIds.push_back(new RecordId(-1, -1));
            }
            for (int i = 0; i < optimizeRecords.size(); i++) {
                searchData = (void*)(((uint8_t*)optimizeRecords[i]->data) + sizeof(int16_t));
                std::vector<int> val, tempVal;
                switch (expressions[0].op) {
                    case EQU_TYPE: indexManager->search(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, val); break;
                    case NEQ_TYPE:
                        indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), nullptr, searchData, val, true, false);
                        indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, nullptr, tempVal, false, true);
                        for (int k = 0; k < tempVal.size(); k++) {
                            val.push_back(tempVal[k]);
                        }
                        break;
                    case LT_TYPE:
                        indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, nullptr, val, false, true);
                        break;
                    case LTE_TYPE:
                        indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, nullptr, val, true, true);
                        break;
                    case GT_TYPE:
                        indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), nullptr, searchData, val, true, false);
                        break;
                    case GTE_TYPE:
                        indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), nullptr, searchData, val, true, true);
                        break;
                    default:
                        fprintf(stderr, "encouter error op type %d in search.\n", expressions[i].op);
                        return -1;
                        break;
                }
                for (int j = 0; j < val.size(); j++) {
                    int pageId, slotId, pos = val[j];
                    fileHandlers[rfileId]->transformR(pos, pageId, slotId);
                    while (pos >= opRecordIds.size()) {
                        opRecordIds.push_back(new RecordId(-1, -1));
                    }
                    opRecordIds[pos]->set(pageId, slotId);
                    if (lfileId == 0) {
                        res[cur].push_back(optimizeRecordIds[i]);
                        res[cur].push_back(opRecordIds[pos]);
                    } else {
                        res[cur].push_back(opRecordIds[pos]);
                        res[cur].push_back(optimizeRecordIds[i]);
                    }
                }
            }
            initialOptimize = true;
        }
    }
    if (!initialOptimize) {
        if (tableNum == 1) {
            for (auto recordId : recordIds[0]) {
                res[cur].push_back(recordId);
            }
        } else if (tableNum == 2) {
            for (auto recordIdI : recordIds[0]) {
                for (auto recordIdJ : recordIds[1]) {
                    res[cur].push_back(recordIdI);
                    res[cur].push_back(recordIdJ);
                }
            }
        } else {
            // tableNum > 2
            // should not be here
            return -1;
        }
    }
    for (int i = 0; i < expressions.size(); i++) {
        if (i == 0 && initialOptimize) {
            continue;
        }
        bool preFlag = false; // flag indicating the previous item has been successfully selected
        cur ^= 1;
        res[cur].clear();
        if (expressions[i].lType != DB_ITEM) { // in SQL.g4, the left must be an item;
            fprintf(stderr, "the left must be an item.\n");
            return -1;
        }
        DBExpItem* lItem = (DBExpItem*)expressions[i].lVal;
        int fileId = (lItem->expTable == selectTables[0]) ? 0 : 1;
        Record curRecord(fileHandlers[fileId]->getRecordLen());
        RecordId* curRecordId;
        RecordData curRecordData;
        RecordDataNode* curRecordDataNode, *preRecordDataNode = nullptr;
        for (int j = fileId; j < res[cur ^ 1].size(); j += tableNum) {
            curRecordId = res[cur ^ 1][j];
            fileHandlers[fileId]->getRecord(*curRecordId, curRecord);
            int colId = tableHeaders[fileId]->getCol(lItem->expCol.c_str());
            TableEntryDesc tableEntryDesc = tableHeaders[fileId]->getTableEntryDesc();
            curRecord.deserialize(curRecordData, tableEntryDesc);
            curRecordDataNode = curRecordData.getData(colId);
            void* searchData = transformType(curRecordDataNode);
            Compare* compare = nullptr;
            switch (curRecordDataNode->nodeType) {
                case COL_INT:
                    compare = new IntCompare();
                    break;
                case COL_FLOAT:
                    compare = new FloatCompare();
                    break;
                case COL_VARCHAR:
                    compare = new CharCompare();
                    break;
                default:
                    break;
            }
            bool equalAsPre = false;
            if (preRecordDataNode && compareEqual(curRecordDataNode, preRecordDataNode)) {
                // same operation need not to search again
                equalAsPre = true;
            }
            if (expressions[i].rType != DB_NULL && curRecordDataNode->nodeType == COL_NULL) {
                printf("[ERROR] null type currently is not supported in op other than IS(NOT) NULL.\n");
                return -1;
            }
            if (expressions[i].rType == DB_ITEM) {
                DBExpItem* rItem = (DBExpItem*)(expressions[i].rVal);
                int rFileId = (rItem->expTable == selectTables[0]) ? 0 : 1;
                int rColId = tableHeaders[rFileId]->getCol(rItem->expCol.c_str());
                TableEntryDesc tableEntryDesc = tableHeaders[rFileId]->getTableEntryDesc();
                TableEntryDescNode* tableEntryDescNode = tableEntryDesc.getCol(rColId);
                if (tableEntryDescNode->colType != curRecordDataNode->nodeType) {
                    printf("[ERROR] %s.%s and %s.%s don't have compatible types.\n", lItem->expTable.c_str(), lItem->expCol.c_str(), rItem->expTable.c_str(), rItem->expCol.c_str());
                    return -1;
                }
                std::vector<int> indexRes, indexResTemp;
                if (rFileId == fileId) { // the same table
                    RecordDataNode* rDataNode = curRecordData.getData(rColId);
                    void* rData = transformType(rDataNode);
                    bool flag = false;
                    switch (expressions[i].op) {
                        case EQU_TYPE:
                            if (compare->equ(searchData, rData)) {
                                flag = true;
                            }
                            break;
                        case NEQ_TYPE:
                            if (!compare->equ(searchData, rData)) {
                                flag = true;
                            }
                            break;
                        case GT_TYPE:
                            if (compare->gt(searchData, rData)) {
                                flag = true;
                            }
                            break;
                        case GTE_TYPE:
                            if (compare->gte(searchData, rData)) {
                                flag = true;
                            }
                            break;
                        case LT_TYPE:
                            if (compare->lt(searchData, rData)) {
                                flag = true;
                            }
                            break;
                        case LTE_TYPE:
                            if (compare->lte(searchData, rData)) {
                                flag = true;
                            }
                            break;
                        default:
                            fprintf(stderr, "encouter error op type %d in search.\n", expressions[i].op);
                            return -1;
                            break;
                    }
                    if (flag) {
                        if (fileId == 0) {
                            res[cur].push_back(curRecordId);
                            if (tableNum > 1) {
                                res[cur].push_back(res[cur ^ 1][j ^ 1]);
                            }
                        } else {
                            res[cur].push_back(res[cur ^ 1][j ^ 1]);
                            res[cur].push_back(curRecordId);
                        }
                    }
                    continue;
                }
                // rFileId != fileId, different table
                if (equalAsPre) {
                    continue;
                } else {
                    if (hasIndex(rItem->expTable, rItem->expCol)) { // search using indexManager
                        switch (expressions[i].op) {
                            case EQU_TYPE:
                                indexManager->search(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, indexRes);
                                break;
                            case NEQ_TYPE:
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), nullptr, searchData, indexRes, true, false);
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, nullptr, indexResTemp, false, true);
                                for (int k = 0; k < indexResTemp.size(); k++) {
                                    indexRes.push_back(indexResTemp[k]);
                                }
                                break;
                            case LT_TYPE:
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, nullptr, indexRes, false, true);
                                break;
                            case LTE_TYPE:
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, nullptr, indexRes, true, true);
                                break;
                            case GT_TYPE:
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), nullptr, searchData, indexRes, true, false);
                                break;
                            case GTE_TYPE:
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), nullptr, searchData, indexRes, true, true);
                                break;
                            default:
                                fprintf(stderr, "encouter error op type %d in search.\n", expressions[i].op);
                                return -1;
                                break;
                        }
                        std::map<int, bool> ma; // hash to get the intersection of two sets
                        for (int k = 0; k < indexRes.size(); k++) {
                            ma[indexRes[k]] = true;
                        }
                        TableEntryDesc lTableEntryDesc = fileHandlers[fileId]->getTableEntryDesc();
                        for (int k = j ^ 1; k < res[cur ^ 1].size(); k += tableNum) {
                            if (res[cur ^ 1][k ^ 1]->getPageId() != curRecordId->getPageId() || res[cur ^ 1][k ^ 1]->getSlotId() != curRecordId->getSlotId()) {
                                RecordData searchedRecordData;
                                Record searchedRecord(fileHandlers[fileId]->getRecordLen());
                                fileHandlers[fileId]->getRecord(*res[cur ^ 1][k ^ 1], searchedRecord);
                                searchedRecord.deserialize(searchedRecordData, lTableEntryDesc);
                                RecordDataNode* searchedRecordDataNode = searchedRecordData.getData(colId);
                                if (!compareEqual(curRecordDataNode, searchedRecordDataNode)) {
                                    break;
                                }
                            }
                            int val;
                            RecordId* rRecordId = res[cur ^ 1][k];
                            fileHandlers[rFileId]->transform(val, rRecordId->getPageId(), rRecordId->getSlotId());
                            if (ma.count(val) > 0) {
                                if (fileId == 0) {
                                    res[cur].push_back(res[cur ^ 1][k ^ 1]);
                                    res[cur].push_back(rRecordId);
                                } else {
                                    res[cur].push_back(rRecordId);
                                    res[cur].push_back(res[cur ^ 1][k ^ 1]);
                                }
                            }
                        }

                    } else { // search using recordManager
                        RecordData searchRecordData;
                        RecordDataNode* searchRecordDataNode;
                        TableEntryDesc lTableEntryDesc = fileHandlers[fileId]->getTableEntryDesc();
                        for (int k = j ^ 1; k < res[cur ^ 1].size(); k += tableNum) {
                            if (res[cur ^ 1][k ^ 1]->getPageId() != curRecordId->getPageId() || res[cur ^ 1][k ^ 1]->getSlotId() != curRecordId->getSlotId()) {
                                RecordData searchedRecordData;
                                Record searchedRecord(fileHandlers[fileId]->getRecordLen());
                                fileHandlers[fileId]->getRecord(*res[cur ^ 1][k ^ 1], searchedRecord);
                                searchedRecord.deserialize(searchedRecordData, lTableEntryDesc);
                                RecordDataNode* searchedRecordDataNode = searchedRecordData.getData(colId);
                                if (!compareEqual(curRecordDataNode, searchedRecordDataNode)) {
                                    break;
                                }
                            }
                            RecordId* rRecordId = res[cur ^ 1][k];
                            Record searchRecord(fileHandlers[rFileId]->getRecordLen());
                            fileHandlers[rFileId]->getRecord(*rRecordId, searchRecord);
                            searchRecord.deserialize(searchRecordData, tableEntryDesc);
                            searchRecordDataNode = searchRecordData.getData(rColId);
                            void* rData = transformType(searchRecordDataNode);
                            bool flag = false;
                            switch (expressions[i].op) {
                                case EQU_TYPE:
                                    if (compare->equ(searchData, rData)) {
                                        flag = true;
                                    }
                                    break;
                                case NEQ_TYPE:
                                    if (!compare->equ(searchData, rData)) {
                                        flag = true;
                                    }
                                    break;
                                case GT_TYPE:
                                    if (compare->gt(searchData, rData)) {
                                        flag = true;
                                    }
                                    break;
                                case GTE_TYPE:
                                    if (compare->gte(searchData, rData)) {
                                        flag = true;
                                    }
                                    break;
                                case LT_TYPE:
                                    if (compare->lt(searchData, rData)) {
                                        flag = true;
                                    }
                                    break;
                                case LTE_TYPE:
                                    if (compare->lte(searchData, rData)) {
                                        flag = true;
                                    }
                                    break;
                                default:
                                    fprintf(stderr, "encouter error op type %d in search.\n", expressions[i].op);
                                    return -1;
                                    break;
                            }
                            if (flag) {
                                if (fileId == 0) {
                                    res[cur].push_back(res[cur ^ 1][k ^ 1]);
                                    res[cur].push_back(rRecordId);
                                } else {
                                    res[cur].push_back(rRecordId);
                                    res[cur].push_back(res[cur ^ 1][k ^ 1]);
                                }
                            }
                        }
                    }
                }
            } else if (expressions[i].rType == DB_NST) {
                if (expressions[i].op > LTE_TYPE && expressions[i].op != IN_TYPE) {
                    printf("[ERROR] op %d is not supported for nesty selection.\n", expressions[i].op);
                    return -1;
                }
                vector<RecordData> tempRecordData;
                vector<string> tempColNames;
                if (((DBSelect*)(expressions[i].rVal))->selectItems.size() > 1) {
                    printf("[ERROR] nesty selection in tuple form not supported.\n");
                    return -1;
                }
                if (equalAsPre && preFlag) {
                    res[cur].push_back(curRecordId);
                    if (tableNum > 1) {
                        res[cur].push_back(res[cur ^ 1][j ^ 1]);
                    }
                    continue;
                }
                // recursive search
                if (_selectRecords((DBSelect*)expressions[i].rVal, tempRecordData, tempColNames) == -1) {
                    return -1;
                }
                if (expressions[i].op == IN_TYPE) {
                    preFlag = false;
                    for (int k = 0; k < tempRecordData.size(); k++) {
                        if (tempRecordData[k].head->nodeType != curRecordDataNode->nodeType) {
                            continue;
                        }
                        void* rData = transformType(tempRecordData[k].head);
                        if (compare->equ(searchData, rData)) {
                            res[cur].push_back(curRecordId);
                            if (tableNum > 1) {
                                res[cur].push_back(res[cur ^ 1][j ^ 1]);
                            }
                            preFlag = true;
                            break;
                        }    
                    }
                } else if (expressions[i].op >= EQU_TYPE && expressions[i].op <= LTE_TYPE) {
                    preFlag = false;
                    if (tempRecordData.size() > 1) {
                        printf("[ERROR] the result of child select contains more than one record.\n");
                        return -1;
                    }
                    if (tempRecordData[0].head->nodeType != curRecordDataNode->nodeType) {
                        printf("[ERROR] can not compare between different types in child select.\n");
                        return -1;
                    }
                    bool flag = false;
                    void* childSelectData = nullptr;
                    switch (tempRecordData[0].head->nodeType) {
                        case COL_INT: childSelectData = tempRecordData[0].head->content.intContent; break;
                        case COL_FLOAT: childSelectData = tempRecordData[0].head->content.floatContent; break;
                        case COL_VARCHAR: childSelectData = tempRecordData[0].head->content.charContent; break;
                        default: break;
                    }
                    switch (expressions[i].op) {
                        case EQU_TYPE: flag = compare->equ(searchData, childSelectData);  break;
                        case NEQ_TYPE: flag = !compare->equ(searchData, childSelectData); break;
                        case GT_TYPE:  flag = compare->gt(searchData, childSelectData);   break;
                        case GTE_TYPE: flag = compare->gte(searchData, childSelectData);  break;
                        case LT_TYPE:  flag = compare->lt(searchData, childSelectData);   break;
                        case LTE_TYPE: flag = compare->lte(searchData, childSelectData);  break;
                        default: break;
                    }
                    if (flag) {
                        if (fileId == 0) {
                            res[cur].push_back(curRecordId);
                            if (tableNum > 1) {
                                res[cur].push_back(res[cur ^ 1][j ^ 1]);
                            }
                        } else {
                            res[cur].push_back(res[cur ^ 1][j ^ 1]);
                            res[cur].push_back(curRecordId);
                        }
                        preFlag = flag;
                    }
                } else {
                    fprintf(stderr, "error\n");
                    return -1;
                }
            } else if (expressions[i].rType == DB_LIST) {
                if (equalAsPre && preFlag) {
                    if (fileId == 0) {
                        res[cur].push_back(curRecordId);
                        if (tableNum > 1) {
                            res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        }
                    } else {
                        res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        res[cur].push_back(curRecordId);
                    }
                    continue;
                }
                preFlag = false;
                std::vector<void*>* valueList = (std::vector<void*>*)expressions[i].rVal;
                for (int k = 0; k < expressions[i].valueListType.size(); k++) {
                    if (expressions[i].valueListType[k] == (DB_LIST_TYPE)curRecordDataNode->nodeType) {
                        if (compare->equ((*valueList)[k], searchData)) {
                            if (fileId == 0) {
                                res[cur].push_back(curRecordId);
                                if (tableNum > 1) {
                                    res[cur].push_back(res[cur ^ 1][j ^ 1]);
                                }
                            } else {
                                res[cur].push_back(res[cur ^ 1][j ^ 1]);
                                res[cur].push_back(curRecordId);
                            }
                            preFlag = true;
                            break;
                        }
                    } 
                }
            } else if (expressions[i].rType == DB_NULL) {
                if (expressions[i].op != IS_TYPE && expressions[i].op != ISN_TYPE) {
                    fprintf(stderr, "Null should have op type IS or IS NOT.\n");
                    return -1;
                }
                if (curRecordDataNode->nodeType == COL_NULL) {
                    if (fileId == 0) {
                        res[cur].push_back(curRecordId);
                        if (tableNum > 1) {
                            res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        }
                    } else {
                        res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        res[cur].push_back(curRecordId);
                    }
                }
            } else { // DB_INT, DB_CHAR, DB_FLOAT
                if (curRecordDataNode->nodeType != (TB_COL_TYPE)expressions[i].rType) {
                    printf("[ERROR] incompatible type for %s.%s and %d.\n", lItem->expTable.c_str(), lItem->expCol.c_str(), expressions[i].rType);
                    return -1;
                }
                if (equalAsPre && preFlag) {
                    if (fileId == 0) {
                        res[cur].push_back(curRecordId);
                        if (tableNum > 1) {
                            res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        }
                    } else {
                        res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        res[cur].push_back(curRecordId);
                    }
                    continue;
                }
                preFlag = false;
                bool flag = false;
                switch (expressions[i].op) {
                    case EQU_TYPE:
                        if (compare->equ(searchData, expressions[i].rVal)) {
                            flag = true;
                        }
                        break;
                    case NEQ_TYPE:
                        if (!compare->equ(searchData, expressions[i].rVal)) {
                            flag = true;
                        }
                        break;
                    case GT_TYPE:
                        if (compare->gt(searchData, expressions[i].rVal)) {
                            flag = true;
                        }
                        break;
                    case GTE_TYPE:
                        if (compare->gte(searchData, expressions[i].rVal)) {
                            flag = true;
                        }
                        break;
                    case LT_TYPE:
                        if (compare->lt(searchData, expressions[i].rVal)) {
                            flag = true;
                        }
                        break;
                    case LTE_TYPE:
                        if (compare->lte(searchData, expressions[i].rVal)) {
                            flag = true;
                        }
                        break;
                    case LIKE_TYPE: {
                        if (expressions[i].rType != DB_CHAR) {
                            printf("[ERROR] right expression in LIKE must be VARCHAR.\n");
                            return -1;
                        }
                        string regStr = ((char*)expressions[i].rVal);
                        for (int i = 0; i < regStr.length(); i++) {
                            if (regStr[i] == '%' && i != 0 && i != regStr.length() - 1) {
                                printf("[ERROR] wrong format of LIKE string.\n");
                                return -1;
                            }
                        }
                        string::size_type pos(0); 
                        while ((pos = regStr.find("%")) != std::string::npos) {
                            regStr.replace(pos, 1, ".*");
                        }
                        if (std::regex_match((char*)searchData, std::regex(regStr))) {
                            flag = true;
                        }
                        break;
                    }
                    default:
                        return -1;
                        break;
                }
                if (flag) {
                    if (fileId == 0) {
                        res[cur].push_back(curRecordId);
                        if (tableNum > 1) {
                            res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        }
                    } else {
                        res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        res[cur].push_back(curRecordId);
                    }
                    preFlag = flag;
                }
            }
            preRecordDataNode = curRecordDataNode;
        }
    }
    for (int i = 0; i < res[cur].size(); i++) {
        RecordId* resRecordId = new RecordId(res[cur][i]->getPageId(), res[cur][i]->getSlotId());
        resRecordIds.push_back(resRecordId);
    }
    for (int i = 0; i < MAX_SELECT_TABLE; i++) {
        for (int j = 0; j < records[i].size(); j++) {
            delete records[i][j];
            delete recordIds[i][j];
        }
    }
    for (int i = 0; i < optimizeRecords.size(); i++) {
        delete optimizeRecords[i];
        delete optimizeRecordIds[i];
    }
    for (int i = 0; i < opRecordIds.size(); i++) {
        delete opRecordIds[i];
    }
    return 0;
}

inline void operateData(RecordData& lRecordData, RecordData& rRecordData, DB_SELECT_TYPE op, int colId, int recordColId) {
    RecordDataNode* lHead = lRecordData.head;
    RecordDataNode* rHead = rRecordData.head;
    while (colId > 0 && lHead) {
        lHead = lHead->next;
        colId--;
    }
    while (recordColId > 0 && rHead) {
        rHead = rHead->next;
        recordColId--;
    }
    if (colId > 0 || recordColId > 0 || !lHead || !rHead) {
        fprintf(stderr, "error in operateData\n");
        return;
    }
    bool initialized = true;
    lHead->nodeType = (op == COUNT_TYPE) ? COL_INT : (op == AVERAGE_TYPE ? COL_FLOAT : rHead->nodeType);
    lHead->len      = (op == COUNT_TYPE) ? 4 /* int */ : (op == AVERAGE_TYPE ? lHead->len : rHead->len);
    void* lData = transformType(lHead);
    void* rData = transformType(rHead);
    if (lData == nullptr) {
        initialized = false;
        if (op == AVERAGE_TYPE) {
            lHead->len = 0;  // here len means the number of data calculated
        }
    }
    Compare* compare = nullptr;
    switch (lHead->nodeType) {
        case COL_INT:
            compare = new IntCompare();
            if (!initialized) {
                lData = new int;
            }
            break;
        case COL_FLOAT:
            compare = new FloatCompare();
            if (!initialized) {
                lData = new float;
            }
            break;
        case COL_VARCHAR:
            compare = new CharCompare();
            if (!initialized) {
                lData = new char[lHead->len];
            }
            break;
        default:
            break;
    }
    int calculateType = (rHead->nodeType == COL_INT) ? 1 : 0;
    switch (op) {
        case ORD_TYPE: // copy
            memcpy(lData, rData, rHead->len);
            break;
        case MAX_TYPE:
            if (!initialized || compare->gt(rData, lData)) {
                memcpy(lData, rData, rHead->len);
            }
            break;
        case MIN_TYPE:
            if (!initialized || compare->lt(rData, lData)) {
                memcpy(lData, rData, rHead->len);
            }
            break;
        case SUM_TYPE:
            if (!initialized) {
                memcpy(lData, rData, rHead->len);
            } else {
                if (calculateType) {
                    *(int*)lData += *(int*)rData;
                } else {
                    *(float*)lData += *(float*)rData;
                }
            }
            break;
        case COUNT_TYPE:
            if (!initialized) {
                *(int*)lData = 0;
            }
            *(int*)lData += 1;
            break;
        case AVERAGE_TYPE:
            if (!initialized) {
                lHead->len = 0;
                *(float*)lData = 0;
                memcpy(lData, rData, rHead->len);
            }
            if (calculateType) {
                *(float*)lData = (*(float*)lData * lHead->len + *(int*)rData) / (lHead->len + 1);
            } else {
                *(float*)lData = (*(float*)lData * lHead->len + *(float*)rData) / (lHead->len + 1);
            }
            lHead->len = lHead->len + 1;
            break;
    }
    writeBackData(lData, lHead);
}

int TableManager::_selectRecords(DBSelect* dbSelect, vector<RecordData>& resRecords, vector<string>& entryNames) {

    int tableSize = dbSelect->selectTables.size();
    if (tableSize > MAX_SELECT_TABLE) {
        printf("[ERROR] join selection for more than two tables is not supported.\n");
        return -1;
    }
    string fisrtTable = dbSelect->expressions.size() > 0 ? ((DBExpItem*)dbSelect->expressions[0].lVal)->expTable : "";
    if (dbSelect->expressions.size() > 0 && tableSize > 1 && fisrtTable != "" && dbSelect->selectTables[0] != fisrtTable) {
        std::swap(dbSelect->selectTables[0], dbSelect->selectTables[1]);
    }
    // fprintf(stderr, "%s %s\n", dbSelect->selectTables[0].c_str(), dbSelect->selectTables[1].c_str());
    FileHandler* fileHandlers[MAX_SELECT_TABLE];
    TableHeader* tableHeaders[MAX_SELECT_TABLE];
    for (int i = 0; i < tableSize; i++) {
        fileHandlers[i] = recordManager->findTable(dbSelect->selectTables[i].c_str());
        if (fileHandlers[i] == nullptr) {
            printf("[Error] table %s does not exit.\n", dbSelect->selectTables[i].c_str());
            return -1;
        }
        tableHeaders[i] = fileHandlers[i]->getTableHeader();
    }

    /* check format */
    vector<DBExpItem*> waitChecked;
    for (int i = 0; i < dbSelect->selectItems.size(); i++) {
        if (!dbSelect->selectItems[i].star) {
            waitChecked.push_back(&dbSelect->selectItems[i].item);
        }
    }
    for (int i = 0; i < dbSelect->expressions.size(); i++) {
        if (dbSelect->expressions[i].lType == DB_ITEM) {
            waitChecked.push_back(((DBExpItem*)dbSelect->expressions[i].lVal));
        }
        if (dbSelect->expressions[i].rType == DB_ITEM) {
            waitChecked.push_back(((DBExpItem*)dbSelect->expressions[i].rVal));
        }
    }
    if (dbSelect->groupByEn) {
        waitChecked.push_back(&dbSelect->groupByCol);
    }
    if (_checkFormat(fileHandlers, tableHeaders, dbSelect->selectTables, waitChecked) == -1) {
        return -1;
    }

    /* check group and aggregate selection*/
    /* group by: https://stackoverflow.com/questions/11991079/select-a-column-in-sql-not-in-group-by */
    if (dbSelect->groupByEn) {
        for (int i = 0; i < dbSelect->selectItems.size(); i++) {
            DBSelItem tempSelItem = dbSelect->selectItems[i];
            if (tempSelItem.selectType == ORD_TYPE) {
                if (tempSelItem.item.expTable != dbSelect->groupByCol.expTable || tempSelItem.item.expCol != dbSelect->groupByCol.expCol) {
                    printf("[ERROR] cannot select %s.%s when group by %s.%s\n", tempSelItem.item.expTable.c_str(), tempSelItem.item.expCol.c_str(), dbSelect->groupByCol.expTable.c_str(), dbSelect->groupByCol.expCol.c_str());
                    return -1;
                }
            }
        }
    }
    for (int i = 0; i < dbSelect->selectItems.size(); i++) {
        DBSelItem tempSelItem = dbSelect->selectItems[i];
        if (dbSelect->selectItems[i].selectType == ORD_TYPE || dbSelect->selectItems[i].star) {
            continue;
        }
        int j;
        for (j = 0; j < dbSelect->selectTables.size(); j++) {
            if (strcmp(tempSelItem.item.expTable.c_str(), dbSelect->selectTables[j].c_str()) == 0) {
                break;
            }
        }
        int index = checkColExist(tableHeaders[j], tempSelItem.item.expCol.c_str());
        /**
         *  Attention: this cannot be wrote as: TableEntryDescNode* tempTableEntryDescNode = tableHeaders[j]->getTableEntryDesc().getCol(index);
         *  because the intermediate variable tableHeaders[j]->getTableEntryDesc() will be destroyed, together with tempTableEntryDescNoe
         */
        TableEntryDesc tableEntryDesc = tableHeaders[j]->getTableEntryDesc();
        TableEntryDescNode* tempTableEntryDescNode = tableEntryDesc.getCol(index);
        if (tempTableEntryDescNode->colType == COL_NULL || tempTableEntryDescNode->colType == COL_VARCHAR) {
            printf("[ERROR] cannot conduct aggregate selection on column with NULL type of VARCHAR type.\n");
            return -1;
        }
    }
    if (!dbSelect->limitEn && dbSelect->offsetEn) {
        fprintf(stderr, "can not have offset without limit.\n");
        return -1;
    }
    if ((dbSelect->limitEn && dbSelect->limitNum < 0) || (dbSelect->offsetEn && dbSelect->offsetNum < 0)) {
        printf("[ERROR] invalid value of limitNum %d or offsetNum %d.\n", dbSelect->limitNum, dbSelect->offsetNum);
        return -1;
    }
    vector<RecordId*> resRecordIds;
    if (_iterateWhere(dbSelect->selectTables, dbSelect->expressions, resRecordIds) == -1) {
        return -1;
    }
    // build entryNames
    bool hasAggregate = false;
    vector<int> colIds[MAX_SELECT_TABLE];
    vector<int> recordColIds[MAX_SELECT_TABLE];
    vector<DB_SELECT_TYPE> ops[MAX_SELECT_TABLE];
    for (int i = 0; i < dbSelect->selectItems.size(); i++) {
        if (dbSelect->selectItems[i].selectType != ORD_TYPE) {
            hasAggregate = true;
        }
        char colName[TAB_MAX_NAME_LEN];
        if (dbSelect->selectItems[i].star && dbSelect->selectItems[i].selectType != ORD_TYPE && dbSelect->selectItems[i].selectType != COUNT_TYPE) {
            fprintf(stderr, "* type only support ORD and COUNT.\n");
            return -1;
        }
        if (dbSelect->selectItems[i].selectType == ORD_TYPE && dbSelect->selectItems[i].star) {
            if (i > 0 || dbSelect->selectItems.size() != 1) {
                fprintf(stderr, "can only have one item *.\n");
                return -1;
            }
            int offset = 0;
            for (int j = 0; j < tableSize; j++) {
                for (int k = 0; k < tableHeaders[j]->colNum; k++) {
                    tableHeaders[j]->getCol(k, colName);
                    entryNames.push_back((string)tableHeaders[j]->tableName + "." + (string)colName);
                    colIds[j].push_back(offset + k);
                    recordColIds[j].push_back(k);
                    ops[j].push_back(ORD_TYPE);
                }
                offset += tableHeaders[j]->colNum;
            }
            break;
        }
        string tempEntryName = dbSelect->selectItems[i].star ? "*" : (dbSelect->selectItems[i].item.expTable + "." + dbSelect->selectItems[i].item.expCol);
        switch (dbSelect->selectItems[i].selectType) {
            case (MAX_TYPE):
                tempEntryName = "MAX(" + tempEntryName + ")";
                break;
            case (MIN_TYPE):
                tempEntryName = "MIN(" + tempEntryName + ")";
                break;
            case (SUM_TYPE):
                tempEntryName = "SUM(" + tempEntryName + ")";
                break;
            case (COUNT_TYPE):
                tempEntryName = "COUNT(" + tempEntryName + ")";
                break;
            case (AVERAGE_TYPE):
                tempEntryName = "AVG(" + tempEntryName + ")";
                break;
            default:
                break;
        }
        entryNames.push_back(tempEntryName);
        int fileId = dbSelect->selectItems[i].star ? 0 : ((dbSelect->selectTables[0] == dbSelect->selectItems[i].item.expTable) ? 0 : 1);
        int recordColId  = dbSelect->selectItems[i].star ? -1 : (tableHeaders[fileId]->getCol(dbSelect->selectItems[i].item.expCol.c_str()));
        int colId = i;
        colIds[fileId].push_back(colId);
        recordColIds[fileId].push_back(recordColId);
        ops[fileId].push_back(dbSelect->selectItems[i].selectType);
    }

    int selectNum = resRecordIds.size() / tableSize;
    int startNum = 0, endNum = selectNum;
    bool exceedFlag = false;
    if (dbSelect->limitEn) {
        if (!dbSelect->offsetEn) {
            dbSelect->offsetNum = 0;
        }
        startNum = dbSelect->offsetNum;
        endNum = startNum + dbSelect->limitNum;
        if (endNum > selectNum) {
            exceedFlag = true;
            endNum = selectNum;
        }
        selectNum = endNum - startNum;
    } else {
        dbSelect->offsetNum = 0; // default: offset = 0
    }

    /**
     * https://stackoverflow.com/questions/5920070/why-cant-you-mix-aggregate-values-and-non-aggregate-values-in-a-single-select
     * when use aggregation query on the whole set, it "collapses" the table to a single group, 
     * making it impossible to access the individual items within a group in a select clause.
     */
    resRecords.clear();
    if (dbSelect->groupByEn) {
        int groupFileId = dbSelect->groupByCol.expTable == dbSelect->selectTables[0] ? 0 : 1;
        int groupColId  = tableHeaders[groupFileId]->getCol(dbSelect->groupByCol.expCol.c_str());
        TableEntryDesc groupTableEntryDesc = tableHeaders[groupFileId]->getTableEntryDesc();
        TableEntryDescNode* groupTableEntryDescNode = groupTableEntryDesc.getCol(groupColId);
        int cnt = -1;
        void* ma = nullptr;
        switch (groupTableEntryDescNode->colType) { // build map
            case COL_INT:
                ma = new map<int, int>;
                break;
            case COL_FLOAT:
                ma = new map<float, int>;
                break;
            case COL_VARCHAR:
                ma = new map<string, int>;
                break;
            default:
                break;
        }
        for (int i = 0; i < resRecordIds.size(); i += tableSize) {
            RecordId* groupRecordId = resRecordIds[i + groupFileId];
            Record groupRecord(fileHandlers[groupFileId]->getRecordLen());
            fileHandlers[groupFileId]->getRecord(*groupRecordId, groupRecord);
            RecordData groupRecordData;
            groupRecord.deserialize(groupRecordData, groupTableEntryDesc);
            RecordDataNode* groupRecordDataNode = groupRecordData.getData(groupColId);
            int mapIndex = getMapIndex(groupRecordDataNode, ma, cnt) - dbSelect->offsetNum;
            if (dbSelect->limitEn && (mapIndex < 0 || (mapIndex >= dbSelect->limitNum))) {
                continue;
            }
            if (mapIndex >= resRecords.size()) {
                resRecords.push_back(RecordData(entryNames.size()));
            }
            for (int j = i; j < i + tableSize; j++) {
                int curFileId = j - i;
                RecordId* recordId = resRecordIds[j];
                Record tempRecord(fileHandlers[curFileId]->getRecordLen());
                fileHandlers[curFileId]->getRecord(*recordId, tempRecord);
                RecordData tempRecordData;
                TableEntryDesc tableEntryDesc = tableHeaders[curFileId]->getTableEntryDesc();
                tempRecord.deserialize(tempRecordData, tableEntryDesc);
                for (int k = 0; k < colIds[curFileId].size(); k++) {
                    operateData(resRecords[mapIndex], tempRecordData, ops[curFileId][k], colIds[curFileId][k], recordColIds[curFileId][k]);
                }
            }
        }
        switch (groupTableEntryDescNode->colType) { // build map
            case COL_INT: delete (map<int, int>*)ma; break;
            case COL_FLOAT: delete (map<float, float>*)ma; break;
            case COL_VARCHAR: delete (map<string, int>*)ma; break;
            default: break;
        }
    } else {
        if (hasAggregate) { // when there is aggregate query, there will be only one result record
            resRecords.push_back(RecordData(entryNames.size()));
        } else {
            for (int i = startNum; i < endNum; i++) {
                resRecords.push_back(RecordData(entryNames.size()));
            }
        }
        for (int i = startNum * tableSize; i < endNum * tableSize; i += tableSize) {
            int resRecordsIndex = (hasAggregate) ? 0 : ((i / tableSize) - startNum);
            for (int j = i; j < i + tableSize; j++) {
                int curFileId = j - i;
                RecordId* recordId = resRecordIds[j];
                Record tempRecord(fileHandlers[curFileId]->getRecordLen());
                fileHandlers[curFileId]->getRecord(*recordId, tempRecord);
                RecordData tempRecordData;
                TableEntryDesc tableEntryDesc = tableHeaders[curFileId]->getTableEntryDesc();
                tempRecord.deserialize(tempRecordData, tableEntryDesc);
                for (int k = 0; k < colIds[curFileId].size(); k++) {
                    operateData(resRecords[resRecordsIndex], tempRecordData, ops[curFileId][k], colIds[curFileId][k], recordColIds[curFileId][k]);
                }
            }
        }
        if (exceedFlag) {
            printf("[Warning] number of records starting at offset %d is less than %d.\n", dbSelect->offsetNum, dbSelect->limitNum);
        }
    }
    for (int i = 0; i < resRecordIds.size(); i++) {
        delete resRecordIds[i];
    }
    return resRecords.size();
}

int TableManager::selectRecords(DBSelect* dbSelect) {
    vector<RecordData> recordDatas;
    vector<string> colNames;
    int cnt = _selectRecords(dbSelect, recordDatas, colNames);
    if (cnt < 0) {
        return -1;
    }
    // print the records
    printf("\n----------------\n");
    for (int i = 0; i < colNames.size(); i++) {
        printf("%s ", colNames[i].c_str());
        if (i < colNames.size() - 1) {
            printf("| ");
        }
    }
    printf("\n----------------\n");
    for (int i = 0; i < recordDatas.size(); i++) {
        RecordDataNode* cur = recordDatas[i].head;
        while (cur) {
            switch (cur->nodeType) {
                case COL_INT:
                    printf("%d ", *(cur->content.intContent));
                    break;
                case COL_FLOAT:
                    printf("%f ", *(cur->content.floatContent));
                    break;
                case COL_VARCHAR:
                    printf("%s ", cur->content.charContent);
                    break;
                case COL_NULL:
                    if (dbSelect->selectItems[i].selectType == COUNT_TYPE) {
                        printf("0 ");
                    } else {
                        printf("NULL ");
                    }
                    break;
                default:
                    break;
            }
            if (cur->next) {
                printf("| ");
            }
            cur = cur->next;
        }
        printf("\n");
    }
    printf("----------------\n");
    return cnt;
}

int TableManager::insertRecords(string tableName, DBInsert* dbInsert, DBMeta* dbMeta) {
    if (dbInsert->valueLists.size() == 0) {
        return 0;
    }
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    FileHandler* insertFileHandler = recordManager->findTable(tableName.c_str());
    TableHeader* tableHeader = insertFileHandler->getTableHeader();
    TableEntryDesc tableEntryDesc = tableHeader->getTableEntryDesc();
    int valueSize = dbInsert->valueLists[0].size();
    if (valueSize != tableHeader->colNum) {
        printf("[ERROR] column size does not match!\n");
        return -1;
    }
    vector<Record*> records;
    vector<vector<void*>> indexData;
    indexData.resize(valueSize);
    // records.resize(dbInsert->valueLists.size());
    for (int i = 0; i < dbInsert->valueLists.size(); i++) {
        records.push_back(new Record(insertFileHandler->getRecordLen()));
    }
    for (int i = 0; i < dbInsert->valueLists.size(); i++) {
        RecordData recordData(valueSize);
        RecordDataNode* recordDataNode = recordData.head;
        for (int j = 0; j < valueSize; j++) {
            TableEntryDescNode* tableEntryDescNode = tableEntryDesc.getCol(j);
            if (dbInsert->valueListsType[i][j] != tableEntryDescNode->colType) {
                printf("[Error] column type done't match. Types are %d and %d\n", dbInsert->valueListsType[i][j], tableEntryDesc.getCol(j)->colType);
                return -1;
            }
            int sLen = -1;
            switch (dbInsert->valueListsType[i][j]) {
                case DB_LIST_INT:
                    recordDataNode->len = 4;
                    recordDataNode->nodeType = COL_INT;
                    recordDataNode->content.intContent = new int;
                    *recordDataNode->content.intContent = *(int*)dbInsert->valueLists[i][j];
                    indexData[j].push_back((int*)dbInsert->valueLists[i][j]);
                    break;
                case DB_LIST_FLOAT:
                    recordDataNode->len = 4;
                    recordDataNode->nodeType = COL_FLOAT;
                    recordDataNode->content.floatContent = new float;
                    *recordDataNode->content.floatContent = *(float*)dbInsert->valueLists[i][j];
                    indexData[j].push_back((float*)dbInsert->valueLists[i][j]);
                    break;
                case DB_LIST_CHAR:
                    recordDataNode->len = tableEntryDescNode->colLen;
                    recordDataNode->nodeType = COL_VARCHAR;
                    recordDataNode->content.charContent = new char[recordDataNode->len];
                    sLen = strlen((char*)dbInsert->valueLists[i][j]);
                    memcpy(recordDataNode->content.charContent, (char*)dbInsert->valueLists[i][j], sLen);
                    if (recordDataNode->len - 1 < sLen) {
                        sLen = recordDataNode->len - 1;
                    } 
                    recordDataNode->content.charContent[sLen] = '\0';
                    indexData[j].push_back((char*)dbInsert->valueLists[i][j]);
                    break;
                case DB_LIST_NULL:
                    recordDataNode->len = 0;
                    recordDataNode->nodeType = COL_NULL;
                    indexData[j].push_back(nullptr);
                    fprintf(stderr, "Null type is not supported currently.\n");
                    return -1;
                    break;
            }
            recordDataNode = recordDataNode->next;
        }
        recordData.serialize(*(records[i]));
    }

    char colName[64];
    vector<RecordId> recordIds;
    vector<bool> colHasIndex;
    bool errorFlag = false;
    int insertedIndex = -1;
    if (insertFileHandler->insertAllRecords(records, recordIds)) { // succeed
        // insert indexes
        for (int i = 0; i < tableHeader->colNum; i++) {
            tableHeader->getCol(i, colName);
            colHasIndex.push_back(indexManager->hasIndex(tableName.c_str(), colName));
        }

        for (int k = 0; k < recordIds.size(); k++) {
            RecordData insertRecordData;
            records[k]->deserialize(insertRecordData, tableEntryDesc);
            if (!_checkConstraintOnInsert(tableName, &insertRecordData, dbMeta)) {
                insertedIndex = k;
                errorFlag = true;
                break;
            }
            for (int i = 0; i < tableHeader->colNum; i++) {
                tableHeader->getCol(i, colName);
                if (colHasIndex[i]) {
                    int val;
                    insertFileHandler->transform(val, recordIds[k].getPageId(), recordIds[k].getSlotId());
                    indexManager->insert(tableName.c_str(), colName, indexData[i][k], val);
                }
            }
        }
        if (!errorFlag) {
            return dbInsert->valueLists.size();
        }
    }
    if (errorFlag) {  // restore all inserted data and indexes
        Record tempRecord(insertFileHandler->getRecordLen());
        for (int i = 0; i < recordIds.size(); i++) {
            insertFileHandler->removeRecord(recordIds[i], tempRecord);
        }
        for (int k = 0; k < insertedIndex; k++) {
            RecordData insertRecordData;
            records[k]->deserialize(insertRecordData, tableEntryDesc);
            for (int i = 0; i < tableHeader->colNum; i++) {
                tableHeader->getCol(i, colName);
                if (colHasIndex[i]) {
                    int val = -1;
                    insertFileHandler->transform(val, recordIds[k].getPageId(), recordIds[k].getSlotId());
                    indexManager->remove(tableName.c_str(), colName, indexData[i][k], val);
                }
            }
        }
    }
    return -1; // failure. Note: when fail, no record will be inserted
}

int TableManager::dropRecords(string tableName, DBDelete* dbDelete, DBMeta* dbMeta) {
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    FileHandler* deleteFileHandler = recordManager->findTable(tableName.c_str());
    TableHeader* tableHeader = deleteFileHandler->getTableHeader();
    vector<string> selectTables = {tableName};
    vector<RecordId*> recordIds;
    vector<Record*> removedRecords;
    TableEntryDesc tableEntryDesc = tableHeader->getTableEntryDesc();

    vector<DBExpItem*> waitChecked;
    for (int i = 0; i < dbDelete->expression.size(); i++) {
        if (dbDelete->expression[i].lType == DB_ITEM) {
            waitChecked.push_back(((DBExpItem*)dbDelete->expression[i].lVal));
        }
        if (dbDelete->expression[i].rType == DB_ITEM) {
            waitChecked.push_back(((DBExpItem*)dbDelete->expression[i].rVal));
        }
    }
    if (_checkFormat(&deleteFileHandler, &tableHeader, selectTables, waitChecked) == -1) {
        return -1;
    }
    if (_iterateWhere(selectTables, dbDelete->expression, recordIds) == -1) {
        return -1;
    }

    bool colHasIndex[TAB_MAX_COL_NUM] = {false};
    char colName[TAB_MAX_COL_NUM][64];
    vector<uint8_t> colTypes;
    for (int i = 0; i < tableHeader->colNum; i++) {
        tableHeader->getCol(i, colName[i]);
        colTypes.push_back(tableHeader->getTableEntryDesc().getCol(i)->colType);
        if (indexManager->hasIndex(tableName.c_str(), colName[i])) {
            colHasIndex[i] = true;
        }
    }

    vector<vector<void*>> indexDatas;
    indexDatas.resize(tableHeader->colNum);
    bool errorFlag = false;

    for (int i = 0; i < recordIds.size(); i++) {
        Record* removedRecord = new Record(deleteFileHandler->getRecordLen());
        deleteFileHandler->getRecord(*recordIds[i], *removedRecord);
        RecordData recordData;
        removedRecord->deserialize(recordData, tableEntryDesc);
        if (!_checkConstraintOnDelete(tableName, &recordData, dbMeta)) {
            errorFlag = true;
            break;
        }
        removedRecords.push_back(removedRecord);
        if (!deleteFileHandler->removeRecord(*recordIds[i], *removedRecords[i])) {
            delete removedRecords[removedRecords.size() - 1];
            removedRecords.pop_back();
            errorFlag = true;
            break;
        }
        RecordDataNode* recordDataNode = recordData.head;
        for (int j = 0; j < tableHeader->colNum; j++) {
            void* indexData = nullptr;
            switch (recordDataNode->nodeType) {
                case COL_INT: 
                    indexData = new int;
                    *(int*)indexData = *recordDataNode->content.intContent; 
                    break;
                case COL_FLOAT: 
                    indexData = new float;
                    *(float*)indexData = *recordDataNode->content.floatContent;
                    indexData = recordDataNode->content.floatContent;
                    break;
                case COL_VARCHAR: 
                    indexData = new char[recordDataNode->len];
                    memcpy(indexData, recordDataNode->content.charContent, recordDataNode->len);
                    break;
                default: 
                    fprintf(stderr, "Null type is not supported currently.\n");
                    return -1;
                    break;
            }
            if (colHasIndex[j]) {
                int val;
                deleteFileHandler->transform(val, recordIds[i]->getPageId(), recordIds[i]->getSlotId());
                indexDatas[j].push_back(indexData);
                indexManager->remove(tableName.c_str(), colName[j], indexData, val);
            }
            recordDataNode = recordDataNode->next;
        }
    }
    if (errorFlag) { // encouter error when removing records
        vector<RecordId> recordIds;
        recordIds.resize(removedRecords.size());
        deleteFileHandler->insertAllRecords(removedRecords, recordIds); // memcpy method, so Record newed can be safely deleted
        // note: although the records are inserted back, its position may not be the same as before
        
        //restore index
        vector<int> pageIds, slotIds;
        for (RecordId recordId : recordIds) {
            pageIds.push_back(recordId.getPageId());
            slotIds.push_back(recordId.getSlotId());
        }
        for (int i = 0; i < tableHeader->colNum; i++) {
            if (colHasIndex[i]) {
                vector<int> vals;
                deleteFileHandler->transform(vals, pageIds, slotIds);
                indexManager->insert(tableName.c_str(), colName[i], indexDatas[i], vals);
            }
        }
    }
    for (int i = 0; i < removedRecords.size(); i++) {
        delete removedRecords[i];
    }
    for (int i = 0; i < recordIds.size(); i++) {
        delete recordIds[i];
    }
    for (int i = 0; i < tableHeader->colNum; i++) {
        for (int j = 0; j < indexDatas[i].size(); j++) {
            switch (colTypes[i]) {
                case COL_INT: delete (int*)indexDatas[i][j]; break;
                case COL_FLOAT: delete (float*)indexDatas[i][j]; break;
                case COL_VARCHAR: delete (char*)indexDatas[i][j]; break;
                default: break;
            }
        }
    }
    return errorFlag ? -1 : recordIds.size();
}

int TableManager::updateRecords(string tableName, DBUpdate* dbUpdate, DBMeta* dbMeta) {
    string path = "database/" + databaseName + '/' + tableName +".db";
    fprintf(stderr, "path = %s, %d\n", path.c_str(), checkTableExist(path));
    if(!checkTableExist(path)) {
        printf("[ERROR] table dose not exist!\n");
        return -1;
    }
    FileHandler* updateFileHandler = recordManager->findTable(tableName.c_str());
    TableHeader* tableHeader = updateFileHandler->getTableHeader();
    TableEntryDesc tableEntryDesc = tableHeader->getTableEntryDesc();
    vector<string> selectTables = {tableName};
    vector<RecordId*> recordIds;
    vector<Record*> rawRecords;
    vector<int> colIds;

    vector<DBExpItem*> waitChecked;
    for (int i = 0; i < dbUpdate->expItem.size(); i++) {
        int colId = tableHeader->getCol(((DBExpItem*)(dbUpdate->expItem[i].lVal))->expCol.c_str());
        for (int j = 0; j < colIds.size(); j++) {
            if (colId == colIds[j]) {
                printf("[ERROR] can not set a column twice in an update statement.\n");
                return -1;
            }
        }
        colIds.push_back(colId);
        TableEntryDescNode* tableEntryDescNode = tableEntryDesc.getCol(colId);
        if (tableEntryDescNode->colType != dbUpdate->expItem[i].rType) {
            printf("[ERROR] incompatible type %d and %d in update set clause.\n", dbUpdate->expItem[i].rType, tableEntryDescNode->colType);
            return -1;
        }
        waitChecked.push_back(((DBExpItem*)dbUpdate->expItem[i].lVal));
    }
    for (int i = 0; i < dbUpdate->expressions.size(); i++) {
        if (dbUpdate->expressions[i].lType == DB_ITEM) {
            waitChecked.push_back(((DBExpItem*)dbUpdate->expressions[i].lVal));
        }
        if (dbUpdate->expressions[i].rType == DB_ITEM) {
            waitChecked.push_back(((DBExpItem*)dbUpdate->expressions[i].rVal));
        }
    }
    if (_checkFormat(&updateFileHandler, &tableHeader, selectTables, waitChecked) == -1) {
        return -1;
    }
    
    if (_iterateWhere(selectTables, dbUpdate->expressions, recordIds) == -1) {
        return -1;
    }

    bool colHasIndex[TAB_MAX_COL_NUM] = {false};
    char colName[TAB_MAX_COL_NUM][64];
    vector<uint8_t> colTypes;
    for (int i = 0; i < tableHeader->colNum; i++) {
        tableHeader->getCol(i, colName[i]);
        colTypes.push_back(tableHeader->getTableEntryDesc().getCol(i)->colType);
        if (indexManager->hasIndex(tableName.c_str(), colName[i])) {
            colHasIndex[i] = true;
        }
    }
    vector<vector<void*>> updatedIndexDatas;
    vector<vector<void*>> rawIndexDatas;
    updatedIndexDatas.resize(tableHeader->colNum);
    rawIndexDatas.resize(tableHeader->colNum);
    bool errorFlag = false;

    for (int i = 0; i < recordIds.size(); i++) {
        rawRecords.push_back(new Record(updateFileHandler->getRecordLen()));
        updateFileHandler->getRecord(*recordIds[i], *rawRecords[i]);
        RecordData updatedRecordData, rawRecordData;
        rawRecords[i]->deserialize(updatedRecordData, tableEntryDesc);
        rawRecords[i]->deserialize(rawRecordData, tableEntryDesc);

        for (int j = 0; j < dbUpdate->expItem.size(); j++) {
            RecordDataNode* recordDataNode = updatedRecordData.getData(colIds[j]);
            TableEntryDescNode* tableEntryDescNode = tableEntryDesc.getCol(j);
            int sLen = -1;
            switch (dbUpdate->expItem[j].rType) {
                case COL_INT:
                    if (recordDataNode->nodeType == COL_NULL) {
                        recordDataNode->len = 4;
                        recordDataNode->content.intContent = new int;
                    }
                    *recordDataNode->content.intContent = *(int*)dbUpdate->expItem[j].rVal;
                    break;
                case COL_FLOAT:
                    if (recordDataNode->nodeType == COL_NULL) {
                        recordDataNode->len = 4;
                        recordDataNode->content.floatContent = new float;
                    }
                    *recordDataNode->content.floatContent = *(float*)dbUpdate->expItem[j].rVal;
                    break;
                case COL_VARCHAR:
                    if (recordDataNode->nodeType == COL_NULL) {
                        recordDataNode->len = tableEntryDescNode->colLen;
                        recordDataNode->content.charContent = new char[recordDataNode->len];
                    }
                    sLen = strlen((char*)dbUpdate->expItem[j].rVal);
                    memcpy(recordDataNode->content.charContent, (char*)dbUpdate->expItem[j].rVal, sLen);
                    if (recordDataNode->len - 1 < sLen) {
                        sLen = recordDataNode->len - 1;
                    }
                    recordDataNode->content.charContent[sLen] = '\0';
                    break;
                default:
                    break;
            }
            recordDataNode->nodeType = (TB_COL_TYPE)dbUpdate->expItem[j].rType;
        }

        if (!_checkConstraintOnUpdate(tableName, &rawRecordData, dbMeta, dbUpdate->expItem, colIds)) {
            errorFlag = true;
            break;
        }

        Record updatedRecord(updateFileHandler->getRecordLen());
        updatedRecordData.serialize(updatedRecord);
        if (updateFileHandler->updateRecord(*recordIds[i], updatedRecord)) {
            RecordData rawRecordData;
            rawRecords[i]->deserialize(rawRecordData, tableEntryDesc);
            RecordDataNode *updatedRecordDataNode = updatedRecordData.head, *rawRecordDataNode = rawRecordData.head;
            for (int j = 0; j < tableHeader->colNum; j++) {
                void* updatedIndexData = nullptr;
                void* rawIndexData = nullptr;
                switch (rawRecordDataNode->nodeType) {
                    case COL_INT:
                        updatedIndexData = new int;
                        *(int*)updatedIndexData = *updatedRecordDataNode->content.intContent;
                        rawIndexData = new int;
                        *(int*)rawIndexData = *rawRecordDataNode->content.intContent;
                        break;
                    case COL_FLOAT:
                        updatedIndexData = new float;
                        *(float*)updatedIndexData = *updatedRecordDataNode->content.floatContent;
                        rawIndexData = new float;
                        *(float*)rawIndexData = *rawRecordDataNode->content.floatContent;
                        break;
                    case COL_VARCHAR:
                        updatedIndexData = new char[updatedRecordDataNode->len];
                        memcpy(updatedIndexData, updatedRecordDataNode->content.charContent, updatedRecordDataNode->len);
                        rawIndexData = new char[rawRecordDataNode->len];
                        memcpy(rawIndexData, rawRecordDataNode->content.charContent, rawRecordDataNode->len);
                        break;
                    default:
                        fprintf(stderr, "Null type is not supported currently.\n");
                        return -1;
                        break;
                }
                bool colHasChanged = false;
                for (int k = 0; k < colIds.size(); k++) {
                    if (j == colIds[k]) {
                        colHasChanged = true;
                        break;
                    }
                }
                if (colHasIndex[j] && colHasChanged) {
                    int val;
                    updateFileHandler->transform(val, recordIds[i]->getPageId(), recordIds[i]->getSlotId());
                    updatedIndexDatas[j].push_back(updatedIndexData);
                    rawIndexDatas[j].push_back(rawIndexData);
                    indexManager->remove(tableName.c_str(), colName[j], rawIndexData, val);
                    indexManager->insert(tableName.c_str(), colName[j], updatedIndexData, val);
                }
            }
        } else {
            errorFlag = true;
            break;
        }
    }

    if (errorFlag) { // restore
        for (int i = 0; i < rawRecords.size() - 1; i++) {
            updateFileHandler->updateRecord(*recordIds[i], *rawRecords[i]);
        }
        //restore index
        vector<int> pageIds, slotIds;
        for (RecordId* recordId : recordIds) {
            pageIds.push_back(recordId->getPageId());
            slotIds.push_back(recordId->getSlotId());
        }
        for (int i = 0; i < tableHeader->colNum; i++) {
            if (colHasIndex[i]) {
                vector<int> vals;
                updateFileHandler->transform(vals, pageIds, slotIds);
                for (int j = 0; j < updatedIndexDatas[i].size(); j++) {
                    indexManager->remove(tableName.c_str(), colName[i], updatedIndexDatas[i][j], vals[j]);
                    indexManager->insert(tableName.c_str(), colName[i], rawIndexDatas[i][j], vals[j]);
                }
            }
        }
    }
    for (int i = 0; i < rawRecords.size(); i++) {
        delete rawRecords[i];
    }
    for (int i = 0; i < recordIds.size(); i++) {
        delete recordIds[i];
    }

    for (int i = 0; i < tableHeader->colNum; i++) {
        for (int j = 0; j < updatedIndexDatas[i].size(); j++) {
            switch (colTypes[i]) {
                case COL_INT: delete (int*)(updatedIndexDatas[i][j]); delete (int*)(rawIndexDatas[i][j]); break;
                case COL_FLOAT: delete (float*)(updatedIndexDatas[i][j]); delete (float*)(rawIndexDatas[i][j]); break;
                case COL_VARCHAR: delete (char*)(updatedIndexDatas[i][j]); delete (char*)(rawIndexDatas[i][j]); break;
                default: break;
            }
        }
    }

    return errorFlag ? -1 : recordIds.size();
}

bool TableManager::_checkConstraintOnInsert(string tableName, RecordData* recordData, DBMeta* dbMeta, vector<int> noCheckOnColIds) {
    int tableNum = -1;
    for (int i = 0; i < dbMeta->tableNum; i++) {
        if (!strcmp(tableName.c_str(), dbMeta->tableNames[i])) {
            tableNum = i;
            break;
        }
    }
    if (tableNum == -1) {
        return false;
    }

    FileHandler* checkFileHandler = recordManager->findTable(tableName.c_str());
    TableHeader* tableHeader = checkFileHandler->getTableHeader();
    char colName[64];
    for (int i = 0; i < dbMeta->colNum[tableNum]; i++) {
        bool noCheck = false;
        for (int j = 0; j < noCheckOnColIds.size(); j++) {
            if (i == noCheckOnColIds[j]) {
                noCheck = true;
                break;
            }
        }
        if (noCheck) {
            continue;
        }
        if (dbMeta->isPrimaryKey[tableNum][i] || dbMeta->isUniqueKey[tableNum][i]) {
            RecordDataNode* recordDataNode = recordData->getData(i);
            void* checkData = nullptr;
            switch (recordDataNode->nodeType) {
                case COL_INT: checkData = recordDataNode->content.intContent; break;
                case COL_FLOAT: checkData = recordDataNode->content.floatContent; break;
                case COL_VARCHAR: checkData = recordDataNode->content.charContent; break;
                default: break;
            }
            if (checkData == nullptr) {
                return false; // note: encounter null value
            }
            
            tableHeader->getCol(i, colName);
            vector<int> res;
            if (!indexManager->hasIndex(tableName.c_str(), colName)) {
                fprintf(stderr, "no index on primary key or unique key.\n");
                return false;
            }
            indexManager->search(tableName.c_str(), colName, checkData, res);
            if (res.size() > 0) {
                printf("[ERROR] fail in checking primary or unique constraint.\n");
                return false;
            }
        }
    }
    for (int i = 0; i < dbMeta->foreignKeyNum[tableNum]; i++) {
        int col = dbMeta->foreignKeyColumn[tableNum][i];
        bool noCheck = false;
        for (int j = 0; j < noCheckOnColIds.size(); j++) {
            if (col == noCheckOnColIds[j]) {
                noCheck = true;
                break;
            }
        }
        if (noCheck) {
            continue;
        }
        int refTable = dbMeta->foreignKeyRefTable[tableNum][i];
        int refCol = dbMeta->foreignKeyRefColumn[tableNum][i];
        string refTableName = dbMeta->tableNames[refTable];
        FileHandler* foreignFileHandler = recordManager->findTable(refTableName.c_str());
        foreignFileHandler->getTableHeader()->getCol(refCol, colName);
        if (!indexManager->hasIndex(refTableName.c_str(), colName)) {
            fprintf(stderr, "%s.%s\n", tableName.c_str(), colName);
            fprintf(stderr, "no index on foreign key when insert.\n");
            return false;
        }
        vector<int> res;
        RecordDataNode* recordDataNode = recordData->getData(col);
        void* checkData = nullptr;
        switch (recordDataNode->nodeType) {
            case COL_INT: checkData = recordDataNode->content.intContent; break;
            case COL_FLOAT: checkData = recordDataNode->content.floatContent; break;
            case COL_VARCHAR: checkData = recordDataNode->content.charContent; break;
            default: break;
        }
        if (checkData == nullptr) {
            return false;
        }
        indexManager->search(refTableName.c_str(), colName, checkData, res);
        if (res.size() == 0) {
            printf("[ERROR] fail to insert/update due to confilct in foreign key constraint.\n");
            return false;
        }
    }
    return true;
}

bool TableManager::_checkConstraintOnDelete(string tableName, RecordData* recordData, DBMeta* dbMeta, vector<int> noCheckOnColIds) {
    int tableNum = -1;
    for (int i = 0; i < dbMeta->tableNum; i++) {
        if (!strcmp(tableName.c_str(), dbMeta->tableNames[i])) {
            tableNum = i;
            break;
        }
    }
    if (tableNum == -1) {
        return false;
    }
    FileHandler* checkFileHandler = recordManager->findTable(tableName.c_str());
    TableHeader* tableHeader = checkFileHandler->getTableHeader();
    char refColName[64], colName[64];
    for (int i = 0; i < dbMeta->refKeyNum[tableNum]; i++) {
        int col = dbMeta->refKeyColumn[tableNum][i];
        bool noCheck = false;
        for (int j = 0; j < noCheckOnColIds.size(); j++) {
            if (col == noCheckOnColIds[j]) {
                noCheck = true;
                break;
            }
        }
        if (noCheck) {
            continue;
        }
        int refTable = dbMeta->refKeyRefTable[tableNum][i];
        int refCol = dbMeta->refKeyRefColumn[tableNum][i];
        string refTableName = dbMeta->tableNames[refTable];
        FileHandler* foreignFileHandler = recordManager->findTable(refTableName.c_str());
        foreignFileHandler->getTableHeader()->getCol(refCol, refColName);
        tableHeader->getCol(col, colName);

        RecordDataNode* recordDataNode = recordData->getData(col);
        void* checkData = nullptr;
        switch (recordDataNode->nodeType) {
            case COL_INT: checkData = recordDataNode->content.intContent; break;
            case COL_FLOAT: checkData = recordDataNode->content.floatContent; break;
            case COL_VARCHAR: checkData = recordDataNode->content.charContent; break;
            default: break;
        }
        if (!indexManager->hasIndex(refTableName.c_str(), refColName)) {
            fprintf(stderr, "no index on foreign key when delete.\n");
            return false;
        }
        /* 
            should foreign key be unique?
            if not, then we should first check if there are duplicated values of the deleted column
         */
        if (!indexManager->hasIndex(tableName.c_str(), colName)) {
            fprintf(stderr, "no index on local key when delete.\n");
            return false;
        }
        vector<int> res;
        indexManager->search(tableName.c_str(), colName, checkData, res);
        if (res.size() <= 1) {
            indexManager->search(refTableName.c_str(), refColName, checkData, res);
            if (res.size() > 0) {
                printf("[ERROR] fail to delete/update due to conflict in foreign key constraint.\n");
                return false;
            }
        }
    }
    return true;
}

bool TableManager::_checkConstraintOnUpdate(string tableName, RecordData* recordData, DBMeta* dbMeta, vector<DBExpression> expItems, vector<int> colIds) {
    int tableNum = -1;
    for (int i = 0; i < dbMeta->tableNum; i++) {
        if (!strcmp(tableName.c_str(), dbMeta->tableNames[i])) {
            tableNum = i;
            break;
        }
    }
    if (tableNum == -1) {
        return false;
    }
    FileHandler* checkFileHandler = recordManager->findTable(tableName.c_str());
    TableHeader* tableHeader = checkFileHandler->getTableHeader();
    RecordData* newRecordData = new RecordData(tableHeader->colNum);
    vector<int> noCheckOnColIds;
    for (int i = 0; i < dbMeta->colNum[tableNum]; i++) {
        bool inColFlag = false;
        for (int j = 0; j < colIds.size(); j++) {
            if (i == colIds[j]) {
                inColFlag = true;
                RecordDataNode* recordDataNode = recordData->getData(i);
                RecordDataNode* newRecordDataNode = newRecordData->getData(i);
                newRecordDataNode->nodeType = recordDataNode->nodeType;
                newRecordDataNode->len = recordDataNode->len;
                Compare* compare;
                void* checkData = nullptr;
                switch (recordDataNode->nodeType) {
                    case COL_INT: 
                        checkData = recordDataNode->content.intContent; 
                        compare = new IntCompare(); 
                        newRecordDataNode->content.intContent = new int;
                        *newRecordDataNode->content.intContent = *(int*)expItems[j].rVal;
                        break;
                    case COL_FLOAT: 
                        checkData = recordDataNode->content.floatContent; 
                        compare = new FloatCompare(); 
                        newRecordDataNode->content.floatContent = new float;
                        *newRecordDataNode->content.floatContent = *(float*)expItems[j].rVal;
                        break;
                    case COL_VARCHAR:  
                        checkData = recordDataNode->content.charContent; 
                        compare = new CharCompare();
                        newRecordDataNode->content.charContent = new char[newRecordDataNode->len];
                        memcpy(newRecordDataNode->content.charContent, expItems[j].rVal, newRecordDataNode->len); 
                        break;
                    default: break;
                }
                if (compare->equ(checkData, expItems[j].rVal)) { // the new value to be set is the same as the old one, then this column need not to be checked
                    noCheckOnColIds.push_back(i);
                }
                break;
            }
        }
        if (!inColFlag) {
            noCheckOnColIds.push_back(i);
        }
    }
    bool resFlag = true;
    if (!_checkConstraintOnDelete(tableName, recordData, dbMeta, noCheckOnColIds) || !_checkConstraintOnInsert(tableName, newRecordData, dbMeta, noCheckOnColIds)) {
        resFlag = false;
    }
    delete newRecordData;
    return resFlag;
}