#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include "RMComponent.h"
#include "FileManager.h"
#include "RMComponent.h"

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
    FileManager* fileManager;
    TableHeader* tableHeader;
    int fileId;
public:

    FileHandler();

    ~FileHandler();

    void init(FileManager* fileManager_, int fileId_);

    int getFileId();

    int operateTable(TB_OP_TYPE opCode, char* colName = nullptr, TableEntry* tableEntry = nullptr);
    // when operating type is init, tableEntry represents the head of a linklist of TableEntry
    
    bool getRecord(const int recordId, Record &record) const;

    bool insertRecord(const char* recordData, int &recordId);

    bool removeRecord(const int recordId);

    bool updateRecord(const Record &record);

    bool writeBack(const int pageNumber); // write page back to disk
};


#endif