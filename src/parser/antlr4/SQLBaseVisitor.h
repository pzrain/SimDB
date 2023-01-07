
// Generated from src/parser/antlr4/SQL.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "SQLVisitor.h"


/**
 * This class provides an empty implementation of SQLVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SQLBaseVisitor : public SQLVisitor {
public:

  virtual std::any visitProgram(SQLParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(SQLParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreate_db(SQLParser::Create_dbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDrop_db(SQLParser::Drop_dbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShow_dbs(SQLParser::Show_dbsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUse_db(SQLParser::Use_dbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShow_tables(SQLParser::Show_tablesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShow_indexes(SQLParser::Show_indexesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLoad_data(SQLParser::Load_dataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDump_data(SQLParser::Dump_dataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreate_table(SQLParser::Create_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDrop_table(SQLParser::Drop_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDescribe_table(SQLParser::Describe_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsert_into_table(SQLParser::Insert_into_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDelete_from_table(SQLParser::Delete_from_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUpdate_table(SQLParser::Update_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelect_table_(SQLParser::Select_table_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelect_table(SQLParser::Select_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlter_add_index(SQLParser::Alter_add_indexContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlter_drop_index(SQLParser::Alter_drop_indexContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlter_table_drop_pk(SQLParser::Alter_table_drop_pkContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlter_table_drop_foreign_key(SQLParser::Alter_table_drop_foreign_keyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlter_table_add_pk(SQLParser::Alter_table_add_pkContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlter_table_add_foreign_key(SQLParser::Alter_table_add_foreign_keyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlter_table_add_unique(SQLParser::Alter_table_add_uniqueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlter_table_drop_unique(SQLParser::Alter_table_drop_uniqueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitField_list(SQLParser::Field_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNormal_field(SQLParser::Normal_fieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimary_key_field(SQLParser::Primary_key_fieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForeign_key_field(SQLParser::Foreign_key_fieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType_(SQLParser::Type_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_lists(SQLParser::Value_listsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_list(SQLParser::Value_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue(SQLParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhere_and_clause(SQLParser::Where_and_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhere_operator_expression(SQLParser::Where_operator_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhere_operator_select(SQLParser::Where_operator_selectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhere_null(SQLParser::Where_nullContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhere_in_list(SQLParser::Where_in_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhere_in_select(SQLParser::Where_in_selectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhere_like_string(SQLParser::Where_like_stringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumn(SQLParser::ColumnContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(SQLParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSet_clause(SQLParser::Set_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectors(SQLParser::SelectorsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelector(SQLParser::SelectorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentifiers(SQLParser::IdentifiersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOperator_(SQLParser::Operator_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAggregator(SQLParser::AggregatorContext *ctx) override {
    return visitChildren(ctx);
  }


};

