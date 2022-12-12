
#ifndef __DATABASEMANAGER_H__
#define __DATABASEMANAGER_H__

#include <string>
#include "TableManager.h"
#include "../filesystem/fileio/FileManager.h"
#include "../filesystem/bufmanager/BufPageManager.h"

using namespace std;

struct DBMeta {
    int tableNum;
    int colNum[DB_MAX_TABLE_NUM];
    int foreignKeyNum;

    char tableNames[DB_MAX_TABLE_NUM][TAB_MAX_NAME_LEN];
    
    bool isPrimaryKey[DB_MAX_TABLE_NUM][TAB_MAX_COL_NUM]; // wheater a column is a primary key
    
    char foreignKeyNames[MAX_FOREIGN_KEY_NUM][64];
    uint8_t foreignKeyColumn[MAX_FOREIGN_KEY_NUM]; // use foreign key index to quickly find column 
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
     * @brief list all the tables' name of the currently opened database.
     * SHOW DATABASE <database name>;
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
     * SHOW TABLE <table name>
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
    int createIndex(string tableName, string indexName, string colName, uint16_t indexLen_);

    int dropIndex(string tableName, string indexName);

    /**
     * @brief add primary key constraint in the table entry and modify meta data
     * ALTER TABLE <table name> ADD PRIMARY KEY (<column name1>, <column name2>, ...);
     * @param colNum the quantity of column need to be added primary key
     * 
    */
    int createPrimaryKey(string tableName, vector<string> colNames, int colNum);

    int dropPrimaryKey(string tableName, vector<string> colNames, int colNum);
    
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

    int insertRecord();

    int dropRecord();

};

#endif