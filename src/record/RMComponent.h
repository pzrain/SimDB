#ifndef __RMCOMPONENT_H_
#define __RMCOMPONENT_H_
#include "../common.h"
#include <cstdio>
#include <cstring>
#include <stdint.h>

class RecordId{
private:
    int16_t pageId, slotId;
public:
    RecordId() {}

    RecordId(int16_t pageId_, int16_t slotId_) {
        pageId = pageId_;
        slotId = slotId_;
    }

    ~RecordId() {}

    int16_t getPageId() {
        return pageId;
    }

    int16_t getSlotId() {
        return slotId;
    }

    void set(int16_t pageId_, int16_t slotId_) {
        pageId = pageId_;
        slotId = slotId_;
    }
};

struct Record{
    uint8_t* data;

    Record(size_t len) {
        data = new uint8_t[len];
    }

    ~Record() {
        delete data;
    }
};

struct TableEntry{
    uint8_t colType;
    bool checkConstraint;
    bool primaryKeyConstraint;
    bool foreignKeyConstraint;
    uint32_t colLen; // VARCHAR(%d), int(4), float(4)
    char colName[TAB_MAX_NAME_LEN];
    bool hasDefault;
    bool notNullConstraint;
    bool uniqueConstraint;
    bool isNull;
    union {
        int defaultValInt;
        char defaultValVarchar[TAB_MAX_LEN];
        float defaultValFloat;
    };
    int8_t next;
    /* 
        TODO: implement of checkConstraint and foreignKeyConstraint
     */

    TableEntry();

    TableEntry(char* colName_, uint8_t colType_, bool checkConstraint_ = false, bool primaryKeyConstraint_ = false, \
               bool foreignKeyConstraint_ = false, uint32_t colLen_ = 0, bool hasDefault_ = false, \
               bool notNullConstraint_ = false, bool uniqueConstraint_ = false, bool isNull_ = false);
};

class TableHeader{
private:
    void calcRecordSizeAndLen();

    int findFreeCol();

public:
    uint8_t valid;
    uint8_t colNum;
    int8_t entryHead;
    int16_t firstNotFullPage;
    uint16_t recordLen, totalPageNumber; // recordLen: length of one record
    uint32_t recordSize; // number of records/slots on one page
    uint32_t recordNum; // total number of records;
    TableEntry entrys[TAB_MAX_COL_NUM];
    char tableName[TAB_MAX_NAME_LEN];

    TableHeader();

    void init(TableEntry* entryHead_, int num);
    // num is the length of the TableEntry array
    // Attention: init will not check same column name, thus should only be called to initial a tableHeader

    int getCol(char* colName, TableEntry& tableEntry);

    int alterCol(TableEntry* tableEntry);

    int removeCol(char* colName);

    int addCol(TableEntry* tableEntry);

    bool existCol(char* colName);

    void printInfo();
};

typedef enum{
    SLOT_LAST_FREE = -1,
    SLOT_DIRTY = -2
} SLOT_TYPE;

struct PageHeader{
    int16_t nextFreePage;
    int16_t firstEmptySlot; // slot id starts at 0
    int16_t totalSlot;
    int16_t maximumSlot;

    PageHeader() {
        nextFreePage = -1;
        firstEmptySlot = -1;
        totalSlot = 0;
        maximumSlot = -1;
    }
};

#endif