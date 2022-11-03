#ifndef __INDEX_PAGE_H__
#define __INDEX_PAGE_H__
#include "../filesystem/utils/pagedef.h"
#include "../common.h"
#include "compare.h"
#include <stdint.h>
#include <vector>

typedef enum{
    INDEX_PAGE_INTERIOR = 0,
    INDEX_PAGE_LEAF = 1
} INDEX_PAGE_TYPE;

struct IndexPageHeader{
    uint8_t isInitialized;
    uint8_t colType;
    uint8_t pageType;
    int16_t nextPage; // related to B+ tree
    int16_t lastPage;
    int16_t firstIndex;
    int16_t lastIndex;
    int16_t firstEmptyIndex;
    int16_t nextFreePage;
    uint16_t totalIndex;
    uint16_t indexLen;
    int fatherIndex; // fatherIndex will be built from B+ tree

    void initialize(uint16_t indexLen_, uint8_t colType_);

    inline uint16_t getTotalLen();
};

class IndexPage{
private:
    IndexPageHeader* indexPageHeader;
    Compare* compare;
    uint16_t capacity;
    uint16_t pageId;

public:
    uint8_t* data;

    IndexPage(uint8_t* pageData, uint16_t indexLen, uint8_t colType, uint16_t pageId_);

    ~IndexPage();

    uint16_t getCapacity();

    void changePageType(uint8_t newPageType);

    Compare* getCompare();

    uint8_t getPageType();

    uint16_t getPageId();

    bool overflow();

    bool underflow();

    uint8_t* accessData(int id);

    void* getData(int id);

    int getVal(int id);

    int* getNextDataIndex(int id);

    int* getLastDataIndex(int id);

    int16_t* getNextIndex(int id);

    int16_t* getLastIndex(int id); // return the last index of this slot

    int16_t* getLastIndex(); // return the last index on this pae

    int16_t* getChildIndex(int id);

    int* getFatherIndex();

    int cut(int k);

    int insert(void* data, const int val, const int16_t childIndex_ = -1); // key and value
                                                                           // return the slot index of the inserted value

    void searchEQ(void* data, std::vector<int> &res);

    int searchLowerBound(void* data); // return the uppermost index of slots whose value is no greater than data
                                      // -1 if all value are greater than data
    
    int searchUpperBound(void* data); // similar to searchLowerBound()

    void remove(void* data, std::vector<int> &res);
};

#endif