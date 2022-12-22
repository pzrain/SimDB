#ifndef __MYSQLVisitor_H__
#define __MYSQLVisitor_H__

#include "antlr4/SQLBaseVisitor.h"
#include "SQLOptimizer.h"
#include <cstdio>


class MySQLVisitor : public SQLBaseVisitor {
private:
    DatabaseManager* databaseManager;
public:
    MySQLVisitor(DatabaseManager* databaseManager_) {
        databaseManager = databaseManager_;
    }

    ~MySQLVisitor() {}

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
        databaseManager->createDatabase(ctx->Identifier()->getSymbol()->getText());
        return visitChildren(ctx);
    }

    std::any visitDrop_db(SQLParser::Drop_dbContext *ctx) override {
        fprintf(stderr, "Visit Drop DB.\n");
        fprintf(stderr, "Database name = %s.\n", ctx->Identifier()->getSymbol()->getText().c_str());
        databaseManager->dropDatabase(ctx->Identifier()->getSymbol()->getText());
        return visitChildren(ctx);
    }

    std::any visitShow_dbs(SQLParser::Show_dbsContext *ctx) override {
        fprintf(stderr, "Visit Show DB.\n");
        databaseManager->showDatabases();
        return visitChildren(ctx);
    }

    std::any visitUse_db(SQLParser::Use_dbContext *ctx) override {
        fprintf(stderr, "Visit Use DB.\n");
        databaseManager->switchDatabase(ctx->Identifier()->getText());
        printf("Database changed\n");
        return visitChildren(ctx);
    }

    std::any visitShow_tables(SQLParser::Show_tablesContext *ctx) override {
        fprintf(stderr, "Visit Show DB tables.\n");
        databaseManager->listTablesOfDatabase();
        return visitChildren(ctx);
    }

    std::any visitShow_indexes(SQLParser::Show_indexesContext *ctx) override {
        fprintf(stderr, "Visit Show DB indexes.\n");
        databaseManager->showIndex();
        return visitChildren(ctx);
    }

    std::any visitLoad_data(SQLParser::Load_dataContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitDump_data(SQLParser::Dump_dataContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitCreate_table(SQLParser::Create_tableContext *ctx) override {
        fprintf(stderr, "Visit Create Table.\n");
        fprintf(stderr, "Table name = %s.\n", ctx->Identifier()->getSymbol()->getText().c_str());
        fprintf(stderr, "Number of field = %ld.\n", ctx->field_list()->field().size());

        string tableName = ctx->Identifier()->getSymbol()->getText();
        int colNum = ctx->field_list()->field().size();
        char (*colName)[COL_MAX_NAME_LEN] = new char[colNum][COL_MAX_NAME_LEN];
        TB_COL_TYPE* colType = new TB_COL_TYPE[colNum];
        int *colLen = new int[colNum];

        for(int i = 0; i < colNum; i++) {
            auto field = ctx->field_list()->field(i);
            if(field->start->getType() == 64) {
                // normal field
                auto normalField = dynamic_cast<SQLParser::Normal_fieldContext*>(field);
                strcpy(colName[i], normalField->getStart()->getText().c_str());
                if(normalField->type_()->start->getText() == "INT") {
                    colType[i] = COL_INT;
                    colLen[i] = 4;
                } else if(normalField->type_()->getStart()->getText() == "VARCHAR") {
                    colType[i] = COL_VARCHAR;
                    colLen[i] = stoi(normalField->type_()->Integer()->getText());
                } else if(normalField->type_()->getStart()->getText() == "FLOAT") {
                    colType[i] = COL_FLOAT;
                    colLen[i] = 4;
                } else {
                    // TODO error
                }
            }
        }
        databaseManager->createTable(tableName, colName, colType, colLen, colNum);
        delete [] colName;
        delete [] colType;
        delete [] colLen;
        return visitChildren(ctx);
    }

    std::any visitDrop_table(SQLParser::Drop_tableContext *ctx) override {
        fprintf(stderr, "Visit Drop Table.\n");
        fprintf(stderr, "Table name = %s.\n", ctx->Identifier()->getSymbol()->getText().c_str());
        databaseManager->dropTable(ctx->Identifier()->getSymbol()->getText());
        return visitChildren(ctx);
    }

    std::any visitDescribe_table(SQLParser::Describe_tableContext *ctx) override {
        fprintf(stderr, "Visit Describe Table.\n");
        databaseManager->listTableInfo(ctx->Identifier()->getSymbol()->getText());
        return visitChildren(ctx);
    }

    std::any visitInsert_into_table(SQLParser::Insert_into_tableContext *ctx) override {
        fprintf(stderr, "Visit Insert Into Table.\n");
        DBInsert* dbInsert;
        auto value_lists = ctx->value_lists()->value_list(); // a vector
        for(int i = 0; i < value_lists.size(); i++) {
            // for each value list
            std::vector<void*> valueList;
            std::vector<DB_LIST_TYPE> listType;
            auto values = value_lists[i]->value();
            for(int j = 0; j < values.size(); j++) {
                // for each value
                switch (values[j]->start->getType())
                {
                case SQLParser::Integer:
                    int intValue = stoi(values[j]->Integer()->getSymbol()->getText());
                    valueList.push_back((void*)&intValue);
                    listType.push_back(DB_LIST_INT);
                    break;
                case SQLParser::String:
                    valueList.push_back((void*)values[j]->String()->getSymbol()->getText().c_str());
                    listType.push_back(DB_LIST_CHAR);
                    break;
                case SQLParser::Float:
                    float floatValue = stof(values[j]->Float()->getSymbol()->getText());
                    valueList.push_back((void*)&floatValue);
                    listType.push_back(DB_LIST_FLOAT);
                    break;
                default:
                    void* p = NULL;
                    valueList.push_back(p);
                    listType.push_back(DB_LIST_NULL);
                    break;
                }
            }
            dbInsert->valueLists.push_back(valueList);
            dbInsert->valueListsType.push_back(listType);
        }
        databaseManager->insertRecords(ctx->Identifier()->getSymbol()->getText(), dbInsert);
        return visitChildren(ctx);
    }

    std::any visitDelete_from_table(SQLParser::Delete_from_tableContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitUpdate_table(SQLParser::Update_tableContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitSelect_table_(SQLParser::Select_table_Context *ctx) override {
        // TODO
        return visitChildren(ctx);
    }
    
    std::any visitSelect_table(SQLParser::Select_tableContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitAlter_add_index(SQLParser::Alter_add_indexContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitAlter_drop_index(SQLParser::Alter_drop_indexContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitAlter_table_drop_pk(SQLParser::Alter_table_drop_pkContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitAlter_table_drop_foreign_key(SQLParser::Alter_table_drop_foreign_keyContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitAlter_table_add_pk(SQLParser::Alter_table_add_pkContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitAlter_table_add_foreign_key(SQLParser::Alter_table_add_foreign_keyContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitAlter_table_add_unique(SQLParser::Alter_table_add_uniqueContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitField_list(SQLParser::Field_listContext *ctx) override {
        fprintf(stderr, "Visit Field List.\n");
        return visitChildren(ctx);
    }

    std::any visitNormal_field(SQLParser::Normal_fieldContext *ctx) override {
        fprintf(stderr, "Visit Normal Field.\n");
        return visitChildren(ctx);
    }

    std::any visitPrimary_key_field(SQLParser::Primary_key_fieldContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitForeign_key_field(SQLParser::Foreign_key_fieldContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitType_(SQLParser::Type_Context *ctx) override {
        fprintf(stderr, "Visit Type.\n");
        fprintf(stderr, "Type is %s.\n", ctx->start->getText().c_str());
        return visitChildren(ctx);
    }

    std::any visitValue_lists(SQLParser::Value_listsContext *ctx) override {
        fprintf(stderr, "Visit Value Lists.\n");
        return visitChildren(ctx);
    }

    std::any visitValue_list(SQLParser::Value_listContext *ctx) override {
        fprintf(stderr, "Visit Value List.\n");
        return visitChildren(ctx);
    }

    std::any visitValue(SQLParser::ValueContext *ctx) override {
        fprintf(stderr, "Visit Value.\n");
        return visitChildren(ctx);
    }
    
    std::any visitWhere_and_clause(SQLParser::Where_and_clauseContext *ctx) override {
        // optimizeWhereClause(ctx, databaseManager);
        return visitChildren(ctx);
    }

    std::any visitWhere_operator_expression(SQLParser::Where_operator_expressionContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitWhere_operator_select(SQLParser::Where_operator_selectContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitWhere_null(SQLParser::Where_nullContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitWhere_in_list(SQLParser::Where_in_listContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitWhere_in_select(SQLParser::Where_in_selectContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitWhere_like_string(SQLParser::Where_like_stringContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitColumn(SQLParser::ColumnContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitExpression(SQLParser::ExpressionContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitSet_clause(SQLParser::Set_clauseContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitSelectors(SQLParser::SelectorsContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitSelector(SQLParser::SelectorContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitIdentifiers(SQLParser::IdentifiersContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitOperator_(SQLParser::Operator_Context *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

    std::any visitAggregator(SQLParser::AggregatorContext *ctx) override {
        // TODO
        return visitChildren(ctx);
    }

};

#endif