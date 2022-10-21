#include "RecordManager.h"
#include <cstdio>

// Section for Record Manager

RecordManager::RecordManager(BufPageManager* bufPageManager_) {
    bufPageManager = bufPageManager_;
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

int RecordManager::createFile(const char* filename) {
    if (bufPageManager->fileManager->createFile(filename)) {
        return initFile(bufPageManager, filename);
    }
    return -1;
}

int RecordManager::removeFile(const char* filename) {
    return bufPageManager->fileManager->removeFile(filename);
}

int RecordManager::openFile(const char* filename, FileHandler* fileHandler) {
    int fileId;
    if (bufPageManager->fileManager->openFile(filename, fileId)) {
        fileHandler->init(bufPageManager, fileId, filename);
        return 0;
    }
    return -1;
}

int RecordManager::closeFile(FileHandler* fileHandler) {
    return bufPageManager->fileManager->closeFile(fileHandler->getFileId());
}
