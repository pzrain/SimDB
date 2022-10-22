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
} TB_OP_TYPE;

class FileHandler{
private:
    BufPageManager* bufPageManager;
    TableHeader* tableHeader;
    int fileId;
public:

    FileHandler();

    ~FileHandler();

    void init(BufPageManager* bufPageManager_, int fileId_, const char* tableName);

    int getFileId();

    int operateTable(TB_OP_TYPE opCode, char* colName = nullptr, TableEntry* tableEntry = nullptr, int num = 0);
    /* 
        when operating type is init, tableEntry represents an array of TableEntry

        @return: -1 if fail, 0 if succeed for all op other than TB_EXIST, which returns 0 or 1
     */
    
    bool getRecord(RecordId recordId, Record &record);

    bool insertRecord(RecordId &recordId, const Record &record);
    // the page id and slot id of the inserted record will be stored in recordId

    bool removeRecord(RecordId &recordId);

    bool updateRecord(RecordId &recordId, const Record &record);

    void getAllRecords(std::vector<Record*>&);

    void insertAllRecords(const std::vector<Record*>&);

    int getRecordNum();

    size_t getRecordLen();

};


#endif