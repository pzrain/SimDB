#ifndef __RMCOMPONENT_H_
#define __RMCOMPONENT_H_
#include "../common.h"
#include <cstdio>
#include <cstring>
#include <stdint.h>

class RecordId{
private:
    int pageNumber, slotNumber;
public:
    RecordId(int pageNumber_, int slotNumber_) {
        pageNumber = pageNumber_;
        slotNumber = slotNumber_;
    }

    ~RecordId() {}

    int getPageNumber() {
        return pageNumber;
    }

    int getSlotNumber() {
        return slotNumber;
    }
};

struct Record{
    char* data;
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
    void calcRecordSize();
public:
    uint8_t valid;
    uint8_t colNum;
    uint32_t recordSize, recordLen;
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

#endif