#include "TableManager.h"
#include "assert.h"
#include <map>
#include <regex>
#include <algorithm>
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
        printf("[Error] error in opening table %s.\n", name.c_str());
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
    if (fileHandler == nullptr) {
        return -1;
    }
    recordManager->closeFile(fileHandler);
    // TODO: test if changes have been saved
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
                printf("[Error] no such table in %s.%s\n", waitChecked[i]->expTable.c_str(), waitChecked[i]->expCol.c_str());
                return -1;
            }
            int index = checkColExist(tableHeaders[j], waitChecked[i]->expCol.c_str());
            if (index < 0) {
                printf("[Error] no such column %s in table%s.\n", waitChecked[i]->expCol.c_str(), waitChecked[i]->expTable.c_str());
                return -1;
            }
        } else {
            int hitTable = 0;
            for (int j = 0; j < tableSize; j++) {
                if (checkColExist(tableHeaders[j], waitChecked[i]->expCol.c_str())) {
                    hitTable++;
                    waitChecked[i]->expTable = selectTables[j];
                }
            }
            if (hitTable == 0) {
                printf("[Error] no table has such column %s.\n", waitChecked[i]->expCol.c_str());
                return -1;
            } else if (hitTable >= 2) {
                printf("[Error] ambiguous column %s.\n", waitChecked[i]->expCol.c_str());
                return -1;
            }
        }
    }
    return 0;
}

inline bool compareEqual(RecordDataNode* cur, RecordDataNode* pre) {
    if (cur->nodeType != pre->nodeType || cur->len != pre->len) {
        return false;
    }
    switch (cur->nodeType)
    {
    case COL_INT:
        return *(int*)(cur->content.intContent) == *(int*)(pre->content.intContent);
        break;
    case COL_FLOAT:
        return *(float*)(cur->content.floatContent) == *(int*)(pre->content.floatContent);
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

void* transformType(RecordDataNode* recordDataNode) {
    void* res = nullptr;
    switch (recordDataNode->nodeType) {
        case COL_INT:
            res = recordDataNode->content.intContent;
            break;
        case COL_FLOAT:
            res = recordDataNode->content.floatContent;
            break;
        case COL_VARCHAR:
            res = recordDataNode->content.floatContent;
            break;
        default:
            break;
    }
    return res;
}

int TableManager::_iterateWhere(vector<string> selectTables, vector<DBExpression> expressions, vector<RecordData>& resRecords, vector<RecordId*>& resRecordIds) {
    int cur = 0;
    std::vector<RecordId*> res[2], tempRes;
    FileHandler* fileHandlers[MAX_SELECT_TABLE];
    TableHeader* tableHeaders[MAX_SELECT_TABLE];
    std::vector<Record*> records[MAX_SELECT_TABLE];
    std::vector<RecordId*> recordIds[MAX_SELECT_TABLE];
    int tableNum = selectTables.size();
    for (int i = 0; i < tableNum; i++) {
        fileHandlers[i] = recordManager->findTable(selectTables[i].c_str());
        tableHeaders[i] = fileHandlers[i]->getTableHeader();
        fileHandlers[i]->getAllRecords(records[i], recordIds[i]);
    }
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
        assert(false);
    }

    for (int i = 0; i < expressions.size(); i++) {
        bool preFlag = false; // flag indicating the previous item has been successfully selected
        std::vector<RecordId*> preRes;
        cur ^= 1;
        res[cur].clear();
        tempRes.clear();
        assert(expressions[i].lType == DB_ITEM); // in SQL.g4, the left must be an item;
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
                printf("[Error] null type currently is not supported in op other than IS(NOT) NULL.\n");
                return -1;
            }
            if (expressions[i].rType == DB_ITEM) {
                DBExpItem* rItem = (DBExpItem*)(expressions[i].rVal);
                int rFileId = (rItem->expTable == selectTables[0]) ? 0 : 1;
                int rColId = tableHeaders[rFileId]->getCol(rItem->expCol.c_str());
                TableEntryDesc tableEntryDesc = tableHeaders[rFileId]->getTableEntryDesc();
                TableEntryDescNode* tableEntryDescNode = tableEntryDesc.getCol(rColId);
                if (tableEntryDescNode->colType != curRecordDataNode->nodeType) {
                    printf("[Error] %s.%s and %s.%s don't have compatible types.\n", lItem->expTable.c_str(), lItem->expCol.c_str(), rItem->expTable.c_str(), rItem->expCol.c_str());
                    return -1;
                }
                std::vector<int> indexRes, indexResTemp;
                if (rFileId == fileId) { // the same table
                    void* rData = curRecordData.getData(rColId);
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
                            assert(false);
                            break;
                    }
                    if (flag) {
                        res[cur].push_back(curRecordId);
                        if (tableNum > 0) {
                            res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        }
                    }
                    continue;
                }
                // rFileId != fileId, different table
                if (equalAsPre) {
                    for (int k = 0; k < preRes.size(); k++) {
                        res[cur].push_back(curRecordId);
                        res[cur].push_back(preRes[k]);
                    }
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
                            case GT_TYPE:
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, nullptr, indexRes, false, true);
                                break;
                            case GTE_TYPE:
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), searchData, nullptr, indexRes, true, true);
                                break;
                            case LT_TYPE:
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), nullptr, searchData, indexRes, true, false);
                                break;
                            case LTE_TYPE:
                                indexManager->searchBetween(rItem->expTable.c_str(), rItem->expCol.c_str(), nullptr, searchData, indexRes, true, true);
                                break;
                            default:
                                fprintf(stderr, "encouter error op type %d in search.\n", expressions[i].op);
                                assert(false);
                                break;
                        }
                        std::map<int, bool> ma; // hash to get the intersection of two sets
                        for (int k = 0; k < indexRes.size(); k++) {
                            ma[indexRes[k]] = true;
                        }
                        preRes.clear();
                        for (int k = rFileId; k < res[cur ^ 1].size(); k += tableNum) {
                            if (tableNum > 1 && res[cur ^ 1][k ^ 1] != curRecordId) {
                                continue;
                            }
                            int val;
                            RecordId* rRecordId = res[cur ^ 1][k];
                            indexManager->transform(rItem->expTable.c_str(), rItem->expCol.c_str(), val, rRecordId->getPageId(), rRecordId->getSlotId());
                            if (ma.count(val) > 0) {
                                res[cur].push_back(curRecordId);
                                preRes.push_back(rRecordId);
                                res[cur].push_back(rRecordId);
                            }
                        }

                    } else { // search using recordManager
                        RecordData searchRecordData;
                        RecordDataNode* searchRecordDataNode;
                        preRes.clear();
                        for (int k = rFileId; k < res[cur ^ 1].size(); k += tableNum) {
                            if (tableNum > 1 && res[cur ^ 1][k ^ 1] != curRecordId) {
                                // only choose those satisfy previous conditions;
                                continue;
                            }
                            RecordId* rRecordId = res[cur ^ 1][k];
                            Record* searchRecord;
                            fileHandlers[rFileId]->getRecord(*rRecordId, *searchRecord);
                            searchRecord->deserialize(searchRecordData,tableEntryDesc);
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
                                    assert(false);
                                    break;
                            }
                            if (flag) {
                                res[cur].push_back(curRecordId);
                                preRes.push_back(rRecordId);
                                res[cur].push_back(rRecordId);
                            }
                        }
                    }
                }
            } else if (expressions[i].rType == DB_NST) {
                if (expressions[i].op != IN_TYPE) {
                    printf("[Error] op %d is not supported for nesty selection.\n");
                    return -1;
                }
                vector<RecordData> tempRecordData;
                if (((DBSelect*)(expressions[i].rVal))->selectItems.size() > 1) {
                    printf("[Error] nesty selection in tuple form is incompatible with IN op.\n");
                    return -1;
                }
                if (equalAsPre && preFlag) {
                    res[cur].push_back(curRecordId);
                    if (tableNum > 0) {
                        res[cur].push_back(res[cur ^ 1][j ^ 1]);
                    }
                    continue;
                }
                // recursive search
                _selectRecords((DBSelect*)expressions[i].rVal, tempRecordData, tempRes);
                preFlag = false;
                for (int k = 0; k < tempRecordData.size(); k++) {
                    void* rData = transformType(tempRecordData[k].head);
                    if (compare->equ(searchData, rData)) {
                        res[cur].push_back(curRecordId);
                        if (tableNum > 0) {
                            res[cur].push_back(res[cur ^ 1][j ^ 1]);
                        }
                        preFlag = true;
                        break;
                    }    
                }
            } else if (expressions[i].rType == DB_LIST) {
                if (equalAsPre && preFlag) {
                    res[cur].push_back(curRecordId);
                    if (tableNum > 0) {
                        res[cur].push_back(res[cur ^ 1][j ^ 1]);
                    }
                    continue;
                }
                preFlag = false;
                std::vector<void*>* valueList = (std::vector<void*>*)expressions[i].rVal;
                for (int k = 0; k < expressions[i].valueListType.size(); k++) {
                    if (expressions[i].valueListType[k] == curRecordDataNode->nodeType) {
                        if (compare->equ((*valueList)[k], searchData)) {
                            res[cur].push_back(curRecordId);
                            if (tableNum > 0) {
                                res[cur].push_back(res[cur ^ 1][j ^ 1]);
                            }
                            preFlag = true;
                            break;
                        }
                    } 
                }
            } else if (expressions[i].rType == DB_NULL) {
                if (expressions[i].op != IS_TYPE && expressions[i].op != ISN_TYPE) {
                    fprintf(stderr, "Null should have op type IS or IS NOT.\n");
                    assert(false);
                }
                if (curRecordDataNode->nodeType == COL_NULL) {
                    res[cur].push_back(curRecordId);
                    if (tableNum > 0) {
                        res[cur].push_back(res[cur ^ 1][j ^ 1]);
                    }
                }
            } else { // DB_INT, DB_CHAR, DB_FLOAT
                if (curRecordDataNode->nodeType != expressions[i].rType) {
                    printf("[Error] incompatible type for %s.%s and %d.\n", lItem->expTable.c_str(), lItem->expCol.c_str(), expressions[i].rType);
                    return -1;
                }
                bool flag = false;
                std::string regStr = "";
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
                    case LIKE_TYPE:
                        if (expressions[i].rType != DB_CHAR) {
                            printf("[Error] right expression in LIKE must be VARCHAR.\n");
                            return -1;
                        }
                        regStr = ((char*)expressions[i].rVal);
                        string::size_type pos(0); 
                        while ((pos = regStr.find("%")) != std::string::npos) {
                            regStr.replace(pos, 1, ".*");
                        }
                        if (std::regex_match((char*)searchData, std::regex(regStr))) {
                            flag = true;
                        }
                        break;
                    default:
                        assert(false);
                        break;
                }
                if (flag) {
                    res[cur].push_back(curRecordId);
                    if (tableNum > 0) {
                        res[cur].push_back(res[cur ^ 1][j ^ 1]);
                    }
                }
            }
        }
        preRecordDataNode = curRecordDataNode;
    }
}

int TableManager::_selectRecords(DBSelect* dbSelect, vector<RecordData>& records, vector<RecordId*>& recordIds) {
    records.clear();
    
    int tableSize = dbSelect->selectTables.size();
    if (tableSize > MAX_SELECT_TABLE) {
        printf("[Error] join selection for more than two tables is not supported.\n");
        return -1;
    }
    FileHandler* fileHandlers[MAX_SELECT_TABLE];
    TableHeader* tableHeaders[MAX_SELECT_TABLE];
    for (int i = 0; i < tableSize; i++) {
        fileHandlers[i] = recordManager->findTable(dbSelect->selectTables[i].c_str());
        tableHeaders[i] = fileHandlers[i]->getTableHeader();
    }

    /* check format */
    vector<DBExpItem*> waitChecked;
    for (int i = 0; i < dbSelect->selectItems.size(); i++) {
        int index;
        if (!dbSelect->selectItems[i].star) {
            waitChecked.push_back(&dbSelect->selectItems[i].item);
        }
    }
    for (int i = 0; i < dbSelect->expressions.size(); i++) {
        if (dbSelect->expressions[i].lType == DB_ITEM) {
            waitChecked.push_back(((DBExpItem*)dbSelect->expressions[i].lVal));
        }
    }
    if (dbSelect->groupByEn) {
        waitChecked.push_back(&dbSelect->groupByCol);
    }
    if (_checkFormat(fileHandlers, tableHeaders, dbSelect->selectTables, waitChecked) == -1) {
        return -1;
    }

    assert(dbSelect->limitEn || (!dbSelect->limitEn && !dbSelect->offsetEn));
    if ((dbSelect->limitEn && dbSelect->limitNum < 0) || (dbSelect->offsetEn && dbSelect->offsetNum < 0)) {
        printf("[Error] invalid value of limitNum %d or offsetNum %d.\n", dbSelect->limitNum, dbSelect->offsetNum);
        return -1;
    }

    _iterateWhere(dbSelect->selectTables, dbSelect->expressions, records, recordIds);

    /* group by: https://stackoverflow.com/questions/11991079/select-a-column-in-sql-not-in-group-by */
}

int TableManager::selectRecords(DBSelect* dbSelect) {
    vector<RecordData> records;
    vector<RecordId*> recordIds;
    int cnt = _selectRecords(dbSelect, records, recordIds);
    // print the records

    return cnt;
}
