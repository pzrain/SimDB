#ifndef __MYSQLVisitor_H__
#define __MYSQLVisitor_H__

#include "antlr4/SQLBaseVisitor.h"
#include "SQLOptimizer.h"
#include <cstdio>

class MySQLVisitor : public SQLBaseVisitor {
public:
    std::any visitProgram(SQLParser::ProgramContext *ctx) override {
        printf("Visit Program.\n");
        return visitChildren(ctx);
    }

    std::any visitStatement(SQLParser::StatementContext *ctx) override {
        printf("Visit Statement.\n");
        return visitChildren(ctx);
    }

    std::any visitCreate_db(SQLParser::Create_dbContext *ctx) override {
        printf("Visit Create DB.\n");
        printf("Database name = %s.\n", ctx->Identifier()->getSymbol()->getText().c_str());
        return visitChildren(ctx);
    }

    std::any visitWhere_and_clause(SQLParser::Where_and_clauseContext *ctx) override {
        optimizeWhereClause(ctx);
        return visitChildren(ctx);
    }
};

#endif