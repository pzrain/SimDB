#include "BPlusTree.h"


BPlusTree::BPlusTree(int fileId_, BufPageManager* bufpageManager, uint16_t indexLen_, uint8_t colType_) {
    fileId = fileId_;
    indexLen = indexLen_;
    colType = colType_;
    indexHeader = new IndexHeader((uint8_t*)bufPageManger->getPage(fileId, 0, tableIndex));
    if (indexHeader->valid == 0) {
        indexHeader->init();
    }
    BufType data = bufpageManager->getPage(fileId, indexHeader->rootPageId, rootIndex);
    root = new IndexPage((uint8_t*)data, indexLen_, colType_, indexHeader->rootPageId);
}

BPlusTree::~BPlusTree() {
    bufPageManger->writeBack(rootIndex);
    delete root;
}

void BPlusTree::writeIndexTable() {
    bufPageManger->writeBack(tableIndex);
}

int BPlusTree::getNextFreePage() {
    indexHeader->totalPageNumber++;
    int res;
    if (indexHeader->firstEmptyPage >= 0) {
        res = indexHeader->firstEmptyPage;
        int index;
        IndexPageHeader* header = (IndexPageHeader*) bufPageManger->getPage(fileId, indexHeader->firstEmptyPage, index);
        indexHeader->firstEmptyPage = header->nextFreePage;
    } else {
        res = indexHeader->totalPageNumber;
    }
    writeIndexTable();
    return res;
}

inline void BPlusTree::transform(int& val, int pageId, int slotId) {
    val = pageId * root->getCapacity() + slotId;
}

inline void BPlusTree::transformR(int val, int& pageId, int& slotId) {
    pageId = val / root->getCapacity();
    slotId = val % root->getCapacity();
}

void BPlusTree::recycle(std::vector<IndexPage*> &rec, std::vector<int> &pageIndex, bool writeback) {
    int siz = rec.size();
    for (int i = 0; i < siz; i++) {
        if (pageIndex[i] == rootIndex) {
            continue;
        }
        if (writeback) {
            bufPageManger->writeBack(pageIndex[i]);
        }
        delete rec[i];
    }
}

int BPlusTree::searchUpperBound(void* data) {
    IndexPage* cur = root;
    std::vector<IndexPage*> rec;
    std::vector<int> pageIndex;
    while (cur->getPageType() == INDEX_PAGE_INTERIOR) {
        int nextSlot = cur->searchUpperBound(data);
        int16_t childIndex;
        if (nextSlot >= 0) {
            childIndex = *cur->getChildIndex(nextSlot);
        } else {
            childIndex = *cur->getChildIndex(*cur->getLastIndex());
        }
        int index;
        cur = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, childIndex, index)), indexLen, colType, childIndex);
        rec.push_back(cur);
        pageIndex.push_back(index);
    }
    int slotId = cur->searchUpperBound(data);
    int val;
    if (slotId >= 0) {
        transform(val, cur->getPageId(), slotId);
    } else {
        val = -cur->getPageId();
    }
    recycle(rec, pageIndex);
    return val;
}

void BPlusTree::dealOverFlow(IndexPage* indexPage, std::vector<IndexPage*> &rec, std::vector<int> &pageIndex) {
    int lNum = (indexPage->getCapacity() + 1) / 2;
    int rNum = (indexPage->getCapacity() + 1) - lNum;
    int head = indexPage->cut(lNum);
    int newPageId = getNextFreePage(), index;
    IndexPage* newIndexPage = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, newPageId, index)), indexLen, colType, newPageId);
    rec.push_back(newIndexPage);
    pageIndex.push_back(index);

    //TODO
}

void BPlusTree::update(IndexPage* indexPage, void* updateData, std::vector<IndexPage*> &rec, std::vector<int> &pageIndex) {
    int fatherIndex = *(indexPage->getFatherIndex());
    if (fatherIndex < 0) {
        return;
    }
    int pageId, slotId, index;
    transformR(fatherIndex, pageId, slotId);
    IndexPage* fatherIndexPage = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, pageId, index)), indexLen, colType, pageId);
    void* data = fatherIndexPage->getData(slotId);
    memcpy(data, updateData, indexLen);
    rec.push_back(fatherIndexPage);
    pageIndex.push_back(index);
    update(fatherIndexPage, updateData, rec, pageIndex);
}

void BPlusTree::search(void* data, std::vector<int> &res) {
    res.clear();
    int pos = searchUpperBound(data), pageId, slotId;
    int curPageId = -1, index;
    IndexPage* cur = nullptr;
    std::vector<IndexPage*> rec;
    std::vector<int> pageIndex;
    while (pos >= 0) {
        transformR(pos, pageId, slotId);
        if (pageId != curPageId) {
            cur = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, pageId, index)), indexLen, colType, pageId);
            curPageId = pageId;
            rec.push_back(cur);
            pageIndex.push_back(index);
        }
        if (!cur->getCompare()->equ(data, cur->getData(slotId))) {
            break;
        }
        res.push_back(cur->getVal(slotId));
        pos = *(cur->getNextDataIndex(slotId));
    }
    recycle(rec, pageIndex);
}

void BPlusTree::searchBetween(void* ldata, void* rdata, std::vector<int> &res) {
    res.clear();
    if (ldata == nullptr && rdata == nullptr) {
        return;
    }
    int pos, pageId, slotId;
    int curPageId = -1, index;
    IndexPage* cur = nullptr;
    std::vector<IndexPage*> rec;
    std::vector<int> pageIndex;
    if (ldata != nullptr) {
        pos = searchUpperBound(ldata);
        while (pos >= 0) {
            transform(pos, pageId, slotId);
            if (pageId != curPageId) {
                cur = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, pageId, index)), indexLen, colType, pageId);
                curPageId = pageId;
                rec.push_back(cur);
                pageIndex.push_back(index);
            }
            if (rdata && cur->getCompare()->gt(cur->getData(slotId), rdata)) {
                break;
            }
            res.push_back(cur->getVal(slotId));
            pos = *(cur->getNextDataIndex(slotId));
        }
    } else { // rdata != nullptr && ldata == nullptr
        pos = searchUpperBound(rdata);
        transform(pos, pageId, slotId);
        cur = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, pageId, index)), indexLen, colType, pageId);
        curPageId = pageId;
        rec.push_back(cur);
        pageIndex.push_back(index);
        int lastPos = *(cur->getLastDataIndex(slotId));
        while (pos >= 0) {
            transform(pos, pageId, slotId);
            if (pageId != curPageId) {
                cur = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, pageId, index)), indexLen, colType, pageId);
                curPageId = pageId;
                rec.push_back(cur);
                pageIndex.push_back(index);
            }
            if (cur->getCompare()->gt(cur->getData(slotId), rdata)) {
                break;
            }
            res.push_back(cur->getVal(slotId));
            pos = *(cur->getNextDataIndex(slotId));
        }
        pos = lastPos;
        while (pos >= 0) {
            transform(pos, pageId, slotId);
            if (pageId != curPageId) {
                cur = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, pageId, index)), indexLen, colType, pageId);
                curPageId = pageId;
                rec.push_back(cur);
                pageIndex.push_back(index);
            }
            res.push_back(cur->getVal(slotId));
            pos = *(cur->getLastDataIndex(slotId));
        }
    }
    recycle(rec, pageIndex);
}

void BPlusTree::insert(void* data, const int val) {
    std::vector<IndexPage*> rec;
    std::vector<int> pageIndex;
    int pos = searchUpperBound(data), pageId, slotId, index;
    IndexPage* indexPage;
    if (pos < 0) {
        pageId = -pos;   
    } else {
        transformR(pos, pageId, slotId);
    }
    indexPage = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, pageId, index)), indexLen, colType, pageId);
    slotId = indexPage->insert(data, val);
    rec.push_back(indexPage);
    pageIndex.push_back(index);
    if (indexPage->overflow()) {
        dealOverFlow(indexPage, rec, pageIndex);
    } else {
        if (slotId == *(indexPage->getLastIndex())) {
            update(indexPage, data, rec, pageIndex);
        }
    }
    recycle(rec, pageIndex, true);
}
