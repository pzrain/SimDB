#include "TableManager.h"
#include <unistd.h>

TableManager::TableManager(string databaseName_,  BufPageManager* bufPageManager_) {
    databaseName = databaseName_;
    recordManager = new RecordManager(bufPageManager_, databaseName.c_str());
}

bool TableManager::checkTableExist(string path) {
    if (!access(path.c_str(), F_OK))
        return true; // already exit
    return false;
}

int TableManager::creatTable(string name) {
    string path = "database/" + databaseName + '/' + name +".db";
    if(!checkTableExist(path))
        if(recordManager->createFile(name.c_str()) == 0)
            return 0;
    printf("[Error] fail to creat the table named %s\n", name);
    return -1;
}

int TableManager::openTable(string name) {
    string path = "database/" + databaseName + '/' + name +".db";
    FileHandler* f;
    if(checkTableExist(path))
        if(recordManager->openFile(name.c_str(), f) == 0)
            return 0;
    printf("[Error] table %s has already been opened. \n", name);
    return -1;
}

int TableManager::dropTable(string name) {
    string path = "database/" + databaseName + '/' + name +".db";
    if(checkTableExist(path))
        if(recordManager->removeFile(name.c_str()) == 0)
            return 0;
    printf("[Error] fail to drop the table named %s\n", name);
    return -1;
}

int TableManager::renameTable(string oldName, string newName) {

}