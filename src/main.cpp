#include <cstdio>
#include "record/RecordManager.h"
#include "record/FileHandler.h"
#include "record/RMComponent.h"
#include "filesystem/bufmanager/BufPageManager.h"
#include "filesystem/fileio/FileManager.h"

void testTable(FileHandler* fileHandler) {
    char colName_1[] = "id0";
    char colName_2[] = "id2";
    char colName_3[] = "id3";
    TableEntry tableEntrys[] = {
        TableEntry(colName_1, COL_INT),
        TableEntry(colName_2, COL_FLOAT),
        TableEntry(colName_3, COL_VARCHAR)
    };
    tableEntrys[2].colLen = 20;
    fileHandler->operateTable(TB_INIT, nullptr, tableEntrys, 3);
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

    // char colName[10] = "id";
    // TableEntry* tableEntry = new TableEntry(colName, COL_INT);
    // fileHandler->operateTable(TB_ADD, nullptr, tableEntry);

    // delete tableEntry;
    // delete tableEntry_1;
    // delete tableEntry_2;
    // delete tableEntry_3;
}

void testRecords(FileHandler* fileHandler) {
    RecordId recordId;
    Record record(10), result(10);
    std::vector<Record*> vecRecord;

    for (int i = 0; i < 5; i++) {
        sprintf((char*)record.data, "%d   ", i);
        fileHandler->insertRecord(recordId, record);
    }

    fileHandler->getRecord(recordId, result);
    printf("result = %s\n", (char*)result.data);
    recordId.set(1, 0);
    fileHandler->getRecord(recordId, result);
    printf("result = %s\n", (char*)result.data);
    
    recordId.set(1, 1);
    fileHandler->removeRecord(recordId);
    recordId.set(1, 3);
    fileHandler->removeRecord(recordId);
    sprintf((char*)record.data, "%d   ", 6);
    fileHandler->insertRecord(recordId, record);
    recordId.set(1, 3);
    sprintf((char*)record.data, "%d   ", 7);
    fileHandler->updateRecord(recordId, record);
    recordId.set(1, 2);
    sprintf((char*)record.data, "%d   ", 8);
    fileHandler->updateRecord(recordId, record);
    
    fileHandler->getAllRecords(vecRecord);
    printf("Total record number = %d\n", fileHandler->getRecordNum());
    for (auto rec : vecRecord) {
        printf("data = %s\n", (char*)(rec->data));
    }

    // fileHandler->insertAllRecords(vecRecord);

    // recycle the memory
    for (auto rec : vecRecord) {
        delete rec;
    }
    vecRecord.clear();
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
    // testRecords(fileHandler);

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