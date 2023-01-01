#ifndef __INDEX_MANAGER_H__
#define __INDEX_MANAGER_H__
#include "../filesystem/bufmanager/BufPageManager.h"
#include "BPlusTree.h"
#include <vector>
class IndexManager{
private:
    bool valid;
    bool validTable[DB_MAX_TABLE_NUM];
    BufPageManager* bufPageManager;
    char databaseName[DB_MAX_NAME_LEN];
    char tableNames[DB_MAX_TABLE_NUM][TAB_MAX_NAME_LEN];
    char indexNames[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM][TAB_MAX_NAME_LEN];
    BPlusTree* bPlusTree[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM];
    int fileIds[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM];

    BPlusTree* findIndex(const char* tableName, const char* indexName);

    int findEmptyIndex(int &emptyI, int &emptyJ, const char* tableName);
    
public:
    IndexManager(BufPageManager* bufPageManager_, const char* databaseName_);

    ~IndexManager();

    /**
     * @brief init index when open a database
     */
    int initIndex(std::vector<std::string> tableNames, std::vector<std::vector<std::string>> colNames, std::vector<std::vector<uint16_t>> indexLens, std::vector<std::vector<uint8_t>> colTypes);

    /**
     * @brief  rename the tableNamess
     */
    void renameIndex(const char* oldTableName, const char* newTableName);

    /**
     * @brief build index
     * @return return 0 if succeed, otherwise -1
     *         so are the returning values of other functions in IndexManager
     */
    int createIndex(const char* tableName, const char* indexName, uint16_t indexLen, uint8_t colType);

    /**
     * @brief drop index, actually drop the corresponding B+ tree
     */
    int removeIndex(const char* tableName, const char* indexName);

    /**
     * @brief return true if indexName has been created
     */
    bool hasIndex(const char* tableName, const char* indexName);

    int insert(const char* tableName, const char* indexName, void* data, const int val);

    /**
     * @brief insert index in batch mode
     *        the sequence of data is not necessarily ordered
     */
    int insert(const char* tableName, const char* indexName, std::vector<void*> data, std::vector<int> val);

    /**
     * @brief search those index whose key is equal to data
     *        than store their val in vector res
     */
    int search(const char* tableName, const char* indexName, void* data, std::vector<int> &res);

    /**
     * @brief search those index whose dat falls in [lData, rData]
     *        set lData or rData to nullptr to get searching range [lData, infinity] or [-infinity, rData]
     */
    int searchBetween(const char* tableName, const char* indexName, void* lData, void* rData, std::vector<int> &res, bool lIn = true, bool rIn = true);

    /**
     * @brief remove those index whose key is equal to data and val equal to val (if val is not set to -1)
     */
    int remove(const char* tableName, const char* indexName, void* data, int val = -1);

    /**
     * @brief update those index whose key is equal to data and val equal to oldVal
     */
    int update(const char* tableName, const char* indexName, void* data, int oldVal, int newVal);

    /**
     * @brief Display all the index
     */
    int showIndex();

    int showIndex(const char* tableName);

    void transform(const char* tableName, const char* indexName, int& val, int pageId, int slotId);

    void transform(const char* tableName, const char* indexName, std::vector<int>& vals, std::vector<int> pageIds, std::vector<int> slotIds);

    void transformR(const char* tableName, const char* indexName, int val, int& pageId, int& slotId);

    void transformR(const char* tableName, const char* indexName, std::vector<int> vals, std::vector<int>& pageIds, std::vector<int>& slotIds);

    bool isValid();
};

#endif