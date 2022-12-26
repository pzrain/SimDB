#include "MyParser.h"


int main() {
    DatabaseManager* databaseManager = new DatabaseManager();
    MyParser* myParser = new MyParser(databaseManager);
    std::string parseString0 = "CREATE DATABASE mysql; SHOW DATABASES; DROP DATABASE mysql; SHOW DATABASES;";
    std::string parseString1 = "SELECT * FROM table_1, table_2, table_3, table_4, table_5 \
                                 WHERE table_3.id = table_2.id AND table_2.id = table_1.id AND table_1.id = table_3.id AND table_1.extra = table_1.extra AND table_2.id = table_4.id AND table_1.age = table_2.age AND table_3.age = 9 AND table_1.name IS NOT NULL AND table_5.id = table_3.id AND table_2.id = table_1.id;";
    std::string parseString2 = "CREATE DATABASE mysql; \
                                USE mysql;\
                                CREATE TABLE test_table(test_int INT DEFAULT 3, test_float FLOAT DEFAULT 3.14, test_char VARCHAR(8) DEFAULT 'hi');\
                                SHOW DATABASES;\
                                DESC test_table;";
                                //CREATE TABLE test_table(test_int INT, test_float FLOAT, test_char VARCHAR(8));";
                                // there is a bug when drop a table created last time, may be meta data read error

    std::string parseString3 = "CREATE DATABASE mysql; \
                                CREATE DATABASE testdb; \
                                USE mysql; \
                                CREATE TABLE test_table(test_int INT, test_float FLOAT, test_char VARCHAR(8)); \
                                USE testdb;\
                                SHOW DATABASES;\
                                DESC test_table;";
                                // INSERT INTO test_table VALUES (1, 3.3, 'abc');"; // cause segmentation fault


    std::string msg = "Fail to parse!";
    if (myParser->parse(parseString2)) {
        msg = "Successfully parsed!";
    }
    printf("parseString2: %s\n", msg.c_str());
    return 0;
}