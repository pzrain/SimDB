#include "FileHandler.h"
#include "assert.h"

void loadTableHeaderFromBuff(BufType &dataLoad, TableHeader* tableHeader) {
    memcpy((uint8_t*)tableHeader, (uint8_t*)dataLoad, sizeof(TableHeader));
    // tableHeader = (TableHeader*)((uint8_t*)dataLoad[sizeof(TableHeader)]);
}

void writeTableHeaderToBuff(BufType &dataLoad, TableHeader* tableHeader) {
    memcpy((uint8_t*)dataLoad, (uint8_t*)tableHeader, sizeof(TableHeader));
}

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

void FileHandler::init(BufPageManager* bufPageManager_, int fileId_, const char* tableName_) {
    bufPageManager = bufPageManager_;
    fileId = fileId_;
    strcpy(tableName, tableName_);
    if (loadTableHeader(bufPageManager, fileId, tableHeader) != 0) {
        printf("[ERROR] fail to load table header");
        return;
    }
    if (tableHeader->valid == 0) {
        // init the tableHeader
        tableHeader->colNum = 0;
        tableHeader->recordSize = 0;
        tableHeader->firstNotFullPage = -1;
        tableHeader->totalPageNumber = 1;
        tableHeader->recordLen = 0;
        tableHeader->entryHead = -1;
        strcpy(tableHeader->tableName, tableName_);
        tableHeader->valid = 1;
        if (writeTableHeader(bufPageManager, fileId, tableHeader) != 0) {
            printf("[ERROR] fail to write table header.\n");
        }
    }
}

FileHandler::~FileHandler() {
    delete tableHeader;
}

int FileHandler::getFileId() {
    return fileId;
}

char* FileHandler::getTableName() {
    return tableName;
}

int FileHandler::operateTable(TB_OP_TYPE opCode, char* colName, TableEntry* tableEntry, int num) {
    if (tableHeader->valid == 0) {
        printf("[ERROR] the table has not been initialized yet.\n");
        return -1;
    }
    if (opCode == TB_INIT || opCode == TB_ALTER || opCode == TB_ADD) {
        if (tableEntry == nullptr) {
            printf("[ERROR] tableEntry should not be null.\n");
            return -1;
        }
    }
    if (opCode == TB_REMOVE || opCode == TB_EXIST) {
        if (colName == nullptr) {
            printf("[ERROR] colName should not be null.\n");
            return -1;
        }
    }

    int res = 0;
    switch (opCode) {
        case TB_INIT:
            tableHeader->init(tableEntry, num);
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
            printf("[ERROR] unknown op code.\n");
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

    if (slotId < 0 || slotId > pageHeader->maximumSlot) {
        printf("[ERROR] slotId not found.\n");
        return false;
    }

    size_t offset;
    offset = sizeof(PageHeader) + slotId * tableHeader->recordLen;
    uint8_t* recordData = (uint8_t*)(((uint8_t*)data) + offset);    

    if (((int16_t*)recordData)[0] != SLOT_DIRTY) {
        printf("[ERROR] specified slot is empty.\n");
        return false;
    }

    offset = sizeof(int16_t);
    memcpy(record.data, recordData + offset, tableHeader->recordLen - sizeof(int16_t));
    return true;
}

bool FileHandler::insertRecord(RecordId &recordId, Record &record) {
    if (!tableHeader->fillDefault(record)) {
        printf("[ERROR] fail to fill default value to record.\n");
        return false;
    }
    if (!tableHeader->verifyConstraint(record)) {
        printf("[ERROR] check constraint on record fails.\n");
        return false;
    }
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
        pageHeader->maximumSlot = -1;
    }

    slotId = pageHeader->firstEmptySlot;
    if (slotId < 0) { // alloc new slot
        offset = sizeof(PageHeader) + pageHeader->totalSlot * tableHeader->recordLen;
        // initialize "slot head"
        int16_t* writeSlot = (int16_t*)(((uint8_t*)data) + offset);
        writeSlot[0] = SLOT_LAST_FREE;

        slotId = pageHeader->totalSlot;
        pageHeader->firstEmptySlot = pageHeader->totalSlot++;
    }

    assert(slotId < tableHeader->recordSize);

    offset = sizeof(PageHeader) + slotId * tableHeader->recordLen;
    uint8_t* recordData = (uint8_t*)(((uint8_t*)data) + offset);
    offset = sizeof(int16_t);
    memcpy(recordData + offset, record.data, tableHeader->recordLen - sizeof(int16_t));
    pageHeader->firstEmptySlot = ((int16_t*)recordData)[0];
    ((int16_t*)recordData)[0] = SLOT_DIRTY; // mark as dirty
    
    recordId.set(pageId, slotId);

    if (pageHeader->totalSlot == tableHeader->recordSize) { // mark page as full
        tableHeader->firstNotFullPage = pageHeader->nextFreePage;
        pageHeader->nextFreePage = -1;
    }
    if (slotId > pageHeader->maximumSlot) {
        pageHeader->maximumSlot = slotId;
    }
    tableHeader->recordNum++;
    writeTableHeader(bufPageManager, fileId, tableHeader);
    return true;
}

bool FileHandler::removeRecord(RecordId &recordId, Record &record) {
    int16_t pageId = recordId.getPageId();
    int16_t slotId = recordId.getSlotId();

    if (pageId <= 0 || pageId >= tableHeader->totalPageNumber) {
        printf("[ERROR] pageId not found.\n");
        return false;
    }

    int index;
    BufType data = bufPageManager->getPage(fileId, pageId, index);
    PageHeader* pageHeader = (PageHeader*)data;

    if (slotId < 0 || slotId > pageHeader->maximumSlot) {
        printf("[ERROR] slotId not found.\n");
        return false;
    }

    size_t offset = sizeof(PageHeader) + slotId * tableHeader->recordLen;
    uint8_t* recordData = (uint8_t*)(((uint8_t*)data) + offset);
    if (((int16_t*)recordData)[0] == SLOT_DIRTY) {
        offset = sizeof(int16_t);
        memcpy(record.data, recordData + offset, tableHeader->recordLen - offset);
        ((int16_t*)recordData)[0] = pageHeader->firstEmptySlot;
        pageHeader->firstEmptySlot = slotId;
    } else {
        printf("[INFO] specified slot is already removed.\n");
        return false;
    }
    tableHeader->recordNum--;
    writeTableHeader(bufPageManager, fileId, tableHeader);
    return true;
}

bool FileHandler::updateRecord(RecordId &recordId, Record &record) {
    if (!tableHeader->fillDefault(record)) {
        printf("[ERROR] fail to fill default value to record.\n");
        return false;
    }
    if (!tableHeader->verifyConstraint(record)) {
        printf("[ERROR] check constraint on record fails.\n");
        return false;
    }
    int16_t pageId = recordId.getPageId();
    int16_t slotId = recordId.getSlotId();

    if (pageId <= 0 || pageId >= tableHeader->totalPageNumber) {
        printf("[ERROR] pageId not found.\n");
        return false;
    }

    int index;
    BufType data = bufPageManager->getPage(fileId, pageId, index);
    PageHeader* pageHeader = (PageHeader*)data;

    if (slotId < 0 || slotId > pageHeader->maximumSlot) {
        printf("[ERROR] slotId not found.\n");
        return false;
    }

    size_t offset;
    offset = sizeof(PageHeader) + slotId * tableHeader->recordLen;
    uint8_t* recordData = (uint8_t*)(((uint8_t*)data) + offset);
    
    if (((int16_t*)recordData)[0] != SLOT_DIRTY) {
        printf("[ERROR] specified slot is empty.\n");
        return false;
    }

    offset = sizeof(int16_t);
    memcpy(recordData + offset, record.data, tableHeader->recordLen - sizeof(int16_t));
    
    writeTableHeader(bufPageManager, fileId, tableHeader);
    return true;
}

void FileHandler::getAllRecords(std::vector<Record*>& records, std::vector<RecordId*>& recordIds) {
    int cnt = 0;
    for (int pageId = 1; pageId < tableHeader->totalPageNumber; pageId++) {
        int index;
        BufType data = bufPageManager->getPage(fileId, pageId, index);
        PageHeader* pageHeader = (PageHeader*)data;
        size_t offset = sizeof(PageHeader);
        for (int slotId = 0; slotId <= pageHeader->maximumSlot; slotId++) {
            uint8_t* recordData = (uint8_t*)(((uint8_t*)data) + offset);
            offset += tableHeader->recordLen;
            if (((int16_t*)recordData)[0] != SLOT_DIRTY) {
                continue;
            }
            records.push_back(new Record(tableHeader->recordLen - sizeof(int16_t)));
            recordIds.push_back(new RecordId(pageId, slotId));
            memcpy(records[cnt++]->data, recordData + sizeof(int16_t), tableHeader->recordLen - sizeof(int16_t));
        }
    }
}

bool FileHandler::getAllRecordsAccordingToFields(std::vector<Record*>& records, std::vector<RecordId*>& recordIds, const uint16_t enable) {
    std::vector<int> entryLen, entryIndex;
    int len = sizeof(int16_t) + sizeof(uint16_t), totalLen = sizeof(uint16_t);
    int8_t head = tableHeader->entryHead;
    for (int i = 0; i < tableHeader->colNum; i++) {
        while (i < tableHeader->colNum && (enable & (1 << i)) == 0) {
            len += tableHeader->entrys[head].colLen;
            head = tableHeader->entrys[head].next;
            i++;
        }
        entryLen.push_back(len);
        len = 0;
        if (i == tableHeader->colNum) {
            break;
        }
        // enable & (1 << i) == 1
        totalLen += tableHeader->entrys[head].colLen;
        entryLen.push_back(tableHeader->entrys[head].colLen);
        entryIndex.push_back(i);
        head = tableHeader->entrys[head].next;
    }
    if (entryIndex.size() == 0) {
        printf("[INFO] there's nothing to get with the specific fields.\n");
        return false;
    }

    int cnt = 0;
    for (int pageId = 1; pageId < tableHeader->totalPageNumber; pageId++) {
        int index;
        BufType data = bufPageManager->getPage(fileId, pageId, index);
        PageHeader* pageHeader = (PageHeader*)data;
        size_t offset = sizeof(PageHeader);
        for (int slotId = 0; slotId <= pageHeader->maximumSlot; slotId++) {
            uint8_t* recordData = (uint8_t*)(((uint8_t*)data) + offset);
            offset += tableHeader->recordLen;
            if (((int16_t*)recordData)[0] != SLOT_DIRTY) {
                continue;
            }
            uint16_t originBitmap = ((uint16_t*)recordData)[1];
            uint16_t bitmap = 0;
            int siz = entryIndex.size();
            for (int i = 0; i < siz; i++) {
                bitmap |= (((originBitmap & (1 << entryIndex[i])) >> entryIndex[i]) << i);
            }
            records.push_back(new Record(totalLen));
            recordIds.push_back(new RecordId(pageId, slotId));
            memcpy(records[cnt]->data, &bitmap, sizeof(uint16_t)); // write bitmap first
            size_t offsetRecord = sizeof(uint16_t), offsetRecordData = 0;
            siz = entryLen.size();
            for (int i = 0; i < siz; i += 2) {
                offsetRecordData += entryLen[i];
                memcpy((records[cnt]->data) + offsetRecord, recordData + offsetRecordData, entryLen[i + 1]);
                offsetRecord += entryLen[i + 1];
                offsetRecordData += entryLen[i + 1];
            }
            cnt++;
        }
    }
    return true;
}

bool FileHandler::insertAllRecords(const std::vector<Record*>& records, std::vector<RecordId>& recordIds) {
    recordIds.clear();
    for (Record* record : records) {
        if (!tableHeader->fillDefault(*record)) {
            printf("[ERROR] fail to fill default value to record.\n");
            return false;
        }
        if (!tableHeader->verifyConstraint(*record)) {
            printf("[ERROR] check constraint on record fails.\n");
            return false;
        }
    }

    int total = records.size(), done = 0;
    while (done < total) {
        int16_t pageId = (tableHeader->firstNotFullPage);
        bool newPage = false;
        if (pageId < 0) {
            pageId = tableHeader->totalPageNumber++;
            newPage = true;
        }
        int index, slotId;
        size_t offset;
        BufType data = bufPageManager->getPage(fileId, pageId, index);
        bufPageManager->markDirty(index);
        PageHeader* pageHeader = (PageHeader*)data;
        if (newPage) {
            pageHeader->nextFreePage = tableHeader->firstNotFullPage;
            tableHeader->firstNotFullPage = pageId;
            pageHeader->firstEmptySlot = -1;
            pageHeader->totalSlot = 0;
            pageHeader->maximumSlot = -1;
        }
        int num = tableHeader->recordSize - pageHeader->totalSlot;
        if (num > total - done) {
            num = total - done;
        }
        while (num--) {
            slotId = pageHeader->firstEmptySlot;
            if (slotId < 0) {
                offset = sizeof(PageHeader) + pageHeader->totalSlot * tableHeader->recordLen;
                int16_t* writeSlot = (int16_t*)(((uint8_t*)data) + offset);
                writeSlot[0] = SLOT_LAST_FREE;
                slotId = pageHeader->totalSlot;
                pageHeader->firstEmptySlot = pageHeader->totalSlot++;
            } else {
                offset = sizeof(PageHeader) + slotId * tableHeader->recordLen;
            }
            uint8_t* recordData = (uint8_t*)(((uint8_t*)data) + offset);
            offset = sizeof(int16_t);
            memcpy(recordData + offset, records[done++]->data, tableHeader->recordLen - sizeof(int16_t));
            recordIds.push_back(RecordId(pageId, slotId));
            pageHeader->firstEmptySlot = ((int16_t*)recordData)[0];
            ((int16_t*)recordData)[0] = SLOT_DIRTY;
            if (slotId > pageHeader->maximumSlot) {
                pageHeader->maximumSlot = slotId;
            }
        }
    }
    tableHeader->recordNum += total;
    writeTableHeader(bufPageManager, fileId, tableHeader);
    return true;
}

int FileHandler::getRecordNum() {
    return tableHeader->recordNum;
}

size_t FileHandler::getRecordLen() {
    return tableHeader->recordLen - sizeof(int16_t);
}

TableEntryDesc FileHandler::getTableEntryDesc() {
    return tableHeader->getTableEntryDesc();
}

TableHeader* FileHandler::getTableHeader() {
    return tableHeader;
}

void FileHandler::renameTable(const char* newName) {
    memcpy(tableName, newName, strlen(newName));
    memcpy(tableHeader->tableName, newName, strlen(newName));
    writeTableHeader(bufPageManager, fileId, tableHeader);
}
