#ifndef __RMCOMPONENT_H_
#define __RMCOMPONENT_H_
#include "../common.h"
#include <cstdio>
#include <cstring>
#include <stdint.h>

class Record{
private:
    int pageNumber, slotNumber;
public:
    Record(int pageNumber_, int slotNumber_) {
        pageNumber = pageNumber_;
        slotNumber = slotNumber_;
    }

    ~Record() {}

    int getPageNumber() {
        return pageNumber;
    }

    int getSlotNumber() {
        return slotNumber;
    }
};

struct TableEntry{
    uint8_t colType;
    bool checkConstraint;
    bool primaryKeyConstraint;
    bool foreignKeyConstraint;
    uint32_t colLen;
    char colName[TAB_MAX_LEN];
    TableEntry* next = nullptr;
    union {
        int defaultValInt;
        char defaultValVarchar[TAB_MAX_LEN];
        float defaultValFloat;
    };
    bool hasDefault;
    bool notNullConstraint;
    bool uniqueConstraint;
    /* 
        TODO: implement of checkConstraint and foreignKeyConstraint
     */
};

class TableHeader{
public:
    uint8_t valid;
    uint8_t colNum;
    uint32_t recordSize;
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