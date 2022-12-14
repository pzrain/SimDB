
#ifndef __DATABASEMANAGER_H__
#define __DATABASEMANAGER_H__

#include <string>
#include "TableManager.h"
#include "DBComponent.h"
#include "../filesystem/fileio/FileManager.h"
#include "../filesystem/bufmanager/BufPageManager.h"

using namespace std;

struct DBMeta {
    int tableNum;
    int colNum[DB_MAX_TABLE_NUM];
    int foreignKeyNum;

    char tableNames[DB_MAX_TABLE_NUM][TAB_MAX_NAME_LEN];
    
    bool isPrimaryKey[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM]; // wheater a column is a primary key
    bool isUniqueKey[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM];
    
    char foreignKeyNames[MAX_FOREIGN_KEY_NUM][64];
    uint8_t foreignKeyColumn[MAX_FOREIGN_KEY_NUM]; // use foreign key index to quickly find column 
    // more ...

    DBMeta();
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
     * @brief -
     * CREATE DATABASE <databse name>;
    */
    int createDatabase(string name);

    /**
     * @brief -
     * DROP DATABASE <database name>;
    */
    int dropDatabase(string name);

    /**
     * @brief save all the tables of the last opened table if there is one, then open the new table.
     * USE DATABASE <database name>;
    */
    int switchDatabase(string name);

    /**
     * @brief show all the databases created
     * SHOW DATABASES
     * 
     * @return 0 if succeed, -1 if encounter error
     */
    int showDatabases();

    string getDatabaseName();

    /**
     * @brief list all the tables' name of the currently opened database.
     * SHOW DATABASE <database name>;
     * SHOW TABLES
    */
    int listTablesOfDatabase(string name);

    /**
     * @brief set the property of all the columns of the table.
     * CREATE TABLE <table name>(
     *     <colname> coltype,
     *     ...
     * );
    */
    int createTable(string name, char colName[][COL_MAX_NAME_LEN], TB_COL_TYPE* colType, int* colLen, int colNum);

    /**
     * @brief only print all columns' name and data type. 
     * SHOW TABLE <table name> (maybe DESC <table name> ?)
    */
    int listTableInfo(string name);

    /**
     * @brief also modify meta data.
     * DROP TABLE <table name>;
    */
    int dropTable(string name);

    /**
     * @brief see the function in TableManager.cpp
     * ALTER TABLE <old table name> RENAME TO <new table name>;
    */
    int renameTable(string oldName, string newName);

    /**
     * not sure
    */
    int createIndex(string tableName, string colName);

    int dropIndex(string tableName, string colName);

    int hasIndex(string tableName, string colName);

    int showIndex();

    /**
     * @brief add primary key constraint in the table entry and modify meta data
     * ALTER TABLE <table name> ADD PRIMARY KEY (<column name1>, <column name2>, ...);
     * @param colNum the quantity of column need to be added primary key
     * 
    */
    int createPrimaryKey(string tableName, vector<string> colNames, int colNum);

    int dropPrimaryKey(string tableName, vector<string> colNames, int colNum);

    /**
     * @brief Create a Unique Key object. Refer to createPrimaryKey for more information
     * 
     * @param tableName 
     * @param colNames 
     * @param colNum 
     * @return int 
     */
    int createUniqueKey(string tableName, vector<string> colNames, int colNum);

    int dropUniqueKey(string tableName, vector<string> colNames, int colNum);
    
    /**
     * @brief add foreign key constraint in the table entry and modify meta data
     * ALTER TABLE <table name> ADD CONSTRAINT <foreign key name>
     * FOREIGN KEY(<column name>) REFERENCES <reference table name>(<reference column>);
    */
    int createForeignKey(string tableName, string foreignKeyName, string colName, string refTableName, string refTableCol);

    /**
     * @brief use foreign key's name to find the entry and modify it.
    */
    int dropForeignKey(string tableName, string foreignKeyName);

    /**
     * @brief select statement, including fuzzy, nesty, group by…… selection
     * 
     * @param dbSelect 
     * @return number of queries affected, -1 if encounters error
     */
    int selectRecords(DBSelect* dbSelect);
    // SELECT column FROM table WHERE expr;
    // parameters are encapsulated mainly due to nesty selections

    int updateRecords(string tableName, DBUpdate* dbUpdate);

    int insertRecords(string tableName, DBInsert* dbInsert);

    int dropRecords(string tableName, DBDelete* dbDelete);

};

#endif