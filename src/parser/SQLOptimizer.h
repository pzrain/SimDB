#ifndef __SQLOPTIMIZER_H__
#define __SQLOPTIMIZER_H__
#include "antlr4/SQLParser.h"

void optimizeWhereClause(SQLParser::Where_and_clauseContext* whereAndClause) {
    // TODO: optimization when joining multiply tables
    for (int i = 0; i < whereAndClause->where_clause().size(); i++) {
        auto whereClause = whereAndClause->where_clause(i);

        auto childWhereClause = dynamic_cast<SQLParser::Where_operator_expressionContext*>(whereClause);
        if (childWhereClause != nullptr) {
            printf("where_operator_expression[%d] is %s\n", i, childWhereClause->getText().c_str());
            for (int j = 0; j < childWhereClause->column()->Identifier().size(); j++) {
                auto ident = childWhereClause->column()->Identifier(j);
                printf("expression[%d]'s left identifier[%d] name = %s\n", i, j, ident->getText().c_str());
            }
            printf("expression[%d]'s operator is %s\n", i, childWhereClause->operator_()->getText().c_str());
            auto exp = childWhereClause->expression();
            if (exp->value() != nullptr) {
                printf("expression[%d]'s right value = %s\n", i, exp->value()->Integer()->getText().c_str());
            }
            if (exp->column() != nullptr) {
                for (int j = 0; j < exp->column()->Identifier().size(); j++) {
                    auto ident = childWhereClause->column()->Identifier(j);
                    printf("expression[%d]'s right identifier[%d] = %s\n", i, j, ident->getText().c_str());
                }
            }
        }
    }
}

#endif