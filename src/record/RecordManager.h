#ifndef __RECORD_H_
#define __RECORD_H_
#include "../filesystem/bufmanager/BufPageManager.h"
#include "FileHandler.h"

class RecordManager{
private:
    BufPageManager* bufPageManager;
    char databaseName[DB_MAX_NAME_LEN];
    char tableNames[DB_MAX_TABLE_NUM][TAB_MAX_NAME_LEN];
    FileHandler* fileHandlers[DB_MAX_TABLE_NUM];
    int currentIndex;

    int findEmptyIndex();

public:
    RecordManager(BufPageManager*, char* databaseName_);

    ~RecordManager();

    int createFile(const char* tableName); 

    int removeFile(const char* tableName);

    int openFile(const char* tableName, FileHandler* fileHandler);// one file corresponds with one fileHandler

    int closeFile(FileHandler* fileHandler);

    FileHandler* findTable(const char* tableName);
};

#endif