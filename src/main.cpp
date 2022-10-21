#include <cstdio>
#include "record/RecordManager.h"
#include "record/FileHandler.h"
#include "record/RMComponent.h"
#include "filesystem/bufmanager/BufPageManager.h"
#include "filesystem/fileio/FileManager.h"

int main() {
    MyBitMap::initConst();
    FileManager* fileManager = new FileManager();
    BufPageManager* bufPageManager = new BufPageManager(fileManager);
    RecordManager* recordManager = new RecordManager(bufPageManager);
    FileHandler* fileHandler = new FileHandler();

    char filename[] = "database/test.db";
    // recordManager->createFile(filename);
    recordManager->openFile(filename, fileHandler);

    char colName_1[] = "id0";
    char colName_2[] = "id2";
    char colName_3[] = "id3";
    TableEntry *tableEntry_1 = new TableEntry(colName_1, COL_INT);
    TableEntry *tableEntry_2 = new TableEntry(colName_2, COL_FLOAT);
    TableEntry *tableEntry_3 = new TableEntry(colName_3, COL_VARCHAR);
    tableEntry_3->colLen = 10;
    fileHandler->operateTable(TB_ALTER, nullptr, tableEntry_1);
    // fileHandler->operateTable(TB_REMOVE, colName_2);
    fileHandler->operateTable(TB_ADD, nullptr, tableEntry_2);
    // fileHandler->operateTable(TB_ADD, nullptr, tableEntry_3);
    fileHandler->operateTable(TB_PRINT, nullptr, nullptr);

    bufPageManager->close();
    recordManager->closeFile(fileHandler);
    // recordManager->removeFile(filename);
    delete fileHandler;
    delete recordManager;
    delete bufPageManager;
    delete fileManager;
    printf("End Bye!\n");
    return 0;
}