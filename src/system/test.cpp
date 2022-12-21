#include "DatabaseManager.h"

int main() {
    printf("RUN TEST\n");
    DatabaseManager* databaseManager = new DatabaseManager;
    string databaseName1 = "demo1";
    printf("database name: %s\n", databaseName1.c_str());
    databaseManager->createDatabase(databaseName1);
    databaseManager->switchDatabase(databaseName1);
    // databaseManager->dropDatabase(databaseName1);
    // string databaseName2 = "demo2";
    // databaseManager->createDatabase(databaseName2);
    // databaseManager->switchDatabase(databaseName2);
    // databaseManager->dropDatabase(databaseName2);
}