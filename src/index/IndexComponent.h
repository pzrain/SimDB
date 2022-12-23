#ifndef __INDEX_COMPONENT_H__
#define __INDEX_COMPONENT_H__
#include "stdint.h"

struct IndexHeader {
    int16_t valid;
    int16_t rootPageId;
    int16_t firstEmptyPage;
    uint16_t totalPageNumber;
    uint16_t indexLen;
    uint16_t colType;

    IndexHeader() {
        valid = 1;
        rootPageId = -1;
        firstEmptyPage = -1;
        totalPageNumber = 0;
        indexLen = -1;
        colType = -1;
    }

    IndexHeader(uint8_t* data) {
        valid = ((int16_t*)data)[0];
        rootPageId = ((int16_t*)data)[1];
        firstEmptyPage = ((int16_t*)data)[2];
        totalPageNumber = ((uint16_t*)data)[3];
        indexLen = ((uint16_t*)data)[4];
        colType = ((uint16_t*)data)[5];
    }

    void init(uint16_t indexLen_, uint8_t colType_) {
        valid = 1;
        rootPageId = 1;
        firstEmptyPage = -1;
        totalPageNumber = 2;
        indexLen = indexLen_;
        colType = colType_;
    }
};

#endif