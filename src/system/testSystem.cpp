#include "DatabaseManager.h"

void testOperateDB(DatabaseManager* databaseManager) {
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
    TB_COL_TYPE colTypes[] = {COL_INT, COL_FLOAT, COL_VARCHAR};
    int colLen[] = {4, 4, 10};
    // databaseManager->createTable("table_1", colName, colTypes, colLen, 3);
    // databaseManager->createTable("table_2", colName, colTypes, colLen, 3);
    // databaseManager->renameTable("table_2", "table_3");
    databaseManager->listTableInfo("table_3");
    databaseManager->listTablesOfDatabase();
    // databaseManager->createIndex("table_1", "colName_1");
    // databaseManager->createIndex("table_1", "colName_2");
    // databaseManager->createIndex("table_1", "colName_3");
    // databaseManager->createIndex("table_1", "colName_4");
    // printf("index of colName_2 in table_1 has been created: %d\n", databaseManager->hasIndex("table_1", "colName_2"));
    // databaseManager->dropIndex("table_1", "colName_2");
    databaseManager->showIndex();
    // databaseManager->dropTable("table_3");
    // databaseManager->listTablesOfDatabase();
    // databaseManager->switchDatabase(databaseName2);
    // databaseManager->switchDatabase(databaseName1);
    // databaseManager->listTablesOfDatabase();
}

int main() {
    printf("RUN TEST\n");
    DatabaseManager* databaseManager = new DatabaseManager;
    testOperateDB(databaseManager);
    delete databaseManager;
}