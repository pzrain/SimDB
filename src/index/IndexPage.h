#ifndef __INDEX_PAGE_H__
#define __INDEX_PAGE_H__
#include "../filesystem/utils/pagedef.h"
#include "../common.h"
#include "Compare.h"
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

    void initialize(uint16_t indexLen_, uint8_t colType);

    void clear();

    uint16_t getCapacity();

    void changePageType(uint8_t newPageType);

    Compare* getCompare();

    uint8_t getPageType();

    uint16_t getTotalIndex();

    uint16_t getPageId();

    bool overflow();

    bool underflow();

    uint8_t* accessData(int id); // get one item accordin to specified slotId

    void* getData(int id); // get true data

    int* getVal(int id);

    int16_t* getNextIndex(int id);

    int16_t* getLastIndex(int id); // return the last index of this slot

    int16_t* getLastIndex(); // return the last index on this page

    int16_t* getFirstIndex();

    int16_t* getChildIndex(int id);

    int16_t* getNextPage();

    int16_t* getLastPage();

    int16_t* getNextFreePage();

    int16_t* getFirstEmptyIndex();

    int* getFatherIndex();

    int cut(int k);

    int insert(void* data, const int val, const int16_t childIndex_ = -1, const int16_t lastIndex_ = -1); // key and value
                                                                           // return the slot index of the inserted value

    int insert(std::vector<void*> data, std::vector<int> val, std::vector<int16_t> childIndex, bool front);
    // return 0 if all the data can fit in this page
    // otherwise -1, and no data will be inserted
    // attention: data should be aranged in no descending order
    //            those data will be inserted right in the end of this page(front = false)
    //                                           or in the front of this page(front = true)

    void searchEQ(void* data, std::vector<int> &res);

    int searchLowerBound(void* data); // return the uppermost index of slots whose value is no greater than data
                                      // -1 if all value are greater than data
    
    int searchUpperBound(void* data); // similar to searchLowerBound()

    void remove(void* data, std::vector<int> &res, int cnt = -1);
    // remove data from page
    // cnt defines the amount of data you want to remove. to remove all, set cnt to -1

    void removeSlot(int slotId);
    // remove data of the specified slot, return 0 if succeed, -1 if fail
    // attention: this method will do extra test(like if the slot is empty or not)
    //            SO CAREFULLY CALL!

    void removeFrom(int16_t index, std::vector<void*>& removeData, std::vector<int>& removeVal, std::vector<int16_t>& removeChildIndex);
    // attention: void* in removeData will only be temporary valid, the value it point to may be overwrite!
    // this interface can only be used in spliting a page, afterwards writing part of its content to another page

    void insertFrom(const std::vector<void*> insertData, const std::vector<int> val, const std::vector<int16_t> insertChildIndex);
    // attention: insertData and insertVal should come directly from removeData and removeVal in removeFrom, respectively
    // this interface can only be used in spliting a page, afterwards writing part of its content to another page
};

#endif