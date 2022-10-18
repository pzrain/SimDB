#include "FileHandler.h"

void loadTableHeaderFromBuff(BufType, TableHeader*); // define in "RMComponent.cpp"

void writeTableHeaderToBuff(BufType, TableHeader*);

int loadTableHeader(FileManager* fileManager, int fileId, TableHeader* tableHeader) {
    BufType data;
    if (fileManager->readPage(fileId, 0, data, 0)) {
        loadTableHeaderFromBuff(data, tableHeader);
        return 0;
    }
    return -1;
}

int writeTableHeader(FileManager* fileManager, int fileId, TableHeader* tableHeader) {
    BufType data;
    writeTableHeaderToBuff(data, tableHeader);
    return fileManager->writePage(fileId, 0, data, 0);
}

FileHandler::FileHandler() {
    fileManager = nullptr;
    tableHeader = new TableHeader();
    fileId = -1;
}

void FileHandler::init(FileManager* fileManager_, int fileId_) {
    fileManager = fileManager_;
    fileId = fileId_;
    if (loadTableHeader(fileManager, fileId, tableHeader) != 0) {
        printf("[Error] fail to load table header");
        return;
    }
    if (tableHeader->valid == 0) {
        // init the tableHeader
        tableHeader->colNum = 0;
        tableHeader->recordSize = 0;
        tableHeader->entryHead = nullptr;
        tableHeader->valid = 1;
        if (writeTableHeader(fileManager, fileId, tableHeader) != 0) {
            printf("[Error] fail to write table header.\n");
        }
    }
}

FileHandler::~FileHandler() {
    delete tableHeader;
}

int FileHandler::getFileId() {
    return fileId;
}

int FileHandler::operateTable(TB_OP_TYPE opCode, char* colName = nullptr, TableEntry* tableEntry = nullptr) {
    if (tableHeader->valid == 0) {
        printf("[Error] the table has not been initialized yet.\n");
        return -1;
    }
    switch (opCode) {
        case TB_INIT:
            tableHeader->init(tableEntry);
            break;
        case TB_ALTER:
            return tableHeader->alterCol(tableEntry);
            break;
        case TB_REMOVE:
            return tableHeader->removeCol(colName);
            break;
        case TB_ADD:
            return tableHeader->addCol(tableEntry);
            break;
        case TB_EXIST:
            return tableHeader->existCol(colName) ? 1 : 0;
            break;
        case TB_PRINT:
            tableHeader->printInfo();
            break;
        default:
            printf("[Error] unknown op code.\n");
            return -1;
    }
    return 0;
}
