#ifndef __INDEX_MANAGER_H__
#define __INDEX_MANAGER_H__
#include "../filesystem/bufmanager/BufPageManager.h"
#include "BPlusTree.h"
#include <vector>
class IndexManager{
private:
    bool valid;
    BufPageManager* bufPageManager;
    char databaseName[DB_MAX_NAME_LEN];
    char tableNames[DB_MAX_TABLE_NUM][TAB_MAX_NAME_LEN];
    char indexNames[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM][TAB_MAX_NAME_LEN];
    BPlusTree* bPlusTree[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM];
    int fileIds[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM];

    BPlusTree* findIndex(const char* tableName, const char* indexName);

    int findEmptyIndex(int &emptyI, int &emptyJ);
    
public:
    IndexManager(BufPageManager* bufPageManager_, const char* databaseName_);

    ~IndexManager();

    int createIndex(const char* tableName, const char* indexName, uint16_t indexLen, uint8_t colType);
    // build index
    // return 0 if succeed, otherwise -1
    // so are the returning values of the following functions

    int removeIndex(const char* tableName, const char* indexName);
    // drop index, actually drop the corresponding B+ tree

    bool hasIndex(const char* tableName, const char* indexName);
    // return true if indexName has been created

    int insert(const char* tableName, const char* indexName, void* data, const int val);

    int insert(const char* tableName, const char* indexName, std::vector<void*> data, std::vector<int> val);
    // insert index in batch mode
    // the sequence of data is not necessarily ordered

    int search(const char* tableName, const char* indexName, void* data, std::vector<int> &res);
    // search those index whose key is equal to data
    // than store their val in vector res

    int searchBetween(const char* tableName, const char* indexName, void* lData, void* rData, std::vector<int> &res);
    // search those index whose dat falls in [lData, rData]
    // set lData or rData to nullptr to get searching range [lData, infinity] or [-infinity, rData]
    // example:
    // int lData = 3;
    // searchBetween(tableName, indexName, &lData, nullptr, res);

    int remove(const char* tableName, const char* indexName, void* data);
    // remove those index whose key is equal to data

    bool isValid();
};

#endif