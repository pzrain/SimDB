
#ifndef __DATABASEMANAGER_H__
#define __DATABASEMANAGER_H__

#include <string>
#include "TableManager.h"
#include "../filesystem/fileio/FileManager.h"
#include "../filesystem/bufmanager/BufPageManager.h"

using namespace std;

struct DBMeta {
    int tableNum;
    char tableNames[DB_MAX_TABLE_NUM][TAB_MAX_NAME_LEN];
    // more ...
};
class DatabaseManager {
private:
    string BASE_PATH;
    string databaseUsedName;
    bool databaseUsed;

    int databaseStroeFileId;
    DBMeta* metaData;

    FileManager* fileManager;
    BufPageManager* bufPageManager;
    TableManager* tableManager;



    inline bool checkDatabaseName(string name);

    inline bool checkFileExist(string path);

    inline int searchTableByName(string name);

    int readMetaData(int fileId, DBMeta* metaData);

    int writeMetaData(int fileId, DBMeta* metaData);

public:
    DatabaseManager();
    ~DatabaseManager();

    /**
     * CREATE DATABASE <databse name>;
    */
    int createDatabase(string name);

    /**
     * DROP DATABASE <database name>;
    */
    int dropDatabase(string name);

    /**
     * USE DATABASE <database name>;
     * @brief 
     * 切换前先把上一个打开的数据库（如果有的话）
     * 所有的表保存回去，可能造成性能上的损失
    */
    int switchDatabase(string name);

    /**
     * SHOW DATABASE <database name>;
    */
    int listTablesOfDatabase(string name);

    /**
     * CREATE TABLE <table name>(
     *     <colname> coltype,
     *     ...
     * );
    */
    int createTable(string name, char colName[][COL_MAX_NAME_LEN], TB_COL_TYPE* colType, int* colLen, int colNum);

    /**
     * SHOW TABLE <table name>
    */
    int listTableInfo(string name);
    /**
     * DROP TABLE <table name>;
    */
    int dropTable(string name);

    /**
     * ALTER TABLE <old table name> RENAME TO <new table name>;
    */
    int renameTable(string oldName, string newName);

};

#endif