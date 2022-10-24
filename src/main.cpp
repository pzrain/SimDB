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
    fileHandler->operateTable(TB_REMOVE, colName_2);
    char colName_4[] = "id4";
    TableEntry* tableEntry_4 = new TableEntry(colName_4, COL_FLOAT);
    fileHandler->operateTable(TB_ADD, nullptr, tableEntry_4);
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
    printf("========== Begin Test Records ==========\n");
    char colName_1[] = "id";
    TableEntry *tableEntry_1 = new TableEntry(colName_1, COL_VARCHAR);
    tableEntry_1->colLen = 10;
    fileHandler->operateTable(TB_ADD, nullptr, tableEntry_1);
    fileHandler->operateTable(TB_PRINT);

    RecordId recordId;
    Record record(fileHandler->getRecordLen()), result(fileHandler->getRecordLen());
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

    printf("==========  End  Test Records ==========\n");
}

void testSerialize(FileHandler* fileHandler) {
    printf("========== Begin Test Serializing ==========\n");
    char colName_1[] = "id";
    char colName_2[] = "content";
    char colName_3[] = "testNull";
    TableEntry *tableEntry_1 = new TableEntry(colName_1, COL_FLOAT);
    TableEntry *tableEntry_2 = new TableEntry(colName_2, COL_VARCHAR);
    TableEntry *tableEntry_3 = new TableEntry(colName_3, COL_INT);
    tableEntry_2->colLen = 10;
    fileHandler->operateTable(TB_ADD, nullptr, tableEntry_1);
    fileHandler->operateTable(TB_ADD, nullptr, tableEntry_2);
    fileHandler->operateTable(TB_ADD, nullptr, tableEntry_3);
    fileHandler->operateTable(TB_PRINT);

    /* test serialize */
    RecordDataNode* TSrecordDataNode1 = new RecordDataNode();
    RecordDataNode* TSrecordDataNode2 = new RecordDataNode();
    RecordDataNode* TSrecordDataNode3 = new RecordDataNode();
    // TSrecordDataNode1->nodeType = COL_INT;
    // TSrecordDataNode1->content.intContent = new int(3);
    TSrecordDataNode1->nodeType = COL_FLOAT;
    TSrecordDataNode1->content.floatContent = new float(9.12);
    TSrecordDataNode1->len = sizeof(float);
    TSrecordDataNode1->next = TSrecordDataNode2;
    TSrecordDataNode2->nodeType = COL_VARCHAR;
    char content[] = "info";
    TSrecordDataNode2->content.charContent = content;
    TSrecordDataNode2->len = tableEntry_2->colLen;
    TSrecordDataNode2->next = TSrecordDataNode3;
    TSrecordDataNode3->nodeType = COL_NULL;
    TSrecordDataNode3->len = sizeof(int);
    RecordData TSrecordData;
    TSrecordData.head = TSrecordDataNode1;
    
    Record TSrecord(TSrecordData.getLen());
    TSrecordData.serialize(TSrecord);
    printf("Serializing result = bitmap(%d) %f %s.\n", *((uint16_t*)(TSrecord.data)), ((float*)(TSrecord.data + sizeof(uint16_t)))[0], ((char*)(TSrecord.data) + sizeof(uint16_t) + sizeof(int)));
    
    /* test deserialize */
    RecordData TDrecordData;
    TableEntryDesc tableEntryDesc = fileHandler->getTableEntryDesc();
    TSrecord.deserialize(TDrecordData, tableEntryDesc);
    RecordDataNode* cur = TDrecordData.head;
    printf("Deserializing result:\n");
    while (cur) {
        switch (cur->nodeType) {
            case COL_INT:
                printf("type = INT, content = %d\n", ((int*)(cur->content.intContent))[0]);
                break;
            case COL_FLOAT:
                printf("type = FLOAT, content = %f\n", ((cur->content.floatContent))[0]);
                break;
            case COL_VARCHAR:
                printf("type = VARCHAR(%d), content = %s\n", cur->len, ((char*)(cur->content.charContent)));
                break;
            case COL_NULL:
                printf("content = NULL\n");
                break;
            default:
                break;
        }
        cur = cur->next;
    }

    delete tableEntry_1;
    delete tableEntry_2;
    delete tableEntry_3;
    printf("==========  End  Test Serializing ==========\n\n");
}

void testSerializeAndGetACFields(FileHandler* fileHandler) {
    char colName_1[] = "float";
    char colName_2[] = "content";
    char colName_3[] = "id";
    TableEntry *tableEntry_1 = new TableEntry(colName_1, COL_FLOAT);
    TableEntry *tableEntry_2 = new TableEntry(colName_2, COL_VARCHAR);
    TableEntry *tableEntry_3 = new TableEntry(colName_3, COL_INT);
    tableEntry_2->colLen = 10;
    // fileHandler->operateTable(TB_ADD, nullptr, tableEntry_1);
    // fileHandler->operateTable(TB_ADD, nullptr, tableEntry_2);
    // fileHandler->operateTable(TB_ADD, nullptr, tableEntry_3);
    fileHandler->operateTable(TB_PRINT);

    RecordDataNode* TSrecordDataNode1 = new RecordDataNode();
    RecordDataNode* TSrecordDataNode2 = new RecordDataNode();
    RecordDataNode* TSrecordDataNode3 = new RecordDataNode();
    TSrecordDataNode1->nodeType = COL_FLOAT;
    TSrecordDataNode1->content.floatContent = new float(9.12);
    TSrecordDataNode1->len = sizeof(float);
    TSrecordDataNode1->next = TSrecordDataNode2;
    TSrecordDataNode2->nodeType = COL_VARCHAR;
    char content1[] = "info1";
    TSrecordDataNode2->content.charContent = content1;
    TSrecordDataNode2->len = tableEntry_2->colLen;
    TSrecordDataNode2->next = TSrecordDataNode3;
    TSrecordDataNode3->nodeType = COL_INT;
    TSrecordDataNode3->content.intContent = new int(10);
    TSrecordDataNode3->len = sizeof(int);
    RecordData TSrecordData1;
    TSrecordData1.head = TSrecordDataNode1;

    Record TSrecord1(TSrecordData1.getLen());
    TSrecordData1.serialize(TSrecord1);

    RecordDataNode* TSrecordDataNode4 = new RecordDataNode();
    RecordDataNode* TSrecordDataNode5 = new RecordDataNode();
    RecordDataNode* TSrecordDataNode6 = new RecordDataNode();
    TSrecordDataNode4->nodeType = COL_FLOAT;
    TSrecordDataNode4->content.floatContent = new float(12.9);
    TSrecordDataNode4->len = sizeof(float);
    TSrecordDataNode4->next = TSrecordDataNode5;
    TSrecordDataNode5->nodeType = COL_VARCHAR;
    char content2[] = "info2";
    TSrecordDataNode5->content.charContent = content2;
    TSrecordDataNode5->len = tableEntry_2->colLen;
    TSrecordDataNode5->next = TSrecordDataNode6;
    TSrecordDataNode6->nodeType = COL_INT;
    TSrecordDataNode6->content.intContent = new int(12);
    TSrecordDataNode6->len = sizeof(int);
    RecordData TSrecordData2;
    TSrecordData2.head = TSrecordDataNode4;

    Record TSrecord2(TSrecordData2.getLen());
    TSrecordData2.serialize(TSrecord2);

    std::vector<Record*> records, result;
    records.push_back(&TSrecord1);
    records.push_back(&TSrecord2);
    // fileHandler->insertAllRecords(records);
    fileHandler->getAllRecordsAccordingToFields(result, 4);
    for (auto record : result) {
        printf("bitmap = %d, result = %d\n", ((uint16_t*)record->data)[0], ((int*)(record->data + sizeof(uint16_t)))[0]);
    }

}

int main() {
    MyBitMap::initConst();
    FileManager* fileManager = new FileManager();
    BufPageManager* bufPageManager = new BufPageManager(fileManager);
    char databaseName[] = "testdata";
    RecordManager* recordManager = new RecordManager(bufPageManager, databaseName);
    FileHandler* fileHandler = new FileHandler();

    // char tableName[] = "testserial";
    char tableName[] = "test";

    // recordManager->createFile(tableName);
    recordManager->openFile(tableName, fileHandler);
    // testSerialize(fileHandler);
    // testTable(fileHandler);
    // fileHandler->operateTable(TB_PRINT, nullptr, nullptr);
    // testRecords(fileHandler);
    testSerializeAndGetACFields(fileHandler);

    // attention : you should call the method BufPageManager.close() 
    //             before you close the file!
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