#ifndef __INDEX_PAGE_H__
#define __INDEX_PAGE_H__
#include "../filesystem/utils/pagedef.h"
#include "../common.h"
#include "compare.h"
#include <stdint.h>
#include <vector>

struct IndexPageHeader{
    uint8_t isInitialized;
    uint8_t colType;
    int16_t nextPage; // related to B+ tree
    int16_t lastPage;
    int16_t firstIndex;
    int16_t firstEmptyIndex;
    uint16_t totalIndex;
    uint16_t childIndex;
    uint16_t indexLen;
    uint8_t maxContent[TAB_MAX_LEN];

    void initialize(uint16_t indexLen_, uint8_t colType_);

    inline uint16_t getTotalLen();
};

class IndexPage{
private:
    IndexPageHeader* indexPageHeader;
    Compare* compare;
    uint16_t capacity;
public:
    uint8_t* data;

    IndexPage(uint8_t* pageData, uint16_t indexLen, uint8_t colType);

    ~IndexPage();

    uint16_t getCapacity();

    bool overflow();

    bool underflow();

    uint8_t* accessData(int id);

    void* getData(int id);

    int getVal(int id);

    int16_t getNextIndex(int id);

    int16_t getLastIndex(int id);

    int16_t getChildIndex(int id);

    int insert(void* data, const int val, const int16_t childIndex_ = -1); // key and value

    void searchEQ(void* data, std::vector<int> &res);

    int searchLowerBound(void* data); // return the uppermost index of slots whose value is no greater than data
                                      // -1 if all value are greater than data

    int searchUpperBound(void* data);

    void remove(void* data, std::vector<int> &res);
};

#endif