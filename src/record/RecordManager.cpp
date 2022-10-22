#include "RecordManager.h"
#include <cstdio>

// Section for Record Manager

RecordManager::RecordManager(BufPageManager* bufPageManager_, char* databaseName_) {
    bufPageManager = bufPageManager_;
    strcpy(databaseName, databaseName_);
}

RecordManager::~RecordManager() {}

inline int initFile(BufPageManager* bufPageManager, const char* filename) {
    int fileId;
    if (bufPageManager->fileManager->openFile(filename, fileId)) {
        int index;
        BufType page = bufPageManager->allocPage(fileId, 0, index, false);
        page[0] = 0;
        bufPageManager->markDirty(index);
        bufPageManager->writeBack(index);
        bufPageManager->fileManager->closeFile(fileId);
        return 0;
    }
    return -1;
}

int RecordManager::createFile(const char* tableName) {
    char filename[TAB_MAX_NAME_LEN];
    sprintf(filename, "database/%s_%s.db", databaseName, tableName);
    if (bufPageManager->fileManager->createFile(filename)) {
        return initFile(bufPageManager, filename);
    }
    return -1;
}

int RecordManager::removeFile(const char* tableName) {
    char filename[TAB_MAX_NAME_LEN];
    sprintf(filename, "database/%s_%s.db", databaseName, tableName);
    return bufPageManager->fileManager->removeFile(filename);
}

int RecordManager::openFile(const char* tableName, FileHandler* fileHandler) {
    char filename[TAB_MAX_NAME_LEN];
    sprintf(filename, "database/%s_%s.db", databaseName, tableName);
    int fileId;
    if (bufPageManager->fileManager->openFile(filename, fileId)) {
        fileHandler->init(bufPageManager, fileId, tableName);
        return 0;
    }
    return -1;
}

int RecordManager::closeFile(FileHandler* fileHandler) {
    return bufPageManager->fileManager->closeFile(fileHandler->getFileId());
}
