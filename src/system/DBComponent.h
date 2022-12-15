#ifndef __DBCOMPONENT_H__
#define __DBCOMPONENT_H__
#include <string>
#include <vector>
#include "../common.h"

// expTable.expCol  or  expCol (set expTable to "")
struct DBExpItem{
    std::string expTable, expCol;

    DBExpItem() {}

    DBExpItem(std::string expTable_, std::string expCol_) {
        expTable = expTable_;
        expCol = expCol_;
    }
};

struct DBSelItem {
    bool star; // * or not
    DBExpItem item;
    DB_SELECT_TYPE selectType;
};

/**
 * @brief expression structure for where in selection
 * @param lItem something like tableName.columnName or columnName, l means left, r means right
 * @param lVal  constant expression
 * @param lType type of left expression
 * @param valueListType types of every item in the valuelist (for "WHERE column IN value_list" only)
 * @param next for nest selection. op is IN_TYPE
 */
struct DBExpression{
    void *lVal, *rVal;
    DB_EXP_TYPE lType, rType;
    DB_EXP_OP_TYPE op;
    std::vector<DB_LIST_TYPE> valueListType;

    DBExpression() {
        lVal = nullptr;
        rVal = nullptr;
        lType = DB_NULL;
        rType = DB_NULL;
    }
};

struct DBSelect {

    std::vector<DBSelItem>    selectItems;
    std::vector<std::string>  selectTables;
    std::vector<DBExpression> expressions;     // where clause

    bool groupByEn = false;                    // group by
    DBExpItem groupByCol;

    bool limitEn = false;                      // limit
    int limitNum;

    bool offsetEn = false;                     // offset
    int offsetNum;
};

struct DBUpdate {
    std::vector<DBExpression> expItem;      // set clause, like "name=value"
    std::vector<DBExpression> expressions;  // where clause
};

struct DBInsert {
    std::vector<std::vector<void*>> valueLists;
    std::vector<std::vector<DB_LIST_TYPE>> valueListsType;
    // list of value list
    // similar to valueListType in DBExpression
    // example 2 below shows some details
};

struct DBDelete {
    std::vector<DBExpression> expression;  // where clause
};

/**
 * Examples:
 * 
 * 1. SELECT COUNT(*) FROM student;
 *  DBSelItem dbSelItem;
 *  dbSelItem.star = true;
 *  dbSelItem.selectType = COUNT_TYPE;
 *  DBSelect dbSelect;
 *  dbSelect.selectItems.push_back(dbSelItem);
 *  dbSelect.selectTables.push_back("student");
 * 
 * 
 * (usage of value list)
 * 2. SELECT teacher.id FROM school, teacher WHERE school.name="tsinghua" AND school.id IN (2, 3, 4)
 *  DBSelItem dbSelItem;
 *  dbSelItem.item.expTable = "teacher";
 *  dbSelItem.item.expCol   = "id";
 *  dbSelItem.star = false;
 *  dbSelItem.selectType = ORD_TYPE;
 * 
 *  DBExpression exp1, exp2;
 *  DBExpItem item1("school", "name");
 *  exp1.lVal = &item1;
 *  std::string schoolName = "tsinghua";
 *  exp1.rVal = &schoolName;
 *  exp1.lType = DB_ITEM;
 *  exp1.rType = DB_CHAR;
 *  exp1.op = EQU_TYPE;
 * 
 *  DBExpItem item2("school", "id");
 *  exp2.lVal = &item2;
 *  exp2.lType = DB_ITEM;
 *  std::vector<int*> val;          // attention: this should be vector<void*>
 *  val.resize(3);
 *  *(val[0]) = 2;
 *  *(val[1]) = 3;
 *  *(val[2]) = 5;
 *  for (int i = 0; i < 3; i++) {
 *      exp2.valueListType.push_back(DB_LIST_INT);
 *  }
 *  exp2.rType = DB_LIST;
 *  expr2.op   = IN_TYPE;
 *  
 *  DBSelect* dbSelect = new DBSelect();
 *  dbSelect->selectItems.push_back(dbSelItem);
 *  dbSelect->selectTables.push_back("school");
 *  dbSelect->selectTables.push_back("teacher");
 *  dbSelect->expressions.push_back(exp1);
 *  dbSelect->expressions.push_back(exp2);
 *  DatabaseManager.selectRecords(dbSelect);
 *  delete dbSelect;
 * 
 * 
 * (usage of nesty selection)
 * 3. SELECT * FROM teacher WHERE id IN (SELECT id FROM student);
 *  // we will pass for the construct of selectItems and selectTables;
 *  DBExpression exp;
 *  DBExpItem item = {"", "id"};
 *  exp.lVal = &item;
 *  DBSelect* nextSelect = new DBSelect();
 *  // construct nextSelect for "SELECT id FROM student"
 *  exp.rVal = nextSelect;
 *  exp.lType = DB_ITEM;
 *  exp.rType = DB_NST;
 *  exp.op = IN_TYPE;
 *  
 *  DBSelect* dbSelect = new DBSelect();
 *  dbSelect->expressions.push_back(exp);
 *  delete dbSelect;
 */

#endif