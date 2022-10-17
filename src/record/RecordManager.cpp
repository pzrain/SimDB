#include "RecordManager.h"
#include <cstdio>

// Section for Record Manager

RecordManager::RecordManager(FileManager* fileManager_) {
    fileManager = fileManager_;
}

RecordManager::~RecordManager() {}

void RecordManager::createFile(const char* filename) {
    fileManager->createFile(filename);
}

void RecordManager::removeFile(const char* filename) {
    fileManager->removeFile(filename);
}

void RecordManager::openFile(const char* filename, FileHandler* fileHandler) {
    int fileId;
    if (fileManager->openFile(filename, fileId)) {
        fileHandler->init(fileManager, fileId);
    } else {
        printf("Error in opening file %s\n", filename);
    }
}

void RecordManager::closeFile(FileHandler* fileHandler) {
    fileManager->closeFile(fileHandler->getFileId());
}
