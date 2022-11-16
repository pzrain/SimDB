/* TODO: basic
1. create a table
2. list table infomation
3. drop a table
4. rename a table
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
    int tableNum;
public:
    TableManager(string databaseName_, BufPageManager* bufPageManager_);
    ~TableManager();

    bool checkTableExist(string path);

    int creatTable(string name);

    int openTable(string name);
    
    int dropTable(string name);

    int listTableInfo(string name);

    int renameTable(string oldName, string newName);

};

#endif