#include <cstdio>
#include "../filesystem/bufmanager/BufPageManager.h"
#include "../filesystem/fileio/FileManager.h"
#include "BPlusTree.h"

void testBPlusTreeInsert(BPlusTree* bPlusTree) {
    int data[5000];
    int val[5000];
    int num = 4000;
    for (int i = 0; i < num; i++) {
        data[i] = i;
        val[i] = i * 10;
    }
    for (int i = 0; i < num; i++) {
        bPlusTree->insert(&data[i], val[i]);
    }
    printf("Insert Done!\n");
    std::vector<int> res;
    int lData = 0, rData = 30, searchData = 2345;
    bPlusTree->searchBetween(&lData, nullptr, res);
    // bPlusTree->search(&searchData, res);
    for (int i = 0; i < res.size(); i++) {
        printf("result %d = %d\n", i, res[i]);
    }
}

void testBPlusTreeRemove(BPlusTree* bPlusTree) {
    int data[10], val[10];
    int num = 7;
    for (int i = 0; i < num; i++) {
        data[i] = i;
        val[i] = i * 10;
    }
    for (int i = 0; i < num; i++) {
        bPlusTree->insert(&data[i], val[i]);
    }
    // printf("Insert Done!\n");
    int removeData = 1;
    bPlusTree->remove(&removeData);
    removeData = 4;
    bPlusTree->remove(&removeData);
    removeData = 2;
    bPlusTree->remove(&removeData);
    removeData = 3;
    bPlusTree->remove(&removeData);
    removeData = 0;
    bPlusTree->remove(&removeData);
    for (int i = 0; i < num; i++) {
        bPlusTree->insert(&data[i], val[i]);
    }
    // removeData = 5;
    // bPlusTree->remove(&removeData);
    // removeData = 6;
    // bPlusTree->remove(&removeData);
    // bPlusTree->insert(&data[3], val[3]);
    // bPlusTree->insert(&data[4], val[4]);
    // bPlusTree->insert(&data[0], val[0]);
    // bPlusTree->insert(&data[2], val[2]);
    // bPlusTree->insert(&data[1], val[1]);
    // printf("Remove Done!\n");
    std::vector<int> res;
    int lData = 0, rData = 30, searchData = 2345;
    bPlusTree->searchBetween(&lData, &rData, res);
    // bPlusTree->search(&searchData, res);
    for (int i = 0; i < res.size(); i++) {
        printf("result %d = %d\n", i, res[i]);
    }
}

int main() {
    printf("==========  Test For Index Section Begin  ==========\n\n");

    MyBitMap::initConst();
    FileManager* fileManager = new FileManager();
    BufPageManager* bufPageManager = new BufPageManager(fileManager);
    char fileName[] = "database/testIndex.index";
    bufPageManager->fileManager->removeFile(fileName);
    bufPageManager->fileManager->createFile(fileName);
    int fileId;
    if (!bufPageManager->fileManager->openFile(fileName, fileId)) {
        return 1;
    }

    BPlusTree* bPlusTree = new BPlusTree(fileId, bufPageManager, 1000, COL_INT);
    printf("Capacity = %d\n", bPlusTree->root->getCapacity());

    // testBPlusTreeInsert(bPlusTree);
    testBPlusTreeRemove(bPlusTree);

    bufPageManager->close();
    bufPageManager->fileManager->closeFile(fileId);
    // bufPageManager->fileManager->removeFile(fileName);

    delete bPlusTree;
    delete bufPageManager;
    delete fileManager;

    printf("==========  Test For Index Section  End   ==========\n");
    return 0;
}