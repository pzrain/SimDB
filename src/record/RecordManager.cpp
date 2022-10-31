#include "RecordManager.h"
#include <cstdio>

// Section for Record Manager

RecordManager::RecordManager(BufPageManager* bufPageManager_, char* databaseName_) {
    bufPageManager = bufPageManager_;
    strcpy(databaseName, databaseName_);
    currentIndex = -1;
    for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
        fileHandlers[i] = nullptr;
    }
}

RecordManager::~RecordManager() {}

int RecordManager::findEmptyIndex() {
    for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
        if (fileHandlers[i] == nullptr) {
            return currentIndex = i;
        }
    }
    return -1;
}

FileHandler* RecordManager::findTable(const char* tableName) {
    for (int i = 0; i < DB_MAX_TABLE_NUM; i++) {
        if (fileHandlers[i] != nullptr && strcmp(tableNames[i], tableName) == 0) {
            return fileHandlers[i];
        }
    }
    return nullptr;
}

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
    if (findTable(tableName) != nullptr) {
        printf("[ERROR] this table has already be opened.\n");
        return -1;
    }
    int index = findEmptyIndex();
    if (index < 0) {
        printf("[ERROR] database can accept no more tables.\n");
        return -1;
    }
    char filename[TAB_MAX_NAME_LEN];
    sprintf(filename, "database/%s_%s.db", databaseName, tableName);
    int fileId;
    if (bufPageManager->fileManager->openFile(filename, fileId)) {
        fileHandler->init(bufPageManager, fileId, tableName);
        strcpy(tableNames[index], tableName);
        fileHandlers[index] = fileHandler;
        return 0;
    }
    return -1;
}

int RecordManager::closeFile(FileHandler* fileHandler) {
    if (bufPageManager->fileManager->closeFile(fileHandler->getFileId()) == 0) {
        findTable(fileHandler->getTableName());
        if (currentIndex >= 0) {
            fileHandlers[currentIndex] = nullptr;
        }
        return 0;
    }
    return -1;
}
