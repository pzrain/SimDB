#ifndef __MYPARSER_H__
#define __MYPARSER_H__
#include <string>
#include "antlr4/SQLLexer.h"
#include "MySQLVisitor.h"
#include "MyErrorListener.h"
#include "../system/DatabaseManager.h"

class MyParser {
private:
    DatabaseManager* databaseManager;
    MySQLVisitor* mySQLVisitor;
public:
    MyParser(DatabaseManager* databaseManager_): databaseManager(databaseManager_) {
        mySQLVisitor = new MySQLVisitor(databaseManager);
    }

    ~MyParser() {
        delete mySQLVisitor;
    }

    /**
     * @brief parse the input SQL string
     */
    bool parse(std::string SQL) {
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
            return false;
        } else {
            mySQLVisitor->visitProgram(iTree);
        }
        delete antlrParserErrorListener;
        delete antlrLexerErrorListener;
        return 1;
    }

    std::string getDatabaseName() {
        return databaseManager->getDatabaseName();
    }
};

#endif