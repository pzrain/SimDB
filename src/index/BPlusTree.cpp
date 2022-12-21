#include "BPlusTree.h"
#include "assert.h"

BPlusTree::BPlusTree(int fileId_, BufPageManager* bufPageManager_, uint16_t indexLen_, uint8_t colType_) {
    fileId = fileId_;
    bufPageManager = bufPageManager_;
    indexHeader = new IndexHeader((uint8_t*)bufPageManager->getPage(fileId, 0, tableIndex));
    if (indexHeader->valid == 0) {
        assert(indexLen_ >= 0 && colType_ >= 0);
        indexHeader->init(indexLen_, colType_);
        writeIndexTable();
    }
    indexLen = indexHeader->indexLen;
    colType = indexHeader->colType;
    BufType data = bufPageManager->getPage(fileId, indexHeader->rootPageId, rootIndex);
    root = new IndexPage((uint8_t*)data, indexLen, colType, indexHeader->rootPageId);
    bufPageManager->markDirty(rootIndex);
    if (root->getCapacity() <= 2) {
        printf("[WARNING] index page capacity less than 3!\n");
    }
}

BPlusTree::~BPlusTree() {
    delete root;
}

void BPlusTree::writeIndexTable() {
    int index;
    BufType data = bufPageManager->getPage(fileId, 0, index);
    memcpy(data, indexHeader, sizeof(IndexHeader));
    bufPageManager->markDirty(index);
}

int BPlusTree::getNextFreePage() {
    int res;
    if (indexHeader->firstEmptyPage >= 0) {
        res = indexHeader->firstEmptyPage;
        int index;
        IndexPageHeader* header = (IndexPageHeader*) bufPageManager->getPage(fileId, indexHeader->firstEmptyPage, index);
        indexHeader->firstEmptyPage = header->nextFreePage;
    } else {
        res = indexHeader->totalPageNumber;
    }
    indexHeader->totalPageNumber++;
    writeIndexTable();
    return res;
}

void BPlusTree::_transform(int& val, int pageId, int slotId) {
    val = pageId * (root->getCapacity() + 2) + slotId;
}

void BPlusTree::_transformR(int val, int& pageId, int& slotId) {
    pageId = val / (root->getCapacity() + 2);
    slotId = val % (root->getCapacity() + 2);
}

void BPlusTree::transform(int& val, int pageId, int slotId) {
    _transform(val, pageId, slotId);
}

void BPlusTree::transformR(int val, int& pageId, int& slotId) {
    _transformR(val, pageId, slotId);
}

void BPlusTree::recycle(std::vector<IndexPage*> &rec, std::vector<int> &pageIndex, bool writeback) {
    int siz = rec.size();
    for (int i = 0; i < siz; i++) {
        if (writeback) {
            bufPageManager->markDirty(pageIndex[i]);
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
        cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, childIndex, index)), indexLen, colType, childIndex);
        rec.push_back(cur);
        pageIndex.push_back(index);
    }
    int slotId = cur->searchUpperBound(data);
    int val;
    if (slotId >= 0) {
        _transform(val, cur->getPageId(), slotId);
    } else { // data searched is greater than all the other data
        val = -cur->getPageId();
    }
    recycle(rec, pageIndex);
    return val;
}

void BPlusTree::dealOverFlow(IndexPage* indexPage, std::vector<IndexPage*> &rec, std::vector<int> &pageIndex) {
    assert(indexPage->getTotalIndex() == indexPage->getCapacity() + 1);
    int lNum = (indexPage->getCapacity() + 1) / 2;
    int rNum = (indexPage->getCapacity() + 1) - lNum;
    int head = indexPage->cut(lNum);
    int newPageId = getNextFreePage(), index;
    IndexPage* newIndexPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, newPageId, index)), indexLen, colType, newPageId);
    newIndexPage->initialize(indexLen, colType);
    bufPageManager->markDirty(index);
    rec.push_back(newIndexPage);
    pageIndex.push_back(index);

    std::vector<void*> removeData;
    std::vector<int> removeVal;
    std::vector<int16_t> removeChildIndex;
    indexPage->removeFrom(head, removeData, removeVal, removeChildIndex);
    newIndexPage->insertFrom(removeData, removeVal, removeChildIndex);
    int siz = removeChildIndex.size();
    for (int i = 0; i < siz; i++) {
        if (removeChildIndex[i] < 0)
            continue;
        IndexPage* childPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, removeChildIndex[i], index)), indexLen, colType, removeChildIndex[i]);
        int fatherPos_;
        _transform(fatherPos_, newPageId, i);
        *childPage->getFatherIndex() = fatherPos_;
        rec.push_back(childPage);
        pageIndex.push_back(index);
    }
    *newIndexPage->getNextPage() = *indexPage->getNextPage();
    if (*indexPage->getNextPage() >= 0) {
        IndexPage* indexPageNextPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, *indexPage->getNextPage(), index)), indexLen, colType, *indexPage->getNextPage());
        *indexPageNextPage->getLastPage() = newPageId;
        bufPageManager->markDirty(index);
        rec.push_back(indexPageNextPage);
        pageIndex.push_back(index);
    }
    *indexPage->getNextPage() = newPageId;
    *newIndexPage->getLastPage() = indexPage->getPageId();
    newIndexPage->changePageType(indexPage->getPageType());
    int fatherPos = *indexPage->getFatherIndex(), fatherPageId, fatherSlot;
    IndexPage* fatherPage = nullptr;
    if (fatherPos >= 0) {
        _transformR(fatherPos, fatherPageId, fatherSlot);
        fatherPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, fatherPageId, index)), indexLen, colType, fatherPageId);
        fatherPage->changePageType(INDEX_PAGE_INTERIOR);
        bufPageManager->markDirty(index);
        rec.push_back(fatherPage);
        pageIndex.push_back(index);

        memcpy(fatherPage->getData(fatherSlot), indexPage->getData(*indexPage->getLastIndex()), indexLen);
        int insertIndex = fatherPage->insert(newIndexPage->getData(*newIndexPage->getLastIndex()), 0, newPageId);
        int newIndexPageFatherIndex;
        _transform(newIndexPageFatherIndex, fatherPageId, insertIndex);
        *newIndexPage->getFatherIndex() = newIndexPageFatherIndex; 
    } else { // new root
        fatherPageId = getNextFreePage();
        fatherPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, fatherPageId, index)), indexLen, colType, fatherPageId);
        fatherPage->initialize(indexLen, colType);
        fatherPage->changePageType(INDEX_PAGE_INTERIOR);
        bufPageManager->markDirty(index);
        int lInsertIndex = fatherPage->insert(indexPage->getData(*indexPage->getLastIndex()), 0, indexPage->getPageId());
        int rInsertIndex = fatherPage->insert(newIndexPage->getData(*newIndexPage->getLastIndex()), 0, newIndexPage->getPageId());
        int lFatherPos, rFatherPos;
        _transform(lFatherPos, fatherPageId, lInsertIndex);
        _transform(rFatherPos, fatherPageId, rInsertIndex);
        *indexPage->getFatherIndex() = lFatherPos;
        *newIndexPage->getFatherIndex() = rFatherPos;
        root = fatherPage;
        indexHeader->rootPageId = fatherPageId;
        writeIndexTable();
    }
    if (fatherPage->overflow()) {
        dealOverFlow(fatherPage, rec, pageIndex);
    }
}

void BPlusTree::dealUnderFlow(IndexPage* indexPage, std::vector<IndexPage*> &rec, std::vector<int> &pageIndex) {
    assert(indexPage->getTotalIndex() == ((indexPage->getCapacity() + 1) / 2) - 1);
    int lastPageId = *indexPage->getLastPage();
    int nextPageId = *indexPage->getNextPage();
    int brotherPageId = -1, index;
    IndexPage* brotherPage = nullptr;
    if (lastPageId >= 0) {
        brotherPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, lastPageId, index)), indexLen, colType, lastPageId);
        rec.push_back(brotherPage);
        pageIndex.push_back(index);
        if (brotherPage->getTotalIndex() > (indexPage->getCapacity() + 1) / 2) {
            brotherPageId = lastPageId;
        }
    } else if (nextPageId >= 0) {
        brotherPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, nextPageId, index)), indexLen, colType, nextPageId);
        rec.push_back(brotherPage);
        pageIndex.push_back(index);
        if (brotherPage->getTotalIndex() > (indexPage->getCapacity() + 1) / 2) {
            brotherPageId = nextPageId;
        }
    }

    std::vector<void*> removeData;
    std::vector<int> removeVal;
    std::vector<int16_t> removeChildIndex;
    IndexPage* fatherPage = nullptr;
    int fatherPos = -1, fatherPageId, fatherSlotId;
    if (brotherPageId >= 0) { // borrow from brotherPage
        if (lastPageId >= 0) {
            int head = brotherPage->cut(brotherPage->getTotalIndex() - 1);
            brotherPage->removeFrom(head, removeData, removeVal, removeChildIndex);
        } else {
            int head = *brotherPage->getFirstIndex();
            removeData.push_back(brotherPage->getData(head));
            removeVal.push_back(*brotherPage->getVal(head));
            removeChildIndex.push_back(*brotherPage->getChildIndex(head));
            brotherPage->removeSlot(head);
        }
        assert(removeData.size() == 1);
        indexPage->insert(removeData[0], removeVal[0], removeChildIndex[0]);
        // due to brotherPage, fatherPage should exist
        fatherPos = *brotherPage->getFatherIndex();
        _transformR(fatherPos, fatherPageId, fatherSlotId);
        fatherPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, fatherPageId, index)), indexLen, colType, fatherPageId);
        rec.push_back(fatherPage);
        pageIndex.push_back(index);
        memcpy(fatherPage->getData(fatherSlotId), brotherPage->getData(*brotherPage->getLastIndex()), indexLen);
        bufPageManager->markDirty(index);
    } else {
        if (brotherPage == nullptr) { // no brother, which means father is root, then root will be changed
            *indexPage->getFatherIndex() = -1;
            rec.push_back(root);
            pageIndex.push_back(rootIndex);
            indexHeader->rootPageId = indexPage->getPageId();
            writeIndexTable();
            root = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, indexPage->getPageId(), rootIndex)), indexLen, colType, indexPage->getPageId());
        } else {
            bufPageManager->markDirty(index);
            indexPage->removeFrom(*indexPage->getFirstIndex(), removeData, removeVal, removeChildIndex);
            *indexPage->getNextFreePage() = indexHeader->firstEmptyPage;
            indexHeader->firstEmptyPage = indexPage->getPageId();
            indexHeader->totalPageNumber--;
            indexPage->clear();
            brotherPage->insert(removeData, removeVal, removeChildIndex, lastPageId < 0);
            fatherPos = *indexPage->getFatherIndex();
            _transformR(fatherPos, fatherPageId, fatherSlotId);
            fatherPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, fatherPageId, index)), indexLen, colType, fatherPageId);
            rec.push_back(fatherPage);
            pageIndex.push_back(index);
            bufPageManager->markDirty(index);
            fatherPage->removeSlot(fatherSlotId);
            if (lastPageId >= 0) { // update value in fatherPage
                fatherPos = *brotherPage->getFatherIndex();
                _transformR(fatherPos, fatherPageId, fatherSlotId);
                memcpy(fatherPage->getData(fatherSlotId), removeData[removeData.size() - 1], indexLen);
            }
            IndexPage* temp = nullptr;
            if (lastPageId >= 0) {
                if (*indexPage->getNextPage() >= 0) {
                    temp = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, *indexPage->getNextPage(), index)), indexLen, colType, *indexPage->getNextPage());
                    *temp->getLastPage() = brotherPage->getPageId();
                    rec.push_back(temp);
                    pageIndex.push_back(index);
                }
                *brotherPage->getNextPage() = *indexPage->getNextPage();
            } else {
                if (*indexPage->getLastPage() >= 0) {
                    temp = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, *indexPage->getLastPage(), index)), indexLen, colType, *indexPage->getLastPage());
                    *temp->getNextPage() = brotherPage->getPageId();
                    rec.push_back(temp);
                    pageIndex.push_back(index);
                }
                *brotherPage->getLastPage() = *indexPage->getLastPage();
            }
            if (fatherPage->getPageId() != indexHeader->rootPageId && fatherPage->underflow()) {
                dealOverFlow(fatherPage, rec, pageIndex);
            }
        }
    }
}

void BPlusTree::update(IndexPage* indexPage, void* updateData, std::vector<IndexPage*> &rec, std::vector<int> &pageIndex) {
    int fatherIndex = *(indexPage->getFatherIndex());
    if (fatherIndex < 0) {
        return;
    }
    int pageId, slotId, index;
    _transformR(fatherIndex, pageId, slotId);
    IndexPage* fatherIndexPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, pageId, index)), indexLen, colType, pageId);
    rec.push_back(fatherIndexPage);
    pageIndex.push_back(index);
    if (slotId != *fatherIndexPage->getLastIndex()) {
        return;
    }
    void* data = fatherIndexPage->getData(slotId);
    memcpy(data, updateData, indexLen);
    bufPageManager->markDirty(index);
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
        _transformR(pos, pageId, slotId);
        if (pageId != curPageId) {
            cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, pageId, index)), indexLen, colType, pageId);
            curPageId = pageId;
            rec.push_back(cur);
            pageIndex.push_back(index);
        }
        if (!cur->getCompare()->equ(data, cur->getData(slotId))) {
            break;
        }
        res.push_back(*cur->getVal(slotId));
        int nextSlot = *(cur->getNextIndex(slotId));
        if (nextSlot < 0 && *(cur->getNextPage()) >= 0) {
            curPageId = *(cur->getNextPage());
            cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, curPageId, index)), indexLen, colType, curPageId);
            rec.push_back(cur);
            pageIndex.push_back(index);
            _transform(pos, curPageId, *(cur->getFirstIndex()));
        } else if (nextSlot >= 0){
            _transform(pos, curPageId, nextSlot);
        } else {
            pos = -1;
        }
    }
    recycle(rec, pageIndex);
}

void IndexVectorReverse(std::vector<int> &res) {
    int siz = res.size();
    for (int i = 0; i < siz / 2; i++) {
        std::swap(res[i], res[siz - 1 - i]);
    }
}

void BPlusTree::searchBetween(void* ldata, void* rdata, std::vector<int> &res, bool lIn, bool rIn) {
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
            _transformR(pos, pageId, slotId);
            if (pageId != curPageId) {
                cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, pageId, index)), indexLen, colType, pageId);
                curPageId = pageId;
                rec.push_back(cur);
                pageIndex.push_back(index);
            }
            if (!lIn && cur->getCompare()->equ(cur->getData(slotId), ldata)) {
                continue;
            }
            if (rIn) { // if >, break
                if (rdata && cur->getCompare()->gt(cur->getData(slotId), rdata)) {
                    break;
                }
            } else {  // if >=, break
                if (rdata && cur->getCompare()->gte(cur->getData(slotId), rdata)) {
                    break;
                }
            }
            res.push_back(*cur->getVal(slotId));
            int nextSlot = *(cur->getNextIndex(slotId));
            if (nextSlot < 0 && *(cur->getNextPage()) >= 0) {
                curPageId = *(cur->getNextPage());
                cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, curPageId, index)), indexLen, colType, curPageId);
                rec.push_back(cur);
                pageIndex.push_back(index);
                _transform(pos, curPageId, *(cur->getFirstIndex()));
            } else if (nextSlot >= 0) {
                _transform(pos, curPageId, nextSlot);
            } else {
                pos = -1;
            }
        }
    } else { // rdata != nullptr && ldata == nullptr
        pos = searchUpperBound(rdata);
        if (pos < 0) { // rdata is greater than all the data
                      // under this condition, pos = -pageId
            pageId = -pos;
        } else {
            _transformR(pos, pageId, slotId);
        }
        cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, pageId, index)), indexLen, colType, pageId);
        curPageId = pageId;
        if (pos < 0) {
            slotId = *cur->getLastIndex();
        }
        _transform(pos, pageId, slotId);
        rec.push_back(cur);
        pageIndex.push_back(index);
        int lastSlot = *(cur->getLastIndex(slotId)), lastPage, lastPos;
        if (lastSlot < 0 && *(cur->getLastPage()) >= 0) {
            lastPage = *(cur->getLastPage());
            IndexPage* lastIndexPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, lastPage, index)), indexLen, colType, lastPage);
            rec.push_back(lastIndexPage);
            pageIndex.push_back(index);
            _transform(lastPos, lastPage, *lastIndexPage->getLastIndex());
        } else if (lastSlot >= 0) {
            lastPage = pageId;
            _transform(lastPos, lastPage, lastSlot);
        } else {
            lastPos = -1;
        }
        while (pos >= 0) {
            _transformR(pos, pageId, slotId);
            if (pageId != curPageId) {
                cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, pageId, index)), indexLen, colType, pageId);
                curPageId = pageId;
                rec.push_back(cur);
                pageIndex.push_back(index);
            }
            if (rIn) {
                if (cur->getCompare()->gt(cur->getData(slotId), rdata)) {
                    break;
                }
            } else {
                if (cur->getCompare()->gte(cur->getData(slotId), rdata)) {
                    break;
                }
            }
            res.push_back(*cur->getVal(slotId));
            int nextSlot = *(cur->getNextIndex(slotId));
            if (nextSlot < 0 && *(cur->getNextPage()) >= 0) {
                curPageId = *(cur->getNextPage());
                cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, curPageId, index)), indexLen, colType, curPageId);
                rec.push_back(cur);
                pageIndex.push_back(index);
                _transform(pos, curPageId, *(cur->getFirstIndex()));
            } else if (nextSlot >= 0){
                _transform(pos, curPageId, nextSlot);
            } else {
                pos = -1;
            }
        }
        IndexVectorReverse(res);
        pos = lastPos;
        while (pos >= 0) {
            _transformR(pos, pageId, slotId);
            if (pageId != curPageId) {
                cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, pageId, index)), indexLen, colType, pageId);
                curPageId = pageId;
                rec.push_back(cur);
                pageIndex.push_back(index);
            }
            res.push_back(*cur->getVal(slotId));
            int lastSlot = *(cur->getLastIndex(slotId));
            if (lastSlot < 0 && *(cur->getLastPage()) >= 0) {
                curPageId = *(cur->getLastPage());
                cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, curPageId, index)), indexLen, colType, curPageId);
                rec.push_back(cur);
                pageIndex.push_back(index);
                _transform(pos, curPageId, *(cur->getLastIndex()));
            } else if (lastSlot >= 0) {
                _transform(pos, curPageId, lastSlot);
            } else {
                pos = -1;
            }
        }
        IndexVectorReverse(res);
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
        _transformR(pos, pageId, slotId);
    }
    indexPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, pageId, index)), indexLen, colType, pageId);
    slotId = indexPage->insert(data, val);
    bufPageManager->markDirty(index);
    rec.push_back(indexPage);
    pageIndex.push_back(index);
    if (slotId == *(indexPage->getLastIndex())) {
        update(indexPage, data, rec, pageIndex);
    }
    if (indexPage->overflow()) {
        dealOverFlow(indexPage, rec, pageIndex);
    }
    recycle(rec, pageIndex, true);
}

void BPlusTree::update(void* data, int oldVal, int newVal) {
    int curPageId = -1, index;
    IndexPage* cur = nullptr;
    std::vector<IndexPage*> rec;
    std::vector<int> pageIndex;
    while (true) {
        int pos = searchUpperBound(data), pageId, slotId;
        while (pos >= 0) {
            _transformR(pos, pageId, slotId);
            if (pageId != curPageId) {
                cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, pageId, index)), indexLen, colType, pageId);
                curPageId = pageId;
                rec.push_back(cur);
                pageIndex.push_back(index);
            }
            if (!cur->getCompare()->equ(data, cur->getData(slotId))) {
                pos = -1;
                break;
            }
            int nextSlot = *cur->getNextIndex(slotId), nextPageId, nextPos;
            int lastSlot = *cur->getLastIndex(slotId);
            if (*cur->getVal(slotId) == oldVal) { // when find oldVal, substituate it to newVal and return
                *(cur->getVal(slotId)) = newVal;
                pos = -1;
                break;
            }
            if (nextSlot < 0 && *(cur->getNextPage()) >= 0) {
                nextPageId = *(cur->getNextPage());
                IndexPage* nextIndexPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, nextPageId, index)), indexLen, colType, nextPageId);
                rec.push_back(nextIndexPage);
                pageIndex.push_back(index);
                _transform(nextPos, nextPageId, *nextIndexPage->getFirstIndex());
            } else if (nextSlot >= 0) {
                nextPageId = pageId;
                _transform(nextPos, nextPageId, nextSlot);
            } else {
                nextPos = -1;
            }
            pos = nextPos;
        }
        if (pos < 0) {
            break;
        }
    }
    recycle(rec, pageIndex, true);
}

void BPlusTree::remove(void* data, int val) {
    int curPageId = -1, index;
    IndexPage* cur = nullptr;
    std::vector<IndexPage*> rec;
    std::vector<int> pageIndex;
    while (true) {
        int pos = searchUpperBound(data), pageId, slotId;
        while (pos >= 0) {
            _transformR(pos, pageId, slotId);
            if (pageId != curPageId) {
                cur = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, pageId, index)), indexLen, colType, pageId);
                curPageId = pageId;
                rec.push_back(cur);
                pageIndex.push_back(index);
            }
            if (!cur->getCompare()->equ(data, cur->getData(slotId))) {
                pos = -1;
                break;
            }
            int nextSlot = *cur->getNextIndex(slotId), nextPageId, nextPos;
            int lastSlot = *cur->getLastIndex(slotId);
            bool removeFlag = false;
            if (val < 0 || *cur->getVal(slotId) == val) { // when val > 0, only remove slot that has the exact val
                cur->removeSlot(slotId);
                removeFlag = true;
            }
            if ((cur->getPageId() != indexHeader->rootPageId) && cur->underflow()) {
                dealUnderFlow(cur, rec, pageIndex);
                break;
            } else {
                if (nextSlot < 0 && *(cur->getNextPage()) >= 0) {
                    // the last index of this page is deleted
                    // under this condition, the father index should be updated
                    if (removeFlag) { // only update when actually remove
                        update(cur, cur->getData(lastSlot), rec, pageIndex);
                    }
                    nextPageId = *(cur->getNextPage());
                    IndexPage* nextIndexPage = new IndexPage((uint8_t*)(bufPageManager->getPage(fileId, nextPageId, index)), indexLen, colType, nextPageId);
                    rec.push_back(nextIndexPage);
                    pageIndex.push_back(index);
                    _transform(nextPos, nextPageId, *nextIndexPage->getFirstIndex());
                } else if (nextSlot >= 0) {
                    nextPageId = pageId;
                    _transform(nextPos, nextPageId, nextSlot);
                } else {
                    nextPos = -1;
                }
                pos = nextPos;
            }
        }
        if (pos < 0) {
            break;
        }
    }
    recycle(rec, pageIndex, true);
}
