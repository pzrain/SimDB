#include "IndexPage.h"

void IndexPageHeader::initialize(uint16_t indexLen_, uint8_t colType_) {
    firstIndex = -1;
    firstEmptyIndex = -1;
    totalIndex = 0;
    childIndex = -1;
    indexLen = indexLen_;
    isInitialized = 1;
    pageType = PAGE_LEAF;
    colType = colType_;
}

inline uint16_t IndexPageHeader::getTotalLen() {
    // nextIndex + content + val
    return indexLen + sizeof(int16_t) + sizeof(int);
}

IndexPage::IndexPage(uint8_t* pageData, uint16_t indexLen, uint8_t colType) {
    data = pageData;
    indexPageHeader = (IndexPageHeader*)data;
    if (indexPageHeader->isInitialized == 0) { // uninitialized but not zero?
        indexPageHeader->initialize(indexLen, colType);
    }
    capacity = (PAGE_SIZE - sizeof(IndexPageHeader)) / indexPageHeader->getTotalLen();
}
