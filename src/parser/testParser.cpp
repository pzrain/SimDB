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
        MySQLVisitor* mySQLVisitor = new MySQLVisitor();
        mySQLVisitor->visitProgram(iTree);
        delete mySQLVisitor;
        msg = "Successfully parse!";
    }
    delete antlrParserErrorListener;
    delete antlrLexerErrorListener;
    return msg;
}

int main() {
    std::string parseString = "CREATE DATABASE mysql;";
    printf("%s\n", parse(parseString).c_str());
    return 0;
}