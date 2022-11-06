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
#include "../common.h"
using namespace std;

class DatabaseManager {
private:
    string BASE_PATH;
    TableManager* tableManager;

    inline bool checkName(string name);
public:
    DatabaseManager();
    ~DatabaseManager();


    int createDatabase(string name);
    int dropDatabase(string name);
    int switchDatabase(string name);
    int listTablesofDatabase(string name);
};

#endif