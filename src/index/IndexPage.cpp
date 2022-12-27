#include "IndexPage.h"

void IndexPageHeader::initialize(uint16_t indexLen_, uint8_t colType_) {
    firstIndex = -1;
    lastIndex = -1;
    fatherIndex = -1;
    firstEmptyIndex = -1;
    nextFreePage = -1;
    nextPage = -1;
    lastPage = -1;
    totalIndex = 0;
    indexLen = indexLen_;
    isInitialized = 1;
    colType = colType_;
    pageType = INDEX_PAGE_LEAF;
}

inline uint16_t IndexPageHeader::getTotalLen() {
    // nextIndex + lastIndex + childIndex + content + val
    return 3 * sizeof(int16_t) + indexLen + sizeof(int);
}

IndexPage::IndexPage(uint8_t* pageData, uint16_t indexLen, uint8_t colType, uint16_t pageId_) {
    data = pageData;
    indexPageHeader = (IndexPageHeader*)data;
    if (indexPageHeader->isInitialized != 1) { // uninitialized but not zero?
        indexPageHeader->initialize(indexLen, colType);
    }
    capacity = ((PAGE_SIZE - sizeof(IndexPageHeader)) / indexPageHeader->getTotalLen()) - 2;
    pageId = pageId_;
    switch (colType) {
        case COL_INT:
            compare = new IntCompare();
            break;
        case COL_FLOAT:
            compare = new FloatCompare();
            break;
        case COL_VARCHAR:
            compare = new CharCompare();
            break;
        default:
            compare = nullptr;
            break;
    }
}

IndexPage::~IndexPage() {
    if (compare) {
        delete compare;
    }
}

void IndexPage::initialize(uint16_t indexLen_, uint8_t colType_) {
    indexPageHeader->initialize(indexLen_, colType_);
    capacity = ((PAGE_SIZE - sizeof(IndexPageHeader)) / indexPageHeader->getTotalLen()) - 2;
}

void IndexPage::clear() {
    indexPageHeader->isInitialized = 0;
}

uint16_t IndexPage::getCapacity() {
    return capacity;
}

Compare* IndexPage::getCompare() {
    return compare;
}

void IndexPage::changePageType(uint8_t newPageType) {
    indexPageHeader->pageType = newPageType;
}

uint8_t IndexPage::getPageType() {
    return indexPageHeader->pageType;
}

uint16_t IndexPage::getTotalIndex() {
    return indexPageHeader->totalIndex;
}

uint16_t IndexPage::getPageId() {
    return pageId;
}

bool IndexPage::overflow() {
    return indexPageHeader->totalIndex > capacity;
}

bool IndexPage::underflow() {
    return indexPageHeader->totalIndex < (capacity + 1) / 2;
}

uint8_t* IndexPage::accessData(int id) {
    if (id < 0 || id > capacity) {
        // id == capacity will result in overflow
        printf("[ERROR] fetch slot with wrong id:%d.\n", id);
        return nullptr;
    }
    uint16_t totalLen = indexPageHeader->getTotalLen();
    return data + sizeof(IndexPageHeader) + id * totalLen;
}

void* IndexPage::getData(int id) {
    uint8_t* data = accessData(id);
    return data + 3 * sizeof(int16_t);
}

int* IndexPage::getVal(int id) {
    uint8_t* data = accessData(id);
    return ((int*)(data + 3 * sizeof(int16_t) + indexPageHeader->indexLen));
}

int16_t* IndexPage::getNextIndex(int id) {
    uint8_t* data = accessData(id);
    return ((int16_t*)data);
}

int16_t* IndexPage::getLastIndex(int id) {
    uint8_t* data = accessData(id);
    return ((int16_t*)(data + sizeof(int16_t)));
}

int16_t* IndexPage::getLastIndex() {
    return &(indexPageHeader->lastIndex);
}

int16_t* IndexPage::getFirstIndex() {
    return &(indexPageHeader->firstIndex);
}

int16_t* IndexPage::getChildIndex(int id) {
    uint8_t* data = accessData(id);
    return ((int16_t*)(data + 2 * sizeof(int16_t)));
}

int16_t* IndexPage::getNextPage() {
    return &(indexPageHeader->nextPage);
}

int16_t* IndexPage::getLastPage() {
    return &(indexPageHeader->lastPage);
}

int16_t* IndexPage::getNextFreePage() {
    return &(indexPageHeader->nextFreePage);
}

int16_t* IndexPage::getFirstEmptyIndex() {
    return &(indexPageHeader->firstEmptyIndex);
}

int* IndexPage::getFatherIndex() {
    return &(indexPageHeader->fatherIndex);
}

int IndexPage::cut(int k) {
    if (k < 0 || k > indexPageHeader->totalIndex) {
        printf("[ERROR] cut number should fall between [0, totalIndex].\n");
        return -1;
    }
    int head = indexPageHeader->firstIndex;
    while (k--) {
        head = *(getNextIndex(head));
    }
    return head;
}

int IndexPage::insert(void* data, const int val, const int16_t childIndex_) {
    int16_t head = indexPageHeader->firstIndex, last = -1;
    int16_t emptyIndex = indexPageHeader->firstEmptyIndex;
    uint8_t* emptySlot = nullptr;
    if (emptyIndex < 0) {
        // new empty slot, create and initialize
        emptyIndex = indexPageHeader->totalIndex;
        emptySlot = accessData(emptyIndex);
        ((int16_t*)emptySlot)[0] = -1;
        ((int16_t*)emptySlot)[1] = -1;
    } else {
        emptySlot = accessData(emptyIndex);
        indexPageHeader->firstEmptyIndex = ((int16_t*)emptySlot)[0];
    }
    indexPageHeader->totalIndex++;
    if (head < 0) { // empty page
        indexPageHeader->firstIndex = emptyIndex;
        indexPageHeader->lastIndex = emptyIndex;
    } else {
        int originHead = head;
        while (head >= 0) {
            if (compare->lte(data, getData(head))) {
                break;
            }
            last = head;
            head = *getNextIndex(head);
        }
        if (head == originHead) { // inserted data is the smallest of all
            indexPageHeader->firstIndex = emptyIndex;
        }
        if (head < 0) { // inserted data is the greatest of all
            indexPageHeader->lastIndex = emptyIndex;
        }
    }

    int16_t* nextIndex = (int16_t*)emptySlot;
    int16_t* lastIndex = ((int16_t*)emptySlot) + 1;
    int16_t* childIndex = ((int16_t*)emptySlot) + 2;
    void* emptyData = emptySlot + 3 * sizeof(int16_t);
    int* emptyVal = (int*)(emptySlot + 3 * sizeof(int16_t) + indexPageHeader->indexLen);
    *nextIndex = head;
    *lastIndex = last;
    if (last >= 0) {
        uint8_t* lastSlot = accessData(last);
        ((int16_t*)lastSlot)[0] = emptyIndex;
    }
    if (head >= 0) {
        uint8_t* headSlot = accessData(head);
        ((int16_t*)headSlot)[1] = emptyIndex;
    }
    *childIndex = childIndex_;
    *emptyVal = val;
    memcpy(emptyData, data, indexPageHeader->indexLen);
    return emptyIndex;
}

int IndexPage::insert(std::vector<void*> data, std::vector<int> val, std::vector<int16_t> childIndex, bool front) {
    int siz = data.size();
    if (siz + indexPageHeader->totalIndex > capacity) {
        printf("[ERROR] number of inserted data surpasses capacity.\n");
        return -1;
    }
    if (siz == 0) {
        return 0;
    }
    std::vector<int16_t> emptyIndex;
    for (int i = 0; i < siz; i++) {
        int curIndex = indexPageHeader->firstEmptyIndex;
        if (curIndex < 0) {
            curIndex = indexPageHeader->totalIndex;
            *getNextIndex(curIndex) = -1;
            *getLastIndex(curIndex) = -1;
        } else {
            indexPageHeader->firstEmptyIndex = *getNextIndex(curIndex);
        }
        indexPageHeader->totalIndex++;
        emptyIndex.push_back(curIndex);
    }
    *getLastIndex(emptyIndex[0]) = -1;
    *getNextIndex(emptyIndex[siz - 1]) = -1;
    for (int i = 0; i < siz; i++) {
        if (i > 0) {
            *getLastIndex(emptyIndex[i]) = emptyIndex[i - 1];
        }
        if (i < siz - 1) {
            *getNextIndex(emptyIndex[i]) = emptyIndex[i + 1];
        }
        memcpy(getData(emptyIndex[i]), data[i], indexPageHeader->indexLen);
        *getVal(emptyIndex[i]) = val[i];
        *getChildIndex(emptyIndex[i]) = childIndex[i];
    }
    if (!front) {
        int head = indexPageHeader->firstIndex;
        if (head >= 0) {
            while (*getNextIndex(head) >= 0) {
                head = *getNextIndex(head);
            }
        } else {
            indexPageHeader->firstIndex = emptyIndex[0];
        }
        *getLastIndex(emptyIndex[0]) = head;
        if (head >= 0) {
            *getNextIndex(head) = emptyIndex[0];
        }
        indexPageHeader->lastIndex = emptyIndex[siz - 1];
    } else {
        int head = indexPageHeader->firstIndex;
        *getNextIndex(emptyIndex[siz - 1]) = head;
        if (head >= 0) {
            *getLastIndex(head) = emptyIndex[siz - 1];
        }
        indexPageHeader->firstIndex = emptyIndex[0];
        if (head < 0) {
            indexPageHeader->lastIndex = emptyIndex[siz - 1];
        }
    }
    return 0;
}

void IndexPage::searchEQ(void* data, std::vector<int> &res) {
    int16_t head = indexPageHeader->firstIndex;
    while (head >= 0) {
        if (compare->equ(data, getData(head))) {
            res.push_back(head);
        } else if (compare->lt(data, getData(head))) {
            break;
        }
    }
}

int IndexPage::searchLowerBound(void* data) {
    int16_t head = indexPageHeader->firstIndex, last = -1;
    while (head >= 0) {
        if (compare->lt(data, getData(head))) {
            break;
        }
        last = head;
        head = *getNextIndex(head);
    }
    return last;
}

int IndexPage::searchUpperBound(void* data) {
    int16_t head = indexPageHeader->firstIndex;
    while (head >= 0) {
        if (compare->lte(data, getData(head))) {
            break;
        }
        head = *getNextIndex(head);
    }
    return head;
}

void IndexPage::remove(void* data, std::vector<int> &res, int cnt) {
    int16_t head = indexPageHeader->firstIndex, last = -1;
    while (head >= 0) {
        int next = *getNextIndex(head);
        if (cnt-- == 0) {
            break;
        }
        if (compare->equ(data, getData(head))) {
            res.push_back(head);
            if (head == indexPageHeader->lastIndex) { // remove last index
                indexPageHeader->lastIndex = *getLastIndex(indexPageHeader->lastIndex);
            }
            if (last >= 0) {
                uint8_t* lastData = accessData(last);
                ((int16_t*)lastData)[0] = next;
            }
            if (next >= 0) {
                uint8_t* nextData = accessData(next);
                ((int16_t*)(nextData + sizeof(int16_t)))[0] = last;
            }
            uint8_t* emptyData = accessData(head);
            // set empty index link
            ((int16_t*)emptyData)[0] = indexPageHeader->firstEmptyIndex;
            indexPageHeader->firstEmptyIndex = head;

            indexPageHeader->totalIndex--;
            head = next;
        } else {
            last = head;
            head = next;
        }
    }
}

void IndexPage::removeSlot(int slotId) {
    int16_t* lastIndex = getLastIndex(slotId);
    int16_t* nextIndex = getNextIndex(slotId);
    if (*lastIndex >= 0) {
        *getNextIndex(*lastIndex) = *nextIndex;
    } else {
        indexPageHeader->firstIndex = *nextIndex;
    }
    if (*nextIndex >= 0) {
        *getLastIndex(*nextIndex) = *lastIndex;
    } else {
        indexPageHeader->lastIndex;
    }
    *getNextIndex(slotId) = indexPageHeader->firstEmptyIndex;
    indexPageHeader->firstEmptyIndex = slotId;
    indexPageHeader->totalIndex--;
}

void IndexPage::removeFrom(int16_t index, std::vector<void*>& removeData, std::vector<int>& removeVal, std::vector<int16_t>& removeChildIndex) {
    removeData.clear();
    removeVal.clear();
    removeChildIndex.clear();
    indexPageHeader->lastIndex = *(getLastIndex(index));
    if (indexPageHeader->lastIndex >= 0) {
        int16_t* lastSlotNextIndex = getNextIndex(indexPageHeader->lastIndex);
        *lastSlotNextIndex = -1;
    } else {
        indexPageHeader->firstIndex = -1;
    }
    while (index >= 0) {
        void* data = getData(index);
        int16_t childIndex = *getChildIndex(index);
        int val = *getVal(index);
        int16_t* nextIndex = getNextIndex(index);
        int temp = *nextIndex;
        *nextIndex = indexPageHeader->firstEmptyIndex;
        indexPageHeader->firstEmptyIndex = index;
        indexPageHeader->totalIndex--;
        removeData.push_back(data);
        removeVal.push_back(val);
        removeChildIndex.push_back(childIndex);
        index = temp;
    }
}

void IndexPage::insertFrom(const std::vector<void*> insertData, const std::vector<int> insertVal, const std::vector<int16_t> insertChildIndex) {
    int siz = insertData.size();
    for (int i = 0; i < siz; i++) {
        int16_t* nextIndex = getNextIndex(i);
        int16_t* lastIndex = getLastIndex(i);
        int16_t* childIndex = getChildIndex(i);
        void* data = getData(i);
        int* val = getVal(i);
        *nextIndex = (i + 1 == siz) ? -1 : (i + 1);
        *lastIndex = i - 1;
        *childIndex = insertChildIndex[i];
        memcpy(data, insertData[i], indexPageHeader->indexLen);
        *val = insertVal[i];
    }
    indexPageHeader->firstIndex = 0;
    indexPageHeader->lastIndex = siz - 1;
    indexPageHeader->totalIndex = siz;
}
