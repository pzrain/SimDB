#ifndef __INDEX_COMPONENT_H__
#define __INDEX_COMPONENT_H__
#include "stdint.h"

struct IndexHeader {
    int16_t valid;
    int16_t rootPageId;
    int16_t firstEmptyPage;
    uint16_t totalPageNumber;

    IndexHeader() {
        valid = 1;
        rootPageId = -1;
        firstEmptyPage = -1;
        totalPageNumber = 0;
    }

    IndexHeader(uint8_t* data) {
        valid = ((int16_t*)data)[0];
        rootPageId = ((int16_t*)data)[1];
        firstEmptyPage = ((int16_t*)data)[2];
        totalPageNumber = ((uint16_t*)data)[3];
    }

    void init() {
        valid = 1;
        rootPageId = 1;
        firstEmptyPage = -1;
        totalPageNumber = 2;
    }
};

#endif