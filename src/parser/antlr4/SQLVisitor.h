
// Generated from SQL.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime/antlr4-runtime.h"
#include "SQLParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SQLParser.
 */
class  SQLVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SQLParser.
   */
    virtual std::any visitProgram(SQLParser::ProgramContext *context) = 0;

    virtual std::any visitStatement(SQLParser::StatementContext *context) = 0;

    virtual std::any visitCreate_db(SQLParser::Create_dbContext *context) = 0;

    virtual std::any visitDrop_db(SQLParser::Drop_dbContext *context) = 0;

    virtual std::any visitShow_dbs(SQLParser::Show_dbsContext *context) = 0;

    virtual std::any visitUse_db(SQLParser::Use_dbContext *context) = 0;

    virtual std::any visitShow_tables(SQLParser::Show_tablesContext *context) = 0;

    virtual std::any visitShow_indexes(SQLParser::Show_indexesContext *context) = 0;

    virtual std::any visitLoad_data(SQLParser::Load_dataContext *context) = 0;

    virtual std::any visitDump_data(SQLParser::Dump_dataContext *context) = 0;

    virtual std::any visitCreate_table(SQLParser::Create_tableContext *context) = 0;

    virtual std::any visitDrop_table(SQLParser::Drop_tableContext *context) = 0;

    virtual std::any visitDescribe_table(SQLParser::Describe_tableContext *context) = 0;

    virtual std::any visitInsert_into_table(SQLParser::Insert_into_tableContext *context) = 0;

    virtual std::any visitDelete_from_table(SQLParser::Delete_from_tableContext *context) = 0;

    virtual std::any visitUpdate_table(SQLParser::Update_tableContext *context) = 0;

    virtual std::any visitSelect_table_(SQLParser::Select_table_Context *context) = 0;

    virtual std::any visitSelect_table(SQLParser::Select_tableContext *context) = 0;

    virtual std::any visitAlter_add_index(SQLParser::Alter_add_indexContext *context) = 0;

    virtual std::any visitAlter_drop_index(SQLParser::Alter_drop_indexContext *context) = 0;

    virtual std::any visitAlter_table_drop_pk(SQLParser::Alter_table_drop_pkContext *context) = 0;

    virtual std::any visitAlter_table_drop_foreign_key(SQLParser::Alter_table_drop_foreign_keyContext *context) = 0;

    virtual std::any visitAlter_table_add_pk(SQLParser::Alter_table_add_pkContext *context) = 0;

    virtual std::any visitAlter_table_add_foreign_key(SQLParser::Alter_table_add_foreign_keyContext *context) = 0;

    virtual std::any visitAlter_table_add_unique(SQLParser::Alter_table_add_uniqueContext *context) = 0;

    virtual std::any visitField_list(SQLParser::Field_listContext *context) = 0;

    virtual std::any visitNormal_field(SQLParser::Normal_fieldContext *context) = 0;

    virtual std::any visitPrimary_key_field(SQLParser::Primary_key_fieldContext *context) = 0;

    virtual std::any visitForeign_key_field(SQLParser::Foreign_key_fieldContext *context) = 0;

    virtual std::any visitType_(SQLParser::Type_Context *context) = 0;

    virtual std::any visitValue_lists(SQLParser::Value_listsContext *context) = 0;

    virtual std::any visitValue_list(SQLParser::Value_listContext *context) = 0;

    virtual std::any visitValue(SQLParser::ValueContext *context) = 0;

    virtual std::any visitWhere_and_clause(SQLParser::Where_and_clauseContext *context) = 0;

    virtual std::any visitWhere_operator_expression(SQLParser::Where_operator_expressionContext *context) = 0;

    virtual std::any visitWhere_operator_select(SQLParser::Where_operator_selectContext *context) = 0;

    virtual std::any visitWhere_null(SQLParser::Where_nullContext *context) = 0;

    virtual std::any visitWhere_in_list(SQLParser::Where_in_listContext *context) = 0;

    virtual std::any visitWhere_in_select(SQLParser::Where_in_selectContext *context) = 0;

    virtual std::any visitWhere_like_string(SQLParser::Where_like_stringContext *context) = 0;

    virtual std::any visitColumn(SQLParser::ColumnContext *context) = 0;

    virtual std::any visitExpression(SQLParser::ExpressionContext *context) = 0;

    virtual std::any visitSet_clause(SQLParser::Set_clauseContext *context) = 0;

    virtual std::any visitSelectors(SQLParser::SelectorsContext *context) = 0;

    virtual std::any visitSelector(SQLParser::SelectorContext *context) = 0;

    virtual std::any visitIdentifiers(SQLParser::IdentifiersContext *context) = 0;

    virtual std::any visitOperator_(SQLParser::Operator_Context *context) = 0;

    virtual std::any visitAggregator(SQLParser::AggregatorContext *context) = 0;


};

