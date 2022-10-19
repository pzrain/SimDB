#include "FileHandler.h"

void loadTableHeaderFromBuff(BufType&, TableHeader*); // define in "RMComponent.cpp"

void writeTableHeaderToBuff(BufType&, TableHeader*);

int loadTableHeader(BufPageManager* bufPageManager, int fileId, TableHeader* tableHeader) {
    BufType data;
    int index;
    data = bufPageManager->getPage(fileId, 0, index);
    loadTableHeaderFromBuff(data, tableHeader);
    return 0;
}

int writeTableHeader(BufPageManager* bufPageManager, int fileId, TableHeader* tableHeader) {
    BufType data;
    int index;
    data = bufPageManager->getPage(fileId, 0, index);
    writeTableHeaderToBuff(data, tableHeader);
    bufPageManager->markDirty(index);
    return 0;
}

FileHandler::FileHandler() {
    bufPageManager = nullptr;
    tableHeader = new TableHeader();
    fileId = -1;
}

void FileHandler::init(BufPageManager* bufPageManager_, int fileId_, const char* filename) {
    bufPageManager = bufPageManager_;
    fileId = fileId_;
    if (loadTableHeader(bufPageManager, fileId, tableHeader) != 0) {
        printf("[Error] fail to load table header");
        return;
    }
    if (tableHeader->valid == 0) {
        // init the tableHeader
        tableHeader->colNum = 0;
        tableHeader->recordSize = 0;
        tableHeader->recordLen = 0;
        tableHeader->entryHead = nullptr;
        strcpy(tableHeader->tableName, filename);
        tableHeader->valid = 1;
        if (writeTableHeader(bufPageManager, fileId, tableHeader) != 0) {
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
    int res = 0;
    switch (opCode) {
        case TB_INIT:
            tableHeader->init(tableEntry);
            break;
        case TB_ALTER:
            res = tableHeader->alterCol(tableEntry);
            break;
        case TB_REMOVE:
            res = tableHeader->removeCol(colName);
            break;
        case TB_ADD:
            res = tableHeader->addCol(tableEntry);
            break;
        case TB_EXIST:
            res = tableHeader->existCol(colName) ? 1 : 0;
            return res;
        case TB_PRINT:
            tableHeader->printInfo();
            return 0;
        default:
            printf("[Error] unknown op code.\n");
            return -1;
    }
    if (res == 0) {
        return writeTableHeader(bufPageManager, fileId, tableHeader);
    }
    return -1;
}

bool FileHandler::getRecord(RecordId recordId, Record &record) {

}
