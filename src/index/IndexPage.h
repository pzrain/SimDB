#ifndef __INDEX_PAGE_H__
#define __INDEX_PAGE_H__
#include "../filesystem/utils/pagedef.h"
#include "../common.h"
#include <stdint.h>

typedef enum {
    PAGE_RAW = 0,
    PAGE_INTERIOR = 1,
    PAGE_LEAF = 2
} IndexPageType;

struct IndexPageHeader{
    uint8_t isInitialized;
    uint8_t pageType;
    uint8_t colType;
    int16_t firstIndex;
    int16_t firstEmptyIndex;
    uint16_t totalIndex;
    uint16_t childIndex;
    uint16_t indexLen;
    union {
        int intContent;
        float floatContent;
        char charContent[TAB_MAX_LEN];
    } maxContent;

    void initialize(uint16_t indexLen_, uint8_t colType_);

    inline uint16_t getTotalLen();
};

class IndexPage{
public:
    uint8_t* data;
    IndexPageHeader* indexPageHeader;
    uint16_t capacity;

    IndexPage(uint8_t* pageData, uint16_t indexLen, uint8_t colType);

    virtual ~IndexPage() = 0;
};

class InteriorPage: public IndexPage{
public:
    ~InteriorPage();
};

class LeafPage: public IndexPage{
public:
    ~LeafPage();
};

#endif