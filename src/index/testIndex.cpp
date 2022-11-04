#include <cstdio>
#include "../filesystem/bufmanager/BufPageManager.h"
#include "../filesystem/fileio/FileManager.h"
#include "BPlusTree.h"

void testBPlusTree(BPlusTree* bPlusTree) {
    int data[100];
    int val[100];
    int num = 22;
    for (int i = 0; i < num; i++) {
        data[i] = num - 1 - i;
        val[i] = i * 10;
    }
    std::vector<int> res;
    for (int i = 0; i < num; i++) {
        bPlusTree->insert(&data[i], val[i]);
    }
    printf("Insert Done!\n");
    int lData = 0, rData = 30, searchData = 20;
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
    char fileName[] = "testIndex.index";
    bufPageManager->fileManager->createFile(fileName);
    int fileId;
    bufPageManager->fileManager->openFile(fileName, fileId);

    BPlusTree* bPlusTree = new BPlusTree(fileId, bufPageManager, 1000, COL_INT);
    printf("Capacity = %d\n", bPlusTree->root->getCapacity());

    testBPlusTree(bPlusTree);

    bufPageManager->fileManager->closeFile(fileId);
    bufPageManager->fileManager->removeFile(fileName);

    delete bPlusTree;
    delete bufPageManager;
    delete fileManager;

    printf("==========  Test For Index Section  End   ==========\n");
    return 0;
}