#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include "RMComponent.h"
#include "../filesystem/bufmanager/BufPageManager.h"
#include <vector>

typedef enum{
    TB_INIT = 0,
    TB_ALTER = 1,
    TB_REMOVE = 2,
    TB_ADD = 3,
    TB_EXIST = 4,
    TB_PRINT = 5,
} TB_OP_TYPE; // type of operating a table

class FileHandler{
private:
    BufPageManager* bufPageManager;
    TableHeader* tableHeader;
    int fileId;
    char tableName[TAB_MAX_NAME_LEN];

public:

    FileHandler();

    ~FileHandler();

    void init(BufPageManager* bufPageManager_, int fileId_, const char* tableName_);

    int getFileId();

    char* getTableName();

    /**
     * @brief When operating type is TB_INIT, tableEntry represents an array of TableEntry
     *        TB_REMOVE, TB_EXIST requires parameter colName
     *        TB_INIT, TB_ALTER, TB_ADD requires parameter tableEntry
     *        TB_INIT requires parameter num, which is the total number of tableEntry array
     * @return -1 if fail,
     *         0 if succeed for all op other than TB_EXIST (TB_EXIST returns 1 if exists otherwise 0)
     */
    int operateTable(TB_OP_TYPE opCode, char* colName = nullptr, TableEntry* tableEntry = nullptr, int num = 0);
    
    bool getRecord(RecordId recordId, Record &record);

    /**
     * @brief the page id and slot id of the inserted record will be stored in recordId      
     */
    bool insertRecord(RecordId &recordId, Record &record);

    bool removeRecord(RecordId &recordId, Record &record);

    /**
     * @brief parameter recordId specifies the position of the record that needed to be updated
     *        parameter record will substitutes the old record
     */
    bool updateRecord(RecordId &recordId, Record &record);

    /**
     * @brief ATTENTION: you should manually delete the pointer to avoid memory leak
     * @return all records stores in this file
     */
    void getAllRecords(std::vector<Record*>&, std::vector<RecordId*>&);

    /**
     * @brief get specific fields of all records storing in this file
     *        if you want get i-th field, please set the i-th bit of enable (from low to high) to 1
     * @return true if successful
     */
    bool getAllRecordsAccordingToFields(std::vector<Record*>&, std::vector<RecordId*>&, const uint16_t enable = 0);

    /**
     * @brief insert records in bulk
     */
    bool insertAllRecords(const std::vector<Record*>&, std::vector<RecordId>&);

    int getRecordNum();

    size_t getRecordLen();

    TableEntryDesc getTableEntryDesc();

    TableHeader* getTableHeader();

    void renameTable(const char* newName);

    void saveTableHeader();

    /**
     * @brief transform pageId together with slotId to a int value
     *        used in indexing
     */
    void transform(int& val, int pageId, int slotId);

    void transform(std::vector<int>& vals, std::vector<int> pageIds, std::vector<int> slotIds);

    void transformR(int val, int& pageId, int& slotId);

    void transformR(std::vector<int> vals, std::vector<int>& pageIds, std::vector<int>& slotIds);

};


#endif