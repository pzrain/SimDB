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

    /**
     * @brief called when create a table
     *        create the file corresponding to a table in a database
     * @return -1 if fail
     */
    int createFile(const char* tableName); 

    /**
     * @brief called when remove a table
     *        remove the file corresponding to a table in a database
     */
    int removeFile(const char* tableName);

    /**
     * @brief open a file according to the tableName
     */
    FileHandler* openFile(const char* tableName);

    int closeFile(FileHandler* fileHandler);

    FileHandler* findTable(const char* tableName);

    void renameTable(const char* oldName, const char* newName);

    bool isValid();
};

#endif