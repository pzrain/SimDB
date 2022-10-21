#include "FileHandler.h"
#include "assert.h"

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
        tableHeader->firstNotFullPage = -1;
        tableHeader->totalPageNumber = 1;
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
    int16_t pageId = recordId.getPageId();
    int16_t slotId = recordId.getSlotId();

    if (pageId <= 0 || pageId >= tableHeader->totalPageNumber) {
        printf("[ERROR] pageId not found.\n");
        return false;
    }

    int index;
    BufType data = bufPageManager->getPage(fileId, pageId, index);
    PageHeader* pageHeader = (PageHeader*)data;

    if (slotId < 0 || slotId >= pageHeader->totalSlot) {
        printf("[ERROR] slotId not found.\n");
        return false;
    }

    size_t offset;
    offset = sizeof(PageHeader) + slotId * tableHeader->recordLen;
    uint8_t* recordData = (uint8_t*)(((uint8_t*)data)[offset]);    

    if (((int16_t*)recordData)[0] != SLOT_DIRTY) {
        printf("[ERROR] specified slot is empty.\n");
        return false;
    }

    offset = sizeof(int16_t);
    memcpy(record.data, recordData + offset, tableHeader->recordLen);
    return true;
}

bool FileHandler::insertRecord(RecordId &recordId, const Record &record) {
    int16_t pageId = (tableHeader->firstNotFullPage);
    bool newPage = false;

    if (pageId < 0) { // alloc new page
        pageId = tableHeader->totalPageNumber++;
        newPage = true;
    }

    int index, slotId;
    size_t offset;
    BufType data = bufPageManager->getPage(fileId, pageId, index);
    bufPageManager->markDirty(index);
    PageHeader* pageHeader = (PageHeader*)data;
    if (newPage) { // initialize page header
        pageHeader->nextFreePage = tableHeader->firstNotFullPage;
        tableHeader->firstNotFullPage = pageId;
        pageHeader->firstEmptySlot = -1;
        pageHeader->totalSlot = 0;
    }

    slotId = pageHeader->firstEmptySlot;
    if (slotId < 0) { // alloc new slot
        offset = sizeof(PageHeader) + pageHeader->totalSlot * tableHeader->recordLen;
        // initialize "slot head"
        int16_t* writeSlot = (int16_t*)(((uint8_t*)data)[offset]);
        writeSlot[0] = SLOT_LAST_FREE;

        slotId = pageHeader->totalSlot;
        pageHeader->firstEmptySlot = pageHeader->totalSlot++;
    }

    assert(slotId < tableHeader->recordSize);

    offset = sizeof(PageHeader) + slotId * tableHeader->recordLen;
    uint8_t* recordData = (uint8_t*)(((uint8_t*)data)[offset]);
    offset = sizeof(int16_t);
    memcpy(recordData + offset, record.data, tableHeader->recordLen);
    pageHeader->firstEmptySlot = ((uint16_t*)recordData)[0];
    ((int16_t*)recordData)[0] = SLOT_DIRTY; // mark as dirty
    
    recordId.set(pageId, slotId);

    if (pageHeader->totalSlot == tableHeader->recordSize) { // mark page as full
        tableHeader->firstNotFullPage = pageHeader->nextFreePage;
        pageHeader->nextFreePage = -1;
    }
    return true;
}

bool FileHandler::removeRecord(RecordId &recordId) {
    int16_t pageId = recordId.getPageId();
    int16_t slotId = recordId.getSlotId();

    if (pageId <= 0 || pageId >= tableHeader->totalPageNumber) {
        printf("[ERROR] pageId not found.\n");
        return false;
    }

    int index;
    BufType data = bufPageManager->getPage(fileId, pageId, index);
    PageHeader* pageHeader = (PageHeader*)data;

    if (slotId < 0 || slotId >= pageHeader->totalSlot) {
        printf("[ERROR] slotId not found.\n");
        return false;
    }

    size_t offset = sizeof(PageHeader) + slotId * tableHeader->recordLen;
    uint8_t* recordData = (uint8_t*)(((uint8_t*)data)[offset]);

    if (((int16_t*)recordData)[0] == SLOT_DIRTY) {
        ((int16_t*)recordData)[0] = pageHeader->firstEmptySlot;
        pageHeader->firstEmptySlot = slotId;
    } else {
        printf("[INFO] specified slot is already removed.\n");
        return false;
    }

    return true;
}

bool FileHandler::updateRecord(RecordId &recordId, const Record &record) {
    int16_t pageId = recordId.getPageId();
    int16_t slotId = recordId.getSlotId();

    if (pageId <= 0 || pageId >= tableHeader->totalPageNumber) {
        printf("[ERROR] pageId not found.\n");
        return false;
    }

    int index;
    BufType data = bufPageManager->getPage(fileId, pageId, index);
    PageHeader* pageHeader = (PageHeader*)data;

    if (slotId < 0 || slotId >= pageHeader->totalSlot) {
        printf("[ERROR] slotId not found.\n");
        return false;
    }

    size_t offset;
    offset = sizeof(PageHeader) + slotId * tableHeader->recordLen;
    uint8_t* recordData = (uint8_t*)(((uint8_t*)data)[offset]);
    
    if (((int16_t*)recordData)[0] != SLOT_DIRTY) {
        printf("[ERROR] specified slot is empty.\n");
        return false;
    }

    offset = sizeof(int16_t);
    memcpy(recordData + offset, record.data, tableHeader->recordLen);
    return true;
}
