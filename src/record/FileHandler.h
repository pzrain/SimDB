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

    int operateTable(TB_OP_TYPE opCode, char* colName = nullptr, TableEntry* tableEntry = nullptr, int num = 0);
    /* 
        When operating type is TB_INIT, tableEntry represents an array of TableEntry

        TB_REMOVE, TB_EXIST requires parameter colName
        TB_INIT, TB_ALTER, TB_ADD requires parameter tableEntry
        TB_INIT requires parameter num, which is the total number of tableEntry array

        @return: -1 if fail, 
                 0 if succeed for all op other than TB_EXIST 
                 TB_EXIST returns 1 if exists otherwise 0
     */
    
    bool getRecord(RecordId recordId, Record &record);

    bool insertRecord(RecordId &recordId, Record &record);
    // the page id and slot id of the inserted record will be stored in recordId
    // at present, no constraints will be checked. (TODO)

    bool removeRecord(RecordId &recordId);

    bool updateRecord(RecordId &recordId, Record &record);
    // parameter recordId specifies the position of the record that needed to be updated
    // parameter record will substitutes the old record

    void getAllRecords(std::vector<Record*>&);
    // returns all records stores in this file

    bool getAllRecordsAccordingToFields(std::vector<Record*>&, const uint16_t enable = 0);
    // get specific fields of all records storing in this file
    // if you want get i-th field, please set the i-th bit of enable (from low to high) to 1
    // return true if succeed
    // to be implemented

    bool insertAllRecords(const std::vector<Record*>&);
    // insert records in bulk

    int getRecordNum();

    size_t getRecordLen();

    TableEntryDesc getTableEntryDesc();

    TableHeader* getTableHeader();

};


#endif