#ifndef __MYSQLVisitor_H__
#define __MYSQLVisitor_H__

#include "antlr4/SQLBaseVisitor.h"
#include "SQLOptimizer.h"
#include <cstdio>

class DatabaseManager {
public:
    bool hasIndex(const char* tableName, const char* indexName);
};

class MySQLVisitor : public SQLBaseVisitor {
private:
    DatabaseManager* databaseManager; // Open one dbManager at most!
public:
    MySQLVisitor() {
        databaseManager = nullptr;
    }

    ~MySQLVisitor() {
        if (databaseManager) {
            delete databaseManager;
            databaseManager = nullptr;
        }
    }

    std::any visitProgram(SQLParser::ProgramContext *ctx) override {
        fprintf(stderr, "Visit Program.\n");
        return visitChildren(ctx);
    }

    std::any visitStatement(SQLParser::StatementContext *ctx) override {
        fprintf(stderr, "Visit Statement.\n");
        return visitChildren(ctx);
    }

    std::any visitCreate_db(SQLParser::Create_dbContext *ctx) override {
        fprintf(stderr, "Visit Create DB.\n");
        fprintf(stderr, "Database name = %s.\n", ctx->Identifier()->getSymbol()->getText().c_str());
        return visitChildren(ctx);
    }

    std::any visitWhere_and_clause(SQLParser::Where_and_clauseContext *ctx) override {
        optimizeWhereClause(ctx);
        return visitChildren(ctx);
    }
};

#endif