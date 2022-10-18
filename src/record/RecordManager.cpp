#include "RecordManager.h"
#include <cstdio>

// Section for Record Manager

RecordManager::RecordManager(FileManager* fileManager_) {
    fileManager = fileManager_;
}

RecordManager::~RecordManager() {}

inline void initFile(FileManager* fileManager, const char* filename) {
    int fileId;
    fileManager->openFile(filename, fileId);
    char data = '0';
    fileManager->writePage(fileId, 0, (BufType)&data, 0);
    fileManager->closeFile(fileId);
}

void RecordManager::createFile(const char* filename) {
    fileManager->createFile(filename);
    initFile(fileManager, filename);
}

void RecordManager::removeFile(const char* filename) {
    fileManager->removeFile(filename);
}

void RecordManager::openFile(const char* filename, FileHandler* fileHandler) {
    int fileId;
    if (fileManager->openFile(filename, fileId)) {
        fileHandler->init(fileManager, fileId);
    } else {
        printf("[Error] Can not open file %s\n", filename);
    }
}

void RecordManager::closeFile(FileHandler* fileHandler) {
    fileManager->closeFile(fileHandler->getFileId());
}
