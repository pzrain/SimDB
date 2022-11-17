/* TODO: basic
1. create a database
2. drop a database
3. switch to another database
4. list all the tables of the database
*/
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


    int createDatabase(string name);

    int dropDatabase(string name);

    int switchDatabase(string name);

    int listTablesofDatabase(string name);

    int createTable(string name);

    int dropTable(string name);

    int renameTable(string oldName, string newName);

};

#endif