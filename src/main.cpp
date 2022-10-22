#include <cstdio>
#include "record/RecordManager.h"
#include "record/FileHandler.h"
#include "record/RMComponent.h"
#include "filesystem/bufmanager/BufPageManager.h"
#include "filesystem/fileio/FileManager.h"

void testTable(FileHandler* fileHandler) {
    // char colName_1[] = "id0";
    // char colName_2[] = "id2";
    // char colName_3[] = "id3";
    // TableEntry *tableEntry_1 = new TableEntry(colName_1, COL_INT);
    // TableEntry *tableEntry_2 = new TableEntry(colName_2, COL_FLOAT);
    // TableEntry *tableEntry_3 = new TableEntry(colName_3, COL_VARCHAR);
    // tableEntry_3->colLen = 10;
    // fileHandler->operateTable(TB_ADD, nullptr, tableEntry_1);
    // fileHandler->operateTable(TB_REMOVE, colName_2);
    // fileHandler->operateTable(TB_ADD, nullptr, tableEntry_2);
    // tableEntry_2->colType = COL_VARCHAR;
    // tableEntry_2->colLen = 32;
    // fileHandler->operateTable(TB_ALTER, nullptr, tableEntry_2);
    // fileHandler->operateTable(TB_ADD, nullptr, tableEntry_3);
    // fileHandler->operateTable(TB_REMOVE, "id0");
    // fileHandler->operateTable(TB_ADD, nullptr, tableEntry_1);
    // tableEntry_1->colType = COL_FLOAT;
    // fileHandler->operateTable(TB_ALTER, nullptr, tableEntry_1);

    char colName[10] = "id";
    TableEntry* tableEntry = new TableEntry(colName, COL_INT);
    fileHandler->operateTable(TB_ADD, nullptr, tableEntry);

    // delete tableEntry;
    // delete tableEntry_1;
    // delete tableEntry_2;
    // delete tableEntry_3;
}

void testRecords(FileHandler* fileHandler) {
    RecordId recordId(1, 0);
    Record record(10), result(10);
    sprintf((char*)record.data, "%d   ", 3);
    
    // fileHandler->insertRecord(recordId, record);
    // printf("%d %d\n", recordId.getPageId(), recordId.getSlotId());
    fileHandler->getRecord(recordId, result);
    printf("result = %s\n", (char*)result.data);
}

int main() {
    MyBitMap::initConst();
    FileManager* fileManager = new FileManager();
    BufPageManager* bufPageManager = new BufPageManager(fileManager);
    char databaseName[] = "testdata";
    RecordManager* recordManager = new RecordManager(bufPageManager, databaseName);
    FileHandler* fileHandler = new FileHandler();

    char tableName[] = "test";
    // recordManager->createFile(tableName);
    recordManager->openFile(tableName, fileHandler);

    // testTable(fileHandler);
    fileHandler->operateTable(TB_PRINT, nullptr, nullptr);
    testRecords(fileHandler);

    bufPageManager->close();
    recordManager->closeFile(fileHandler);
    // recordManager->removeFile(tableName);
    delete fileHandler;
    delete recordManager;
    delete bufPageManager;
    delete fileManager;
    printf("End Bye!\n");
    return 0;
}