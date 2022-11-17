/* TODO: basic
1. create a table
2. list table infomation
*/
#ifndef __TABLEMANAGER_H__
#define __TABLEMANAGER_H__

#include <string>
#include "../record/RecordManager.h"
#include "../filesystem/bufmanager/BufPageManager.h"
#include "../common.h"
using namespace std;

class TableManager {
private:
    string databaseName;
    RecordManager* recordManager;
    FileHandler* fileHandler;
    int tableNum;
public:
    TableManager(string databaseName_, BufPageManager* bufPageManager_);
    ~TableManager();

    inline bool checkTableName(string name);

    inline bool checkTableExist(string path);
    /**
     * 表名， 每一列的数据类型
     * TODO: 创建表时定义好每一列的数据类型以及长度
    */
    int creatTable(string tableName, char colName[][COL_MAX_NAME_LEN], TB_COL_TYPE* colType, int* colLen, int colNum);

    int openTable(string name);
    
    int dropTable(string name);

    int listTableInfo(string name);

    /**
     * 流程：
     * 1.检查表名是否合法
     * 2.检查旧表文件是否存在
     * 3.关闭旧表的文件
     * 4.重命名
     * 5.重新打开新表
    */
    int renameTable(string oldName, string newName);

    int saveChangeToFile(const char* tableName);
};

#endif