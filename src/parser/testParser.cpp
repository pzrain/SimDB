#include <string>
#include "antlr4/SQLLexer.h"
#include "MySQLVisitor.h"
#include "MyErrorListener.h"

std::string parse(std::string SQL) {
    int err = 0;
    antlr4::ANTLRInputStream sInputStream(SQL);
    SQLLexer iLexer(&sInputStream);
    antlr4::CommonTokenStream sTokenStream(&iLexer);
    SQLParser iParser(&sTokenStream);
    MyANTLRParserErrorListener* antlrParserErrorListener = new MyANTLRParserErrorListener(&err);
    MyANTLRLexerErrorListener* antlrLexerErrorListener = new MyANTLRLexerErrorListener(&err);
    iParser.removeErrorListeners();
    iParser.addErrorListener(antlrParserErrorListener);
    iLexer.removeErrorListeners();
    iLexer.addErrorListener(antlrLexerErrorListener);
    auto iTree = iParser.program();
    std::string msg;
    if (err > 0) {
        printf("[ERROR] detect %d error in parsing.\n", err);
        msg = "Fail to parse!";
    } else {
        DatabaseManager* databaseManager = new DatabaseManager(); // may be updated when dbms is done
        MySQLVisitor* mySQLVisitor = new MySQLVisitor(databaseManager);
        mySQLVisitor->visitProgram(iTree);
        delete mySQLVisitor;
        msg = "Successfully parse!";
    }
    delete antlrParserErrorListener;
    delete antlrLexerErrorListener;
    return msg;
}

int main() {
    // std::string parseString = "CREATE DATABASE mysql;";
    std::string parseString_1 = "SELECT * FROM table_1, table_2, table_3, table_4, table_5 WHERE table_3.id = table_2.id AND table_2.id = table_1.id AND table_1.id = table_3.id AND table_1.extra = table_1.extra AND table_2.id = table_4.id AND table_1.age = table_2.age AND table_3.age = 9 AND table_1.name IS NOT NULL AND table_5.id = table_3.id AND table_2.id = table_1.id;";
    printf("parseString_1: %s\n", parse(parseString_1).c_str());
    return 0;
}