#include "parser/MyParser.h"
#include <cstdio>

int main() {
    DatabaseManager* databaseManager = new DatabaseManager(); // maybe updated when DBMS is completed
    MyParser* myParser = new MyParser(databaseManager);
    std::string inputSQLString = "";
    char inputSQLChar[MAX_INPUT_SIZE];
    std::string welcomeMsg = "Welcome to SimDB, a simple MySQL engine.\nCommands end with ;\n";
    printf("%s\n", welcomeMsg.c_str());
    int flag = 0;
    // flag is used to check if a input is cut off
    // if so, this input should be regard as invalid
    while (1) {
        if (flag < 2) {
            printf("%s> ", myParser->getDatabaseName().c_str());
        }
        fgets(inputSQLChar, MAX_INPUT_SIZE, stdin);
        if (strlen(inputSQLChar) > 0 && inputSQLChar[strlen(inputSQLChar) - 1] == '\n') {
            inputSQLChar[strlen(inputSQLChar) - 1] = '\0';
            if (flag > 0) {
                flag--;
            }
        } else {
            if (flag < 2) {
                printf("[ERROR] Input is too long or has unsupported type. Please re-enter.\n");
            }
            flag = 2;
        }
        if (flag) {
            continue;
        }
        // getchar();
        inputSQLString = inputSQLChar;
        fprintf(stderr, "inputSQLString = %s length = %ld\n", inputSQLString.c_str(), inputSQLString.length());
        if (inputSQLString == "quit" || inputSQLString == "quit;" || inputSQLString == "exit" || inputSQLString == "exit;") {
            std::cout << "Bye!" << std::endl;
            break;
        } else if (inputSQLString != "") {
            myParser->parse(inputSQLString);
        } else {
            fprintf(stderr, "Caution: Empty Input String.\n");
        }
    }
    delete myParser;
    delete databaseManager;
    return 0;
}