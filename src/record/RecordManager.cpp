#include "RecordManager.h"
#include <cstdio>

// Section for Record Manager

RecordManager::RecordManager(FileManager* fileManager_) {
    fileManager = fileManager_;
}

RecordManager::~RecordManager() {}

inline int initFile(FileManager* fileManager, const char* filename) {
    int fileId;
    if (fileManager->openFile(filename, fileId)) {
        char data = '0';
        fileManager->writePage(fileId, 0, (BufType)&data, 0);
        fileManager->closeFile(fileId);
        return 0;
    }
    return -1;
}

int RecordManager::createFile(const char* filename) {
    if (fileManager->createFile(filename)) {
        return initFile(fileManager, filename);
    }
    return -1;
}

int RecordManager::removeFile(const char* filename) {
    return fileManager->removeFile(filename);
}

int RecordManager::openFile(const char* filename, FileHandler* fileHandler) {
    int fileId;
    if (fileManager->openFile(filename, fileId)) {
        fileHandler->init(fileManager, fileId);
        return 0;
    }
    return -1;
}

int RecordManager::closeFile(FileHandler* fileHandler) {
    return fileManager->closeFile(fileHandler->getFileId());
}
