/* TODO: basic
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
    
    int creatTable(string tableName, TableEntry* entrys, int colNum);

    int openTable(string name);
    
    int dropTable(string name);

    int listTableInfo(string name);

    /**
     * @brief: 重命名流程
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