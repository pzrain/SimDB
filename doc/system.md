## <center>系统管理</center>

系统管理模块位于SQL解析以及记录管理、索引管理中间，负责接受、处理SQL解析后的指令，并实际操作相关记录和索引。其功能主要分为两部分，分别为数据表的管理以及查询解析。前者对应SQL中的DDL（Data Definition Language），后者对应SQL中的DML（Data Manipulation Language）。对外接口说明如下：

```C++
class DatabaseManager {
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
    int listTablesOfDatabase();

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
     * @brief Create a index on a specific table.column
     * 'ALTER' 'TABLE' Identifier 'ADD' 'INDEX' '(' identifiers ')'   
     */
    int createIndex(string tableName, string colName);

    /**
     * @brief Drop the index on table.column
     * 'ALTER' 'TABLE' Identifier 'DROP' 'INDEX' '(' identifiers ')'
     */
    int dropIndex(string tableName, string colName);

    int hasIndex(string tableName, string colName);

    /**
     * @brief Display all the index of a table
     * 'SHOW' 'INDEXES'
     */
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
     * 'SELECT' selectors 'FROM' identifiers ('WHERE' where_and_clause)? ('GROUP' 'BY' column)? ('LIMIT' Integer ('OFFSET' Integer)?)?
     * @param dbSelect 
     * @return number of queries affected, -1 if encounters error
     */
    int selectRecords(DBSelect* dbSelect);

    /**
     * @brief update statement
     * 'UPDATE' Identifier 'SET' set_clause 'WHERE' where_and_clause
     * @param dbUpdate 
     * @return int 
     */
    int updateRecords(string tableName, DBUpdate* dbUpdate);

    /**
     * @brief insert statement
     * 'INSERT' 'INTO' Identifier 'VALUES' value_lists
     * @param dbInsert 
     * @return int 
     */
    int insertRecords(string tableName, DBInsert* dbInsert);

    /**
     * @brief delete statement
     * 'DELETE' 'FROM' Identifier 'WHERE' where_and_clause
     * @param dbDelete 
     * @return int 
     */
    int dropRecords(string tableName, DBDelete* dbDelete);

};
```