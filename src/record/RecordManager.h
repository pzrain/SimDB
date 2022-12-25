#ifndef __RECORD_H_
#define __RECORD_H_
#include "../filesystem/bufmanager/BufPageManager.h"
#include "FileHandler.h"

class RecordManager{
private:
    bool valid;
    BufPageManager* bufPageManager;
    char databaseName[DB_MAX_NAME_LEN];
    char tableNames[DB_MAX_TABLE_NUM][TAB_MAX_NAME_LEN];
    FileHandler* fileHandlers[DB_MAX_TABLE_NUM];
    int currentIndex;

    int findEmptyIndex();

public:
    RecordManager(BufPageManager*, const char* databaseName_);

    ~RecordManager();

    int createFile(const char* tableName); 

    int removeFile(const char* tableName);

    FileHandler* openFile(const char* tableName);// one file corresponds with one fileHandler

    int closeFile(FileHandler* fileHandler);

    FileHandler* findTable(const char* tableName);

    void renameTable(const char* oldName, const char* newName);

    bool isValid();
};

#endif