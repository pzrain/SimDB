/* TODO: advanced
✔1. 删除主键
✔2. 创建主键
✔3. 创建外键
✔4. 删除外键

5. 添加一列
6. 删除一列
7. 修改列属性

✔8. 添加 HASH 索引
✔9. 删除索引
*/
#ifndef __TABLEMANAGER_H__
#define __TABLEMANAGER_H__

#include <string>
#include <vector>
#include "../record/RecordManager.h"
#include "../index/IndexManager.h"
#include "../filesystem/bufmanager/BufPageManager.h"
#include "../common.h"
using namespace std;

class TableManager {
private:
    string databaseName;
    RecordManager* recordManager;
    IndexManager* indexManager;
    FileHandler* fileHandler;
    int tableNum;
public:
    TableManager(string databaseName_, BufPageManager* bufPageManager_);
    ~TableManager();

    inline bool checkTableName(string name);

    inline bool checkTableExist(string path);
    
    int creatTable(string tableName, TableEntry* entrys, int colNum);

    int openTable(string name);
    
    int dropTable(string name);

    int listTableInfo(string name);

    /**
     * @brief: rename process:
     * 1.check if the new name is valid.
     * 2.check if the old table exists.
     * 3.close the old table.
     * 4.rename the table, actually need to modify database meta data.
     * 5.open the table again.
    */
    int renameTable(string oldName, string newName);

    int saveChangeToFile(const char* tableName);

    int createIndex(string tableName, string indexName, string colName, uint16_t indexLen_);

    int dropIndex(string tableName, string indexName);

    /**
     * @brief add only one primary key once and also drop one once, the loop is in DatabaseManager to add more then one.
     * @return column index if successfully find the correct column else -1
    */
    int createPrimaryKey(string tableName, string colName);

    int dropPrimaryKey(string tableName, string colName);

    /**
     * @brief add one foreign key, it is stored in the TableEntry. The return value will be store in database meta data.
     * @return column index if successfully find the correct column else -1
    */
    int createForeignKey(string tableName, string foreignKeyName, string colName, string refTableName, string refTableCol);
    
    /**
     * @brief set the foreignKeyConstraint of the column's entry to false.
     * @param colIndex column index of the entrys array, quickly find the correct entry
     * @return 0 if success, -1 if fail
    */
    int dropForeignKey(string tableName, uint8_t colIndex);
};

#endif