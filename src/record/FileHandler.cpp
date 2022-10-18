#include "FileHandler.h"

void loadTableHeader(BufType, TableHeader*); // define in "RMComponent.cpp"

void writeTableHeader(BufType, TableHeader*);

FileHandler::FileHandler(FileManager* fileManager_, int fileId_) {
    fileManager = fileManager_;
    fileId = fileId_;
    tableHeader = new TableHeader();

    BufType data;
    fileManager->readPage(fileId, 0, data, 0);
    loadTableHeader(data, tableHeader);
    if (tableHeader->valid == 0) {
        /* 
            TODO: fill in the tableHeader
        */
        writeTableHeader(data, tableHeader);
        fileManager->writePage(fileId, 0, data, 0);
    }
}

FileHandler::~FileHandler() {
    delete tableHeader;
}

int FileHandler::getFileId() {
    return fileId;
}