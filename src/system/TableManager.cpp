#include "TableManager.h"
#include "assert.h"
#include <unistd.h>

TableManager::TableManager(string databaseName_,  BufPageManager* bufPageManager_) {
    databaseName = databaseName_;
    recordManager = new RecordManager(bufPageManager_, databaseName.c_str());
    indexManager = new IndexManager(bufPageManager_, databaseName.c_str());
}

TableManager::~TableManager() {
    delete recordManager;
    delete indexManager;
    delete fileHandler;
}

int TableManager::checkColExist(TableHeader* tableHeader, const char* colName) {
    return tableHeader->getCol(colName);
}

inline bool TableManager::checkTableName(string name) {
    size_t length = name.length();
    if(length == 0 || length > TAB_MAX_NAME_LEN) {
        printf("[Error] invalid table name ! \n");
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
        printf("[Error] table named %s already exist!\n", tableName.c_str());
        return -1;
    }
    if(recordManager->createFile(tableName.c_str()) != 0){
        fprintf(stderr, "report error when create file in table manager\n");
        return -1;
    }
    if(recordManager->openFile(tableName.c_str(), fileHandler) != 0) {
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
        printf("[Error] table dose not exist!\n");
        return -1;
    }

    if(recordManager->openFile(name.c_str(), fileHandler) != 0) {
        printf("[Error] table %s has already been opened. \n", name.c_str());
        return -1;
    }

    return 0;
}

int TableManager::dropTable(string name) {
    if(!checkTableName(name))
        return -1;
    string path = "database/" + databaseName + '/' + name +".db";
    if(!checkTableExist(path)) {
        printf("[Error] table dose not exist!\n");
        return -1;
    }
        
    if(recordManager->removeFile(name.c_str()) != 0) {
        printf("[Error] fail to drop the table named %s\n", name.c_str());
        return -1;
    }

    return 0;

}

int TableManager::listTableInfo(string name) {
    if(!checkTableName(name))
        return -1;
    string path = "database/" + databaseName + '/' + name +".db";
    if(!checkTableExist(path))
        return -1;
    fileHandler = recordManager->findTable(name.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    printf("======================begin======================\n");
    for(int i = 0; i < tableHeader->colNum; i++) {
        printf("%64s|", tableHeader->entrys[i].colName);
    }
    printf("\n===============================================\n");
    for(int i = 0; i < tableHeader->colNum; i++) {
        switch (tableHeader->entrys[i].colType)
        {
        case 0:
            printf("NULL|");
            break;
        case 1:
            printf("INT|");
        case 2:
            printf("VARCHAR(%d)|", tableHeader->entrys[i].colLen);
        case 3:
            printf("FLOAT|");
        default:
            break;
        }
    }
    printf("\n=====================end======================\n");
    return 0;
}

int TableManager::renameTable(string oldName, string newName) {
    if(!checkTableName(newName))
        return -1;

    string oldPath = "database/" + databaseName + '/' + oldName +".db";
    if(!checkTableExist(oldPath)) {
        printf("[Error] table named %s does not exist !\n", oldName.c_str());
        return -1;
    }

    fileHandler = recordManager->findTable(oldName.c_str());
    if(fileHandler == nullptr) {
        printf("[Error] can not find the table named %s !\n", oldName.c_str());
        return -1;
    }
    if(recordManager->closeFile(fileHandler) != 0) {
        printf("[Error] can not close the file before rename it !\n");
        return-1;
    }
    fileHandler = nullptr;

    string newPath = "database/" + databaseName + '/' + newName +".db";
    int ret = rename(oldPath.c_str(), newPath.c_str());
    if(ret != 0) {
        printf("[Error] can not rename the table !\n");
        return -1;
    }
    
    if(recordManager->openFile(newName.c_str(), fileHandler) != 0) {
        printf("[Error] can not open the file after rename it !\n");
        return -1;
    }
    return 0;
}

int TableManager::saveChangeToFile(const char* tableName) {
    fileHandler = recordManager->findTable(tableName);
    recordManager->closeFile(fileHandler);
    // TODO
    return 0;
}

int TableManager::createIndex(string tableName, string colName) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[Error] table dose not exist!\n");
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
        printf("[Error] specified column does not exist.\n");
        return -1;
    }

    if (indexManager->hasIndex(tableName.c_str(), colName.c_str())) {
        printf("[Info] the index has already been created.\n");
        return index;
    }

    int res = indexManager->createIndex(tableName.c_str(), colName.c_str(), indexLen, colType);

    // add pre-existing records to the newly-added index
    if (res >= 0) {
        std::vector<Record*> records;
        std::vector<RecordId*> recordIds;
        fileHandler->getAllRecordsAccordingToFields(records, recordIds, (1 << index));
        std::vector<void*> insertDatas;
        std::vector<int> insertVals;
        std::vector<int> pageIds, slotIds;
        for (int i = 0; i < records.size(); i++) {
            insertDatas.push_back((void*)(records[i]->data));
            pageIds.push_back(recordIds[i]->getPageId());
            slotIds.push_back(recordIds[i]->getSlotId());
        }
        insertVals.resize(records.size());
        indexManager->transform(tableName.c_str(), colName.c_str(), insertVals, pageIds, slotIds);
        res = indexManager->insert(tableName.c_str(), colName.c_str(), insertDatas, insertVals);
    }
    return res;    
}

int TableManager::dropIndex(string tableName, string colName) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[Error] table dose not exist!\n");
        return -1;
    }
    // check column exists? maybe do not have to
    if (!indexManager->hasIndex(tableName.c_str(), colName.c_str())) {
        printf("[Info] No index on %s.%s\n", tableName.c_str(), colName.c_str());
        return 0;
    }

    return indexManager->removeIndex(tableName.c_str(), colName.c_str());
}

bool TableManager::hasIndex(string tableName, string colName) {
    return indexManager->hasIndex(tableName.c_str(), colName.c_str());
}

int TableManager::showIndex() {
    int cnt = indexManager->showIndex();
    printf("%d indexes in total.\n", cnt);
    return 0;
}

int TableManager::createPrimaryKey(string tableName, string colName) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[Error] table dose not exist!\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int index = checkColExist(tableHeader, colName.c_str());

    // make sure there is only one primary key
    if (tableHeader->hasPrimaryKey()) {
        printf("[Error] can not add primary key to this table.\n");
        return -1;
    }

    uint16_t indexLen;
    uint8_t colType;
    if (index >= 0) {
        tableHeader->entrys[index].primaryKeyConstraint = true;
        tableHeader->entrys[index].uniqueConstraint = false; // make sure primaryKeyConstraint and uniqueConstrain don't co-exist
        indexLen = tableHeader->entrys[index].colLen;
        colType = tableHeader->entrys[index].colType;
    } else {
        printf("[ERROR] specified column dose not exit.\n");
        return -1;
    }
    if (indexManager->hasIndex(tableName.c_str(), colName.c_str())) {
        printf("[Info] the index has already been created.\n");
        return index;
    }
    indexManager->createIndex(tableName.c_str(), colName.c_str(), indexLen, colType);
    // add pre-existing records to the newly-added index
    // however when primary key is enabled as the table is created
    // there should be no record to add

    std::vector<Record*> records;
    std::vector<RecordId*> recordIds;
    fileHandler->getAllRecordsAccordingToFields(records, recordIds, (1 << index));
    std::vector<void*> insertDatas;
    std::vector<int> insertVals;
    std::vector<int> pageIds, slotIds;
    for (int i = 0; i < records.size(); i++) {
        insertDatas.push_back((void*)(records[i]->data));
        pageIds.push_back(recordIds[i]->getPageId());
        slotIds.push_back(recordIds[i]->getSlotId());
    }
    insertVals.resize(records.size());
    indexManager->transform(tableName.c_str(), colName.c_str(), insertVals, pageIds, slotIds);
    indexManager->insert(tableName.c_str(), colName.c_str(), insertDatas, insertVals);
    return index;
}

int TableManager::dropPrimaryKey(string tableName, string colName) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[Error] table dose not exist!\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int index = checkColExist(tableHeader, colName.c_str());
    if (index >= 0) {
        if (tableHeader->entrys[index].primaryKeyConstraint == false) {
            printf("[Info] there is no primary key constraint on column %s\n", colName.c_str());
            return index;
        }
        tableHeader->entrys[index].primaryKeyConstraint = false;
    } else {
        printf("[Error] specified column does not exist.\n");
        return -1;
    }

    indexManager->removeIndex(tableName.c_str(), colName.c_str());

    return index;
}

int TableManager::createForeignKey(string tableName, string foreignKeyName, string colName, string refTableName, string refTableCol) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[Error] table dose not exist!\n");
        return -1;
    }
    if(!checkTableName(refTableName))
        return -1;
    string tablePath = "database/" + databaseName + '/' + refTableName +".db";
    if(!checkTableExist(tablePath)) {
        printf("[Error] table dose not exist!\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int index = checkColExist(tableHeader, colName.c_str());
    FileHandler* refFileHandler = recordManager->findTable(refTableName.c_str());
    TableHeader* refTableHeader = refFileHandler->getTableHeader();
    assert(refFileHandler != nullptr);
    int refIndex = checkColExist(refFileHandler->getTableHeader(), refTableCol.c_str());
    if (index >= 0 && refIndex >= 0) {
        if (refTableHeader->entrys[refIndex].primaryKeyConstraint == false) {
            printf("[Error] can not build foreign key on ref table's non-primary-key field.\n");
            return -1;
        }
        if (tableHeader->entrys[index].foreignKeyConstraint == true) {
            printf("[Error] the column %s already has a foreign key.\n", colName.c_str());
            return -1;
        }
        tableHeader->entrys[index].foreignKeyConstraint = true;
        strcpy(tableHeader->entrys[index].foreignKeyTableName, refTableName.c_str());
        strcpy(tableHeader->entrys[index].foreignKeyColName, refTableCol.c_str());
    } else {
        printf("[Error] specified column does not exist.\n");
        return -1;
    }
    return index;
}

int TableManager::dropForeignKey(string tableName, uint8_t colIndex) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[Error] table dose not exist!\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    if(tableHeader->entrys[colIndex].foreignKeyConstraint == false) {
        printf("[Error] this table dose not have this foreign key\n");
        return -1;
    }
    tableHeader->entrys[colIndex].foreignKeyConstraint = false;
    return 0;
}

int TableManager::createUniqueKey(string tableName, string colName) {
    int res = createIndex(tableName, colName);
    // the index is established. Next should build unique constraint
    if (res >= 0) {
        fileHandler = recordManager->findTable(tableName.c_str());
        assert(fileHandler != nullptr);
        TableHeader* tableHeader = fileHandler->getTableHeader();
        int index = checkColExist(tableHeader, colName.c_str());
        assert(index >= 0);
        if (tableHeader->entrys[index].uniqueConstraint == true) {
            printf("[Info] the unique constraint of %s has already been created.\n", colName.c_str());
            return index;
        } else if (tableHeader->entrys[index].primaryKeyConstraint == true) {
            printf("[Error] can not create unique constraint on primary key.\n");
            return -1;
        }
        tableHeader->entrys[index].uniqueConstraint = true;
    }
    return res;
}

int TableManager::dropUniqueKey(string tableName, string colName) {
    if(!checkTableName(tableName))
        return -1;
    string path = "database/" + databaseName + '/' + tableName +".db";
    if(!checkTableExist(path)) {
        printf("[Error] table dose not exist!\n");
        return -1;
    }
    fileHandler = recordManager->findTable(tableName.c_str());
    if(fileHandler == nullptr)
        return -1;
    TableHeader* tableHeader = fileHandler->getTableHeader();
    int index = checkColExist(tableHeader, colName.c_str());
    if (index >= 0) {
        if (tableHeader->entrys[index].uniqueConstraint == false) {
            printf("[Info] specified column dose not have unique constraint.\n");
            return index;
        }
        tableHeader->entrys[index].uniqueConstraint = false;
    } else {
        printf("[Error] specified column does not exist.\n");
        return -1;
    }

    assert(indexManager->hasIndex(tableName.c_str(), colName.c_str()));
    return indexManager->removeIndex(tableName.c_str(), colName.c_str());
}
