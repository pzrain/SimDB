#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include "RMComponent.h"
#include "../filesystem/bufmanager/BufPageManager.h"

typedef enum{
    TB_INIT = 0,
    TB_ALTER = 1,
    TB_REMOVE = 2,
    TB_ADD = 3,
    TB_EXIST = 4,
    TB_PRINT = 5
} TB_OP_TYPE;

class FileHandler{
private:
    BufPageManager* bufPageManager;
    TableHeader* tableHeader;
    int fileId;
public:

    FileHandler();

    ~FileHandler();

    void init(BufPageManager* bufPageManager_, int fileId_, const char* filename);

    int getFileId();

    int operateTable(TB_OP_TYPE opCode, char* colName, TableEntry* tableEntry);
    // when operating type is init, tableEntry represents the head of a linklist of TableEntry
    
    bool getRecord(RecordId recordId, Record &record);

    bool insertRecord(RecordId &recordId, const Record &record);
    // the page id and slot id of the inserted record will be stored in recordId

    bool removeRecord(RecordId &recordId);

    bool updateRecord(RecordId &recordId, const Record &record);
};


#endif