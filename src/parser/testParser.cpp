#include "MyParser.h"
#include <fstream>
#include <iostream>

int main() {
    DatabaseManager* databaseManager = new DatabaseManager();
    MyParser* myParser = new MyParser(databaseManager);

    ifstream input;
    input.open("test.sql", ios::in);

    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string content(buffer.str());
    myParser->parse(content);
    // std::string instruction;
    // size_t pos = content.find(";");
    // while(pos != std::string::npos) {
    //     instruction = content.substr(0, pos+1);
    //     myParser->parse(instruction);
    //     content = content.substr(pos+1);
    //     pos = content.find(";");
    // }
}
