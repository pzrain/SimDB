#include <cstdio>
#include "../filesystem/bufmanager/BufPageManager.h"
#include "../filesystem/fileio/FileManager.h"
#include "BPlusTree.h"

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

    bufPageManager->fileManager->closeFile(fileId);
    bufPageManager->fileManager->removeFile(fileName);

    delete bPlusTree;
    delete bufPageManager;
    delete fileManager;

    printf("==========  Test For Index Section  End   ==========\n");
    return 0;
}