#ifndef __RMCOMPONENT_H_
#define __RMCOMPONENT_H_
#include "../common.h"
#include <cstdio>
#include <cstring>
#include <stdint.h>

class RecordId{
private:
    int pageId, slotId;
public:
    RecordId(int pageId_, int slotId_) {
        pageId = pageId_;
        slotId = slotId_;
    }

    ~RecordId() {}

    int getpageId() {
        return pageId;
    }

    int getslotId() {
        return slotId;
    }

    void set(int pageId_, int slotId_) {
        pageId = pageId_;
        slotId = slotId_;
    }
};

struct Record{
    uint8_t* data;
};

class RecordList {
    Record record;
    Record* next;
};

struct TableEntry{
    uint8_t colType;
    bool checkConstraint;
    bool primaryKeyConstraint;
    bool foreignKeyConstraint;
    uint32_t colLen;
    uint32_t recordLen;
    char colName[TAB_MAX_NAME_LEN];
    TableEntry* next = nullptr;
    bool hasDefault;
    bool notNullConstraint;
    bool uniqueConstraint;
    bool isNull;
    union {
        int defaultValInt;
        char defaultValVarchar[TAB_MAX_LEN];
        float defaultValFloat;
    };
    /* 
        TODO: implement of checkConstraint and foreignKeyConstraint
     */
    void calcRecordLen();
};

class TableHeader{
private:
    void calcRecordSizeAndLen();
public:
    uint8_t valid;
    uint8_t colNum;
    int16_t firstNotFullPage;
    uint16_t recordLen, totalPageNumber; // recordLen: length of one record
    uint32_t recordSize; // total number of records/slots

    TableEntry* entryHead;
    char tableName[TAB_MAX_NAME_LEN];

    TableHeader();

    ~TableHeader();

    TableEntry* getCol(char* colName);

    void init(TableEntry* entryHead_);

    int alterCol(TableEntry* tableEntry);

    int removeCol(char* colName);

    int addCol(TableEntry* tableEntry);

    bool existCol(char* colName);

    void printInfo();
};

struct PageHeader{
    int16_t nextFreePage;
    int16_t firstEmptySlot; // slot id starts at 0
    int16_t totalSlot;

    PageHeader() {
        nextFreePage = -1;
        firstEmptySlot = -1;
        totalSlot = 0;
    }
};

#endif