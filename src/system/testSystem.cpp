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
    TB_COL_TYPE colTypes[] = {COL_VARCHAR, COL_INT, COL_INT};
    int colLen[] = {10, 4, 4};
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
    // table_3, colName_1: unique key
    // table_3, colName_2: primary key
    printf("\n");
}

void testSIUR(DatabaseManager* databaseManager) {
    printf("=====TEST SIUR=====\n");
    DBInsert* dbInsert = new DBInsert();
    char colName11[] = "col1";
    int colName12 = 1;
    int colName13 = 9;
    char colName21[] = "col2";
    int colName22 = 2;
    int colName23 = 6;
    std::vector<std::vector<void*>> valueLists;
    std::vector<std::vector<DB_LIST_TYPE>> valueListsType;
    std::vector<std::vector<int>> valueListsLen;
    valueLists.push_back(std::vector<void*>());
    valueLists.push_back(std::vector<void*>());
    valueLists[0].push_back(colName11);
    valueLists[0].push_back(&colName12);
    valueLists[0].push_back(&colName13);
    valueLists[1].push_back(colName21);
    valueLists[1].push_back(&colName22);
    valueLists[1].push_back(&colName23);
    valueListsType.push_back(std::vector<DB_LIST_TYPE>());
    valueListsType[0].push_back(DB_LIST_CHAR);
    valueListsType[0].push_back(DB_LIST_INT);
    valueListsType[0].push_back(DB_LIST_INT);
    valueListsType.push_back(valueListsType[0]);
    valueListsLen.push_back(std::vector<int>({10, 4, 4}));
    valueListsLen.push_back(std::vector<int>({10, 4, 4}));
    dbInsert->valueLists = valueLists;
    dbInsert->valueListsType = valueListsType;
    dbInsert->valueListsLen = valueListsLen;
    printf("Inserted record num = %d\n", databaseManager->insertRecords("table_3", dbInsert));
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