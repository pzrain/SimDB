#include "TableManager.h"
#include <unistd.h>

TableManager::TableManager(string databaseName_,  BufPageManager* bufPageManager_) {
    databaseName = databaseName_;
    recordManager = new RecordManager(bufPageManager_, databaseName.c_str());
}

inline bool TableManager::checkTableName(string name) {
    size_t length = name.length();
    if(length == 0 || length > TAB_MAX_NAME_LEN) {
        printf("[Error] invalid table name ! \n");
        return false;
    }
    return true;
}

inline bool TableManager::checkTableExist(string path) {
    if (!access(path.c_str(), F_OK))
        return true; // already exit
    return false;
}

int TableManager::creatTable(string name) {
    if(!checkTableName(name))
        return -1;
    string path = "database/" + databaseName + '/' + name +".db";
    if(!checkTableExist(path))
        if(recordManager->createFile(name.c_str()) == 0)
            return 0;
    printf("[Error] fail to creat the table named %s\n", name.c_str());
    return -1;
}

int TableManager::openTable(string name) {
    if(!checkTableName(name))
        return -1;
    string path = "database/" + databaseName + '/' + name +".db";
    FileHandler* f;
    if(checkTableExist(path))
        if(recordManager->openFile(name.c_str(), f) == 0)
            return 0;
    printf("[Error] table %s has already been opened. \n", name.c_str());
    return -1;
}

int TableManager::dropTable(string name) {
    if(!checkTableName(name))
        return -1;
    string path = "database/" + databaseName + '/' + name +".db";
    if(checkTableExist(path))
        if(recordManager->removeFile(name.c_str()) == 0)
            return 0;
    printf("[Error] fail to drop the table named %s\n", name.c_str());
    return -1;
}

// int TableManager::renameTable(string oldName, string newName) {

// }