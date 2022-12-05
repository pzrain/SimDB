#include "MyParser.h"


int main() {
    DatabaseManager* databaseManager = new DatabaseManager();
    MyParser* myParser = new MyParser(databaseManager);
    // std::string parseString = "CREATE DATABASE mysql;";
    std::string parseString_1 = "SELECT * FROM table_1, table_2, table_3, table_4, table_5 WHERE table_3.id = table_2.id AND table_2.id = table_1.id AND table_1.id = table_3.id AND table_1.extra = table_1.extra AND table_2.id = table_4.id AND table_1.age = table_2.age AND table_3.age = 9 AND table_1.name IS NOT NULL AND table_5.id = table_3.id AND table_2.id = table_1.id;";
    std::string msg = "Fail to parse!";
    if (myParser->parse(parseString_1)) {
        msg = "Successfully parsed!";
    }
    printf("parseString_1: %s\n", msg.c_str());
    return 0;
}