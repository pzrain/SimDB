#include "FileHandler.h"

FileHandler::FileHandler(FileManager* fileManager_, int fileId_) {
    fileManager = fileManager_;
    fileId = fileId_;
}

FileHandler::~FileHandler() {}

int FileHandler::getFileId() {
    return fileId;
}