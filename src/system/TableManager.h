/* TODO: basic
1. create a table
2. list table infomation
3. drop a table
4. rename a table
*/
#ifndef __TABLEMANAGER_H__
#define __TABLEMANAGER_H__

#include <string>
using namespace std;

class TableManager {
private:
    string path;
public:
    TableManager(string path);
    ~TableManager();

    int creatTable(string name);
    int dropTable(string name);
    int listTableInfo(string name);
    int renameTable(string name);
};

#endif