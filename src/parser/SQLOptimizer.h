#ifndef __SQLOPTIMIZER_H__
#define __SQLOPTIMIZER_H__
#include "antlr4/SQLParser.h"
#include "../common.h"
#include <vector>
#include <stdio.h>
#include "../system/DatabaseManager.h"

struct SQLOptimizerEdge {
    int x;
    int v;
    int next;
};
class SQLOptimizerGraph {

#define MAX_NODE_NUM 200
#define MAX_EDGE_NUM MAX_NODE_NUM
private:
    int num, tot;
    int head[MAX_NODE_NUM];
    int fa[MAX_NODE_NUM];
    int mapping[MAX_EDGE_NUM];
    bool terminal[MAX_NODE_NUM];
    bool vis[MAX_EDGE_NUM] = {false};
    bool check[MAX_NODE_NUM][MAX_NODE_NUM] = {{false}};
    char tableNames[MAX_NODE_NUM][TAB_MAX_NAME_LEN];
    char indexNames[MAX_NODE_NUM][TAB_MAX_NAME_LEN];
    SQLOptimizerEdge edges[MAX_EDGE_NUM];

    int findFa(int x) {
        if (fa[x] == x) {
            return x;
        }
        return fa[x] = findFa(fa[x]); // path compression
    }

    int merge(int x, int y) {
        int faX = findFa(x), faY = findFa(y);
        if (faX == faY) {
            return - 1;
        }
        fa[faX] = faY;
        return 0;
    }

    void visitEdge(int x) {
        vis[x] = true;
        if (x & 1) {
            vis[x + 1] = true;
        } else {
            vis[x - 1] = true;
        }
    }

    inline void swap(SQLParser::Where_operator_expressionContext* whereClause) {
        auto LIdentifier = whereClause->column()->Identifier();
        auto RIdentifier = whereClause->expression()->column()->Identifier();
        whereClause->column()->children[0] = RIdentifier[0];
        whereClause->column()->children[2] = RIdentifier[1];
        whereClause->expression()->column()->children[0] = LIdentifier[0];
        whereClause->expression()->column()->children[2] = LIdentifier[1];
        if (whereClause->operator_() != nullptr) {
            std::string oper = whereClause->operator_()->getText();
            if (oper == ">" || oper == "<" || oper == ">=" || oper == "<=") {
                whereClause->operator_()->children.resize(2);
            }
        }
    }

public:
    SQLOptimizerGraph(): num(0), tot(0) {}

    int addNode(const char* tableName, const char* indexName, bool isTerminal = false) {
        int res = -1;
        if ((res = findNode(tableName, indexName)) >= 0) {
            return res;
        }
        res = tot;
        strcpy(tableNames[tot], tableName);
        strcpy(indexNames[tot], indexName);
        terminal[tot] = isTerminal;
        head[tot] = 0;
        fa[tot] = tot;
        tot++;
        return res;
    }

    int findNode(const char* tableName, const char* indexName) {
        for (int i = 0; i < tot; i++) {
            if (strcmp(tableName, tableNames[i]) == 0 && strcmp(indexName, indexNames[i]) == 0) {
                return i;
            }
        }
        return -1;
    }

    int addEdge(int x, int y, int mapping_) {
        if (merge(x, y) == -1) { // x and y is already connected in this graph
            return -1;
        }
        if (x >= tot || y >= tot) {
            fprintf(stderr, "Node index surpasses total number.\n");
            return - 1;
        }
        edges[++num] = {x, y, head[x]};
        head[x] = num;
        mapping[num] = mapping_;
        edges[++num] = {y, x, head[y]};
        head[y] = num;
        mapping[num] = mapping_;
        check[x][y] = true;
        check[y][x] = true;
        return 0;
    }
    
    bool existEdge(int x, int y) {
        return check[x][y];
    }

    void calc(std::vector<SQLParser::Where_clauseContext*> &vec, std::vector<SQLParser::Where_operator_expressionContext*> omitVec) {
        // The graph constructed will be an AG
        for (int i = 0; i < tot; i++) {
            if (terminal[i]) {
                for (int j = head[i]; j; j = edges[j].next) {
                    if (vis[j] || terminal[edges[j].v]) {
                        continue;
                    }
                    int now = edges[j].v, last = i;
                    int edgeNum = j;
                    
                    while (1) {
                        // fprintf(stderr, "%d x = %d y = %d\n", j, i, now);
                        visitEdge(edgeNum);
                        auto whereClause = omitVec[mapping[edgeNum]];
                        auto identifiers = whereClause->column()->Identifier();
                        // fprintf(stderr, "%s %s\n", tableNames[last], tableNames[now]);
                        if (strcmp(identifiers[0]->getText().c_str(), tableNames[last]) != 0 || strcmp(identifiers[1]->getText().c_str(), indexNames[last]) != 0) {
                            swap(whereClause);
                        }
                        vec.push_back(whereClause);
                        last = now;
                        int k;
                        for (k = head[now]; k && (vis[k] || terminal[edges[k].v]); k = edges[k].next);
                        if (k == 0) {
                            break;
                        } else {
                            edgeNum = k;
                            now = edges[k].v;
                        }
                    }
                }
            }
        }
        // the remaining edges can only be terminal<->terminal or non-terminal<->non-terminal
        for (int i = 1; i <= num; i += 2) {
            if (vis[i]) {
                continue;
            }
            // fprintf(stderr, "%d %d\n", i, mapping[i]);
            int x = edges[i].x;
            int y = edges[i].v;
            if (terminal[x]) {
                visitEdge(i);
                vec.push_back(omitVec[mapping[i]]);
            } else {
                int edgeNum = i;
                while (true) {
                    visitEdge(edgeNum);
                    auto whereClause = omitVec[mapping[edgeNum]];
                    auto identifiers = whereClause->column()->Identifier();
                    if (strcmp(identifiers[0]->getText().c_str(), tableNames[y]) != 0 || strcmp(identifiers[1]->getText().c_str(), indexNames[y]) != 0) {
                        swap(whereClause);
                    }
                    vec.push_back(whereClause);
                    int j;
                    // find first j the satisfy vis[j] == false
                    for (j = head[x]; j && vis[j]; j = edges[j].next);
                    if (j) {
                        y = x;
                        x = edges[j].v;
                        edgeNum = j;
                    } else {
                        for (j = head[y]; j && vis[j]; j = edges[j].next);
                        if (j) {
                            x = edges[j].v;
                            edgeNum = j;
                        } else {
                            break;
                        }
                    }
                }
            }
        }
    }


#undef MAX_EDGE_NUM
#undef MAX_NODE_NUM
};

void testOptimizer(SQLParser::Where_and_clauseContext* whereAndClause) {
    int siz = whereAndClause->where_clause().size();
    fprintf(stderr, "where clause size = %d\n", siz);
    for (int i = 0; i < whereAndClause->where_clause().size(); i++) {
        auto whereClause = whereAndClause->where_clause(i);
        auto childWhereClause = dynamic_cast<SQLParser::Where_operator_expressionContext*>(whereClause);
        if (childWhereClause != nullptr) {
            fprintf(stderr, "where_operator_expression[%d] is [%s]\n", i, childWhereClause->getText().c_str());
            for (int j = 0; j < childWhereClause->column()->Identifier().size(); j++) {
                auto ident = childWhereClause->column()->Identifier(j);
                fprintf(stderr, "expression[%d]'s left identifier[%d] name = %s\n", i, j, ident->getText().c_str());
            }
            fprintf(stderr, "expression[%d]'s operator is %s\n", i, childWhereClause->operator_()->getText().c_str());
            auto exp = childWhereClause->expression();
            if (exp->value() != nullptr) {
                if (exp->value()->Integer() != nullptr) {
                    fprintf(stderr, "expression[%d]'s right value = %s\n", i, exp->value()->Integer()->getText().c_str());
                } else if (exp->value()->Float() != nullptr) {
                    fprintf(stderr, "expression[%d]'s right value = %s\n", i, exp->value()->Float()->getText().c_str());
                } else if (exp->value()->String() != nullptr) {
                    fprintf(stderr, "expression[%d]'s right value = %s\n", i, exp->value()->String()->getText().c_str());
                }
            }
            if (exp->column() != nullptr) {
                for (int j = 0; j < exp->column()->Identifier().size(); j++) {
                    auto ident = exp->column()->Identifier(j);
                    fprintf(stderr, "expression[%d]'s right identifier[%d] = %s\n", i, j, ident->getText().c_str());
                }
            }
        }
    }
}

void optimizeWhereClause(SQLParser::Where_and_clauseContext* whereAndClause, DatabaseManager* databaseManager) {
    SQLOptimizerGraph graph;
    std::vector<SQLParser::Where_clauseContext*> newWhereClause, resWhereClause;
    std::vector<SQLParser::Where_operator_expressionContext*> omitWhereClause;
    fprintf(stderr, "Before Optimization:\n");
    testOptimizer(whereAndClause);
    int siz = whereAndClause->where_clause().size();
    for (int i = 0; i < siz; i++) {
        auto whereClause = whereAndClause->where_clause(i);
        auto childWhereClause = dynamic_cast<SQLParser::Where_operator_expressionContext*>(whereClause);
        bool operFlag = false;
        if (childWhereClause != nullptr) {
            auto exp = childWhereClause->expression();
            if (exp->column() != nullptr) {
                if (childWhereClause->column()->Identifier().size() == 2 && exp->column()->Identifier().size() == 2) {
                    // table1.index1 = table2.index2
                    std::string tableName1 = childWhereClause->column()->Identifier(0)->getText();
                    std::string indexName1 = childWhereClause->column()->Identifier(1)->getText();
                    std::string tableName2 = exp->column()->Identifier(0)->getText();
                    std::string indexName2 = exp->column()->Identifier(1)->getText();
                    // fprintf(stderr, "%s %s %d\n", tableName1.c_str(), indexName1.c_str(), databaseManager->hasIndex(tableName1, indexName1));
                    bool isTerminal1 = !databaseManager->hasIndex(tableName1, indexName1);
                    bool isTerminal2 = !databaseManager->hasIndex(tableName2, indexName2);
                    int index1 = graph.addNode(tableName1.c_str(), indexName1.c_str(), isTerminal1);
                    int index2 = graph.addNode(tableName2.c_str(), indexName2.c_str(), isTerminal2);
                    // fprintf(stderr, "%s %s %d %s %s %d\n", tableName1.c_str(), indexName1.c_str(), isTerminal1, tableName2.c_str(), indexName2.c_str(), isTerminal2);
                    // fprintf(stderr, "index = %d, %d\n", index1, index2);
                    if (!graph.existEdge(index1, index2)) {
                        if (graph.addEdge(index1, index2, omitWhereClause.size()) != -1) {
                            omitWhereClause.push_back(childWhereClause);
                        }
                    }
                    operFlag = true; // don't add this edge to newWhereClause
                    // there are two situations:
                    // 1. the edge is added to omitWhereClause, and will be operated in the future
                    // 2. the edge is duplicated, or connecting two nodes that are already connected, 
                    //    we simply need to throw this edge, for purpose of optimization
                }
            }
        }
        if (!operFlag) {
            newWhereClause.push_back(whereClause);
        }
    }
    graph.calc(resWhereClause, omitWhereClause);
    whereAndClause->children.clear();
    for (auto whereClause : resWhereClause) {
        whereAndClause->children.push_back(whereClause);
    }
    for (auto whereClause : newWhereClause) {
        whereAndClause->children.push_back(whereClause);
    }
    fprintf(stderr, "\nAfter Optimization:\n");
    testOptimizer(whereAndClause);
}

#endif