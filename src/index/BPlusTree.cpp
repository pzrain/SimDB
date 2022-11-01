#include "BPlusTree.h"

BPlusTree::BPlusTree(int fileId_, BufPageManager* bufpageManager, int pageId, uint16_t indexLen_, uint8_t colType_) {
    fileId = fileId_;
    indexLen = indexLen_;
    colType = colType_;
    BufType data = bufpageManager->getPage(fileId, pageId, rootIndex);
    root = new IndexPage((uint8_t*)data, indexLen_, colType_, pageId);
}

BPlusTree::~BPlusTree() {
    bufPageManger->writeBack(rootIndex);
    delete root;
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
        int16_t childIndex = cur->getChildIndex(nextSlot);
        int index;
        cur = new IndexPage((uint8_t*)(bufPageManger->getPage(fileId, childIndex, index)), indexLen, colType, childIndex);
        rec.push_back(cur);
        pageIndex.push_back(index);
    }
    int slotId = cur->searchUpperBound(data);
    int val;
    transform(val, cur->getPageId(), slotId);
    recycle(rec, pageIndex, true);
    return val;
}

void BPlusTree::search(void* data, std::vector<int> &res) {
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
            pageIndex.push_back(pageId);
        }
        if (!cur->getCompare()->equ(data, cur->getData(slotId))) {
            break;
        }
        res.push_back(cur->getVal(slotId));
        pos = cur->getNextDataIndex(slotId);
    }
    recycle(rec, pageIndex);
}
