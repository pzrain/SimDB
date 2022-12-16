#ifndef __RMCOMPONENT_H_
#define __RMCOMPONENT_H_
#include "../common.h"
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include "assert.h"

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

struct RecordDataNode{
    union{
        int* intContent;
        float* floatContent;
        char* charContent;
    }content;
    uint32_t len;
    TB_COL_TYPE nodeType;
    RecordDataNode* next = nullptr;

    ~RecordDataNode() {}
};

class Record;

class RecordData{
public:
    RecordDataNode* head; // attention : you should not delete head manually
    size_t recordLen = 0;

    RecordData() { head = nullptr; }

    RecordData(RecordDataNode* head_): head(head_) {}

    RecordData(const RecordData &other);

    RecordData(int len);
    
    bool serialize(Record& record);

    size_t getLen();

    RecordDataNode* getData(unsigned int i);

    ~RecordData();
};

struct TableEntry{
    uint8_t colType;
    bool checkConstraint;

    CHECK_TYPE* checkType;
    union {
        int* checkInt;
        float* checkFloat;
        // char* checkVarchar;
    } checkContent;
    uint32_t* checkKeyLen;
    uint32_t checkKeyNum;
    
    bool primaryKeyConstraint;
    bool foreignKeyConstraint;
    // new
    // only be used when foreignKeyConstraint is true 
    char foreignKeyTableName[TAB_MAX_NAME_LEN];
    char foreignKeyColName[COL_MAX_NAME_LEN];

    uint32_t colLen; // VARCHAR(%d), int(4), float(4)
    char colName[COL_MAX_NAME_LEN];
    bool hasDefault;
    bool notNullConstraint;
    bool uniqueConstraint;
    bool isNull;
    union {
        int defaultValInt;
        char defaultValVarchar[TAB_MAX_LEN];
        float defaultValFloat;
    } defaultVal;
    int8_t next; // don't forget to build the link 
    

    TableEntry();

    TableEntry(char* colName_, uint8_t colType_, bool checkConstraint_ = false, bool primaryKeyConstraint_ = false, \
               bool foreignKeyConstraint_ = false, uint32_t colLen_ = 0, bool hasDefault_ = false, \
               bool notNullConstraint_ = false, bool uniqueConstraint_ = false, bool isNull_ = false);

    bool verifyConstraint(RecordDataNode* data); // TODO
    // verify checkConstraint, notNullConstraint, (primaryKeyConstraint, UniqueConstraint) (and foreighKeyConstraint ?)
    // on deserialized data
    // return true if succeed else false

    void fillDefault(RecordDataNode* data);
};

struct TableEntryDescNode{
    uint8_t colType;
    uint32_t colLen;
    TableEntryDescNode* next;
};

class TableEntryDesc{
private:
    size_t len = 0;
public:
    TableEntryDescNode* head;

    ~TableEntryDesc();

    size_t getLen();

    TableEntryDescNode* getCol(unsigned int i);
};

class Record{
public:
    uint8_t* data; // The first two byte of data is set to be bitmap
    size_t len;
    uint16_t* bitmap;

    Record(size_t len_) {
        len = len_;
        data = new uint8_t[len_];
        bitmap = (uint16_t*)data;
        *bitmap = 0;
    }

    void clearBitmap() {
        *bitmap = 0;
    }

    void setItemNull(int index) {
        assert(index >= 0 && index < 16);
        *bitmap = (*bitmap) | (1 << index);
    }

    void setItemNotNull(int index) {
        assert(index >= 0 && index < 16);
        if ((*bitmap >> index) & 1) {
            *bitmap = (*bitmap) ^ (1 << index);
        }
    }

    bool isNull(int index) {
        assert(index >= 0 && index < 16);
        return (((*bitmap) >> index) & 1);
    }

    ~Record() {
        delete data;
    }

    bool deserialize(RecordData& rData, TableEntryDesc& tableEntryDesc);
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

    int getCol(const char* colName, TableEntry& tableEntry);

    // only get column id
    int getCol(const char* colName);

    int getCol(int colId, char* colName);
    // get colName according to id, return -1 if fails

    int alterCol(TableEntry* tableEntry);
    // update a column
    // according to the doc, this method may not be called

    int removeCol(char* colName);

    int addCol(TableEntry* tableEntry);

    bool existCol(char* colName);

    void printInfo();

    TableEntryDesc getTableEntryDesc();

    bool verifyConstraint(const RecordData& recordData);

    bool verifyConstraint(Record& record);

    bool fillDefault(RecordData& recordData);

    bool fillDefault(Record& record);

    bool hasPrimaryKey();
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