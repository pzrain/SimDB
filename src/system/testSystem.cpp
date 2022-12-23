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
    // databaseManager->dropForeignKey("table_3", "foreignKey1");
    databaseManager->createPrimaryKey("table_3", {"colName_2"}, 1);
    databaseManager->dropUniqueKey("table_3", {"colName_2"}, 1);
    databaseManager->showIndex();
    // table_3.colName_1: unique key
    // table_3.colName_2: primary key
    // table_3.colName_2 AND table_1.colName_2: foreign key
    printf("\n");
}

void testInsert(DatabaseManager* databaseManager) {
    printf("=====TEST INSERT=====\n");
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
    valueLists[0].push_back(colName11);
    valueLists[0].push_back(&colName12);
    valueLists[0].push_back(&colName13);
    valueListsType.push_back(std::vector<DB_LIST_TYPE>());
    valueListsType[0].push_back(DB_LIST_CHAR);
    valueListsType[0].push_back(DB_LIST_INT);
    valueListsType[0].push_back(DB_LIST_INT);
    valueLists.push_back(std::vector<void*>());
    valueLists[1].push_back(colName21);
    valueLists[1].push_back(&colName22);
    valueLists[1].push_back(&colName23);
    valueListsType.push_back(valueListsType[0]);
    dbInsert->valueLists = valueLists;
    dbInsert->valueListsType = valueListsType;
    printf("Inserted record num on %s = %d\n", "table_1", databaseManager->insertRecords("table_1", dbInsert));
    dbInsert->valueLists.resize(1);
    dbInsert->valueListsType.resize(1);
    printf("Inserted record num on %s = %d\n", "table_3", databaseManager->insertRecords("table_3", dbInsert));
    printf("\n");
    delete dbInsert;
}

void testDrop(DatabaseManager* databaseManager) {
    printf("=====TEST DROP=====\n");
    std::vector<DBExpression> expressions;
    DBExpItem item1("", "colName_2");
    int val = 2;
    DBExpression dbExpression;
    dbExpression.lVal = &item1;
    dbExpression.lType = DB_ITEM;
    dbExpression.rType = DB_INT;
    dbExpression.rVal = &val;
    dbExpression.op = EQU_TYPE;
    expressions.push_back(dbExpression);
    DBDelete* dbDelete = new DBDelete();
    dbDelete->expression = expressions;

    printf("Dropped record num = %d\n", databaseManager->dropRecords("table_1", dbDelete));
    printf("\n");
    delete dbDelete;
}

void testUpdate(DatabaseManager* databaseManager) {
    printf("=====TEST UPDATE=====\n");
    vector<DBExpression> expItem, expressions;
    DBExpression itemExp;
    DBExpItem item("", "colName_3");
    itemExp.lVal = &item;
    itemExp.lType = DB_ITEM;
    int rVal = 129;
    itemExp.rVal = &rVal;
    itemExp.rType = DB_INT;
    itemExp.op = EQU_TYPE;
    expItem.push_back(itemExp);
    DBExpression exp;
    DBExpItem item1("", "colName_2");
    exp.lVal = &item1;
    exp.lType = DB_ITEM;
    int expRVal = 2;
    exp.rVal = &expRVal;
    exp.rType = DB_INT;
    exp.op = LTE_TYPE;
    expressions.push_back(exp);

    DBUpdate* dbUpdate = new DBUpdate();
    dbUpdate->expItem = expItem;
    dbUpdate->expressions = expressions;
    printf("Update record num = %d\n", databaseManager->updateRecords("table_1", dbUpdate));
    printf("\n");
    delete dbUpdate;
}

void testSelect(DatabaseManager* databaseManager) {
    /**
     *  Attention: Null type currently is not supported, 
     *             so "IS(NOT) Null" is not tested
     */
    printf("=====TEST SELECT=====\n");
    databaseManager->switchDatabase("demo1");
    vector<DBSelItem> selectItems;
    vector<string> selectTables;
    vector<DBExpression> expressions;
    DBSelItem selItem, selItem2;

    // SELECT *
    selItem.star = true;
    selItem.selectType = ORD_TYPE;
    selectItems.push_back(selItem);
    selectTables.push_back("table_1");
    DBSelect* dbSelect = new DBSelect();
    dbSelect->selectItems = selectItems;
    dbSelect->selectTables = selectTables;
    dbSelect->expressions = expressions;
    printf("(SELECT *) Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    // COUNT(*)
    selItem.selectType = COUNT_TYPE;
    selectItems.clear();
    selectItems.push_back(selItem);
    dbSelect->selectItems = selectItems;
    printf("(COUNT(*))Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    // aggregation query
    selItem.star = false;
    selItem.item = DBExpItem("", "colName_2");
    selItem.selectType = SUM_TYPE;
    selectItems.clear();
    selectItems.push_back(selItem);
    dbSelect->selectItems = selectItems;
    printf("(Aggregation Query) Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    // test Where operation
    selItem.selectType = ORD_TYPE;
    DBExpression exp;
    DBExpItem item1("", "colName_2");
    exp.lVal = &item1;
    exp.lType = DB_ITEM;
    int expRVal = 2;
    exp.rVal = &expRVal;
    exp.rType = DB_INT;
    exp.op = EQU_TYPE;
    expressions.push_back(exp);
    dbSelect->expressions = expressions;
    printf("(Where Operation) Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    // LIKE: fuzzy query
    selectItems.clear();
    selectTables.clear();
    expressions.clear();
    selItem.selectType = ORD_TYPE;
    selItem.star = true;
    selectItems.push_back(selItem);
    selectTables.push_back("table_1");
    item1 = DBExpItem("", "colName_1");
    exp.lVal = &item1;
    exp.lType = DB_ITEM;
    char expCharRVal[] = "%ol1%";
    exp.rVal = expCharRVal;
    exp.rType = DB_CHAR;
    exp.op = LIKE_TYPE;
    expressions.push_back(exp);
    dbSelect->selectItems = selectItems;
    dbSelect->selectTables = selectTables;
    dbSelect->expressions = expressions;
    printf("(LIKE) Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    // IN value_list
    selectItems.clear();
    selectTables.clear();
    expressions.clear();
    selItem.star = false;
    selItem.item = DBExpItem("", "colName_2");
    selItem.selectType = ORD_TYPE;
    selectItems.push_back(selItem);
    selectTables.push_back("table_1");
    item1 = DBExpItem("", "colName_2");
    exp.lVal = &item1;
    exp.lType = DB_ITEM;
    std::vector<void*> intValList;
    for (int i = 0; i < 2; i++) {
        intValList.push_back(new int);
    }
    intValList.push_back(new float);
    *(int*)(intValList[0]) = 2;
    *(int*)(intValList[1]) = 3;
    *(float*)(intValList[2]) = 1;
    for (int i = 0; i < 2; i++) {
        exp.valueListType.push_back(DB_LIST_INT);
    }
    exp.valueListType.push_back(DB_LIST_FLOAT);
    exp.rVal = &intValList;
    exp.rType = DB_LIST;
    exp.op = IN_TYPE;
    expressions.push_back(exp);
    dbSelect->selectItems = selectItems;
    dbSelect->selectTables = selectTables;
    dbSelect->expressions = expressions;
    printf("(In ValueList) Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    // IN Select
    selectItems.clear();
    selectTables.clear();
    expressions.clear();
    selItem.star = true;
    selItem.selectType = ORD_TYPE;
    selectItems.push_back(selItem);
    selectTables.push_back("table_1");
    DBSelect* childDBSelect = new DBSelect();
    DBSelItem childSelItem;
    childSelItem.star = false;
    childSelItem.item = DBExpItem("", "colName_3");
    childSelItem.selectType = ORD_TYPE;
    childDBSelect->selectItems.push_back(childSelItem);
    childDBSelect->selectTables.push_back("table_1");
    item1 = DBExpItem("", "colName_3");
    exp.lVal = &item1;
    exp.lType = DB_ITEM;
    exp.rVal = childDBSelect;
    exp.rType = DB_NST;
    exp.op = IN_TYPE;
    expressions.push_back(exp);
    dbSelect->selectItems = selectItems;
    dbSelect->selectTables = selectTables;
    dbSelect->expressions = expressions;
    printf("(In Select) Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    // limit and offset
    dbSelect->limitEn = true;
    dbSelect->limitNum = 1;
    printf("(Limit) Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    dbSelect->offsetEn = true;
    dbSelect->offsetNum = 1;
    printf("(Offset) Select record num = %d\n", databaseManager->selectRecords(dbSelect));
    dbSelect->limitEn = false;
    dbSelect->offsetEn = false;

    // operate_ select
    selectItems.clear();
    selectTables.clear();
    expressions.clear();
    childDBSelect->selectItems.clear();
    childDBSelect->expressions.clear();
    childDBSelect->selectTables.clear();
    selItem.star = true;
    selItem.selectType = ORD_TYPE;
    selectItems.push_back(selItem);
    selectTables.push_back("table_1");
    childSelItem.star = false;
    childSelItem.item = DBExpItem("", "colName_3");
    childSelItem.selectType = MAX_TYPE;
    childDBSelect->selectItems.push_back(childSelItem);
    childDBSelect->selectTables.push_back("table_1");
    item1 = DBExpItem("", "colName_3");
    exp.lVal = &item1;
    exp.lType = DB_ITEM;
    exp.rVal = childDBSelect;
    exp.rType = DB_NST;
    exp.op = LTE_TYPE;
    expressions.push_back(exp);
    dbSelect->selectItems = selectItems;
    dbSelect->selectTables = selectTables;
    dbSelect->expressions = expressions;
    printf("(Operate Select) Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    // group by
    selectItems.clear();
    selectTables.clear();
    expressions.clear();
    // selItem.star = true;
    // selItem.selectType = COUNT_TYPE;
    selItem.star = false;
    selItem.item = DBExpItem("", "colName_2");
    selItem.selectType = SUM_TYPE;
    selItem2.star = false;
    selItem2.item = DBExpItem("", "colName_3");
    selItem2.selectType = ORD_TYPE;
    selectItems.push_back(selItem);
    selectItems.push_back(selItem2);
    selectTables.push_back("table_1");
    dbSelect->selectItems = selectItems;
    dbSelect->selectTables = selectTables;
    dbSelect->expressions = expressions;
    dbSelect->groupByEn = true;
    dbSelect->groupByCol = DBExpItem("", "colName_3");
    printf("(Group By) Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    // join two tables
    // SELECT table_3.colName_2, table_1.colName_2 FROM table_1, table_3 WHERE table_1.colName_2 <= table_3.colName_2 AND table_1.colName_2 = 1;
    dbSelect->groupByEn = false;
    selectItems.clear();
    selectTables.clear();
    expressions.clear();
    selItem.star = false;
    selItem.item = DBExpItem("table_3", "colName_2");
    selItem.selectType = ORD_TYPE;
    selItem2.star = false;
    selItem2.item = DBExpItem("table_1", "colName_2");
    selItem2.selectType = ORD_TYPE;
    selectItems.push_back(selItem);
    selectItems.push_back(selItem2);
    selectTables.push_back("table_1");
    selectTables.push_back("table_3");
    item1 = DBExpItem("table_1", "colName_2");
    DBExpItem item2 = DBExpItem("table_3", "colName_2");
    exp.lVal = &item1;
    exp.lType = DB_ITEM;
    exp.rVal = &item2;
    exp.rType = DB_ITEM;
    exp.op = LTE_TYPE;
    DBExpression exp2;
    DBExpItem item3 = DBExpItem("table_1", "colName_2");
    exp2.lVal = &item3;
    exp2.lType = DB_ITEM;
    int mulExpRVal = 1;
    exp2.rVal = &mulExpRVal;
    exp2.rType = DB_INT;
    exp2.op = EQU_TYPE;
    expressions.push_back(exp);
    expressions.push_back(exp2);
    dbSelect->selectItems = selectItems;
    dbSelect->selectTables = selectTables;
    dbSelect->expressions = expressions;
    printf("(Join )Select record num = %d\n", databaseManager->selectRecords(dbSelect));

    delete childDBSelect;
    delete dbSelect;
}

int main() {
    printf("RUN TEST\n");
    DatabaseManager* databaseManager = new DatabaseManager();
    testOperateDB(databaseManager);
    testConstraint(databaseManager);
    testInsert(databaseManager);
    // testDrop(databaseManager);
    testUpdate(databaseManager);
    testSelect(databaseManager);
    delete databaseManager;
}