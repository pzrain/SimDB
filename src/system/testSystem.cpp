#include "DatabaseManager.h"

void testOperateDB(DatabaseManager* databaseManager) {
    printf("=====TEST OPERATE DB=====\n");
    string databaseName1 = "demo1";
    string databaseName2 = "demo2";
    string databaseName3 = "demo3";
    // // printf("database name: %s\n", databaseName1.c_str());
    databaseManager->createDatabase(databaseName1);
    databaseManager->createDatabase(databaseName2);
    // // databaseManager->switchDatabase(databaseName1);
    // // databaseManager->dropDatabase(databaseName1);
    // databaseManager->switchDatabase(databaseName2);
    // // databaseManager->dropDatabase(databaseName2);
    // printf("current databaseName: %s\n", databaseManager->getDatabaseName().c_str());
    // // databaseManager->showDatabases();
    databaseManager->switchDatabase(databaseName1);

    char colName[3][64] = {"colName_1", "colName_2", "colName_3"};
    TB_COL_TYPE colTypes[] = {COL_VARCHAR, COL_FLOAT, COL_INT};
    int colLen[] = {4, 4, 10};
    databaseManager->createTable("table_1", colName, colTypes, colLen, 3);
    databaseManager->createTable("table_2", colName, colTypes, colLen, 3);
    databaseManager->renameTable("table_2", "table_3");
    databaseManager->listTableInfo("table_3");
    databaseManager->listTablesOfDatabase();
    databaseManager->createIndex("table_1", "colName_1");
    databaseManager->createIndex("table_1", "colName_2");
    databaseManager->createIndex("table_1", "colName_3");
    databaseManager->createIndex("table_1", "colName_4");
    printf("index of colName_2 in table_1 has been created: %d\n", databaseManager->hasIndex("table_1", "colName_2"));
    databaseManager->dropIndex("table_1", "colName_2");
    databaseManager->showIndex();
    // databaseManager->dropTable("table_3");
    // databaseManager->listTablesOfDatabase();
    // databaseManager->switchDatabase(databaseName2);
    // databaseManager->switchDatabase(databaseName1);
    // databaseManager->listTablesOfDatabase();
    printf("\n");
}

void testConstraint(DatabaseManager* databaseManager) {
    printf("=====TEST CONSTRAINT=====\n");
    string databaseName1 = "demo1";
    string databaseName2 = "demo2";
    string databaseName3 = "demo3";
    databaseManager->switchDatabase(databaseName1);
    databaseManager->createPrimaryKey("table_3", {"colName_1"}, 1);
    databaseManager->createPrimaryKey("table_3", {"colName_2"}, 1);
    databaseManager->dropPrimaryKey("table_3", {"colName_2"}, 1);
    databaseManager->dropPrimaryKey("table_3", {"colName_1"}, 1);
    databaseManager->createPrimaryKey("table_3", {"colName_2"}, 1);

    databaseManager->createUniqueKey("table_3", {"colName_1"}, 1);
    databaseManager->dropPrimaryKey("table_3", {"colName_2"}, 1);
    databaseManager->createUniqueKey("table_3", {"colName_2"}, 1);

    databaseManager->createForeignKey("table_3", "foreignKey1", "colName_2", "table_1", "colName_2");
    databaseManager->createForeignKey("table_3", "foreignKey2", "colName_2", "table_1", "colName_1");
    databaseManager->dropForeignKey("table_3", "foreignKey1");
    databaseManager->createPrimaryKey("table_3", {"colName_2"}, 1);
    databaseManager->dropUniqueKey("table_3", {"colName_2"}, 1);
    databaseManager->showIndex();
    printf("\n");
}

void testSIUR(DatabaseManager* databaseManager) {
    printf("=====TEST SIUR=====\n");
    DBInsert* dbInsert = new DBInsert();
    char colName1[] = "col";
    float colName2 = 3.14;
    int colName3 = 9;
    std::vector<std::vector<void*>> valueLists;
    std::vector<std::vector<DB_LIST_TYPE>> valueListsType;
    valueLists.push_back(std::vector<void*>());
    valueLists[0].push_back(colName1);
    valueLists[0].push_back(&colName2);
    valueLists[0].push_back(&colName3);
    valueListsType.push_back(std::vector<DB_LIST_TYPE>());
    valueListsType[0].push_back(DB_LIST_CHAR);
    valueListsType[0].push_back(DB_LIST_FLOAT);
    valueListsType[0].push_back(DB_LIST_INT);
    dbInsert->valueLists = valueLists;
    dbInsert->valueListsType = valueListsType;
    databaseManager->insertRecords("table_3", dbInsert);
    printf("\n");
}

int main() {
    printf("RUN TEST\n");
    DatabaseManager* databaseManager = new DatabaseManager;
    testOperateDB(databaseManager);
    testConstraint(databaseManager);
    testSIUR(databaseManager);
    delete databaseManager;
}