#include <cstdio>
#include "../filesystem/bufmanager/BufPageManager.h"
#include "../filesystem/fileio/FileManager.h"
#include "BPlusTree.h"

void testBPlusTree(BPlusTree* bPlusTree) {
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
    printf("total = %d\n", bPlusTree->root->getTotalIndex());
    printf("Insert Done!\n");
    std::vector<int> res;
    int lData = 0, rData = 30, searchData = 2345;
    bPlusTree->searchBetween(&lData, nullptr, res);
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
    bufPageManager->fileManager->openFile(fileName, fileId);

    BPlusTree* bPlusTree = new BPlusTree(fileId, bufPageManager, 1000, COL_INT);
    printf("Capacity = %d\n", bPlusTree->root->getCapacity());

    testBPlusTree(bPlusTree);

    bufPageManager->close();
    bufPageManager->fileManager->closeFile(fileId);
    // bufPageManager->fileManager->removeFile(fileName);

    delete bPlusTree;
    delete bufPageManager;
    delete fileManager;

    printf("==========  Test For Index Section  End   ==========\n");
    return 0;
}