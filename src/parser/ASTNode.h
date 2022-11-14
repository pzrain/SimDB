#ifndef __ASTNODE_H__
#define __ASTNODE_H__
#include <vector>
#include <cstring>
#include "ASTBase.h"
#include "antlr/SQLBaseVisitor.h"

class Identifier: public ASTBase {
public:
    std::string ident;
    Identifier(std::string ident_):ASTBase("identifier"), ident(ident_) {}

    void accept(SQLBaseVisitor* visitor) override {
        
    }
};

class Integer: public ASTBase {
public:
    int value;
    Integer(int value_):ASTBase("integer"), value(value_) {}
};

class String: public ASTBase {
public:
    std::string value;
    String(std::string value_):ASTBase("string"), value(value_) {}
};

class Float: public ASTBase {
public:
    float value;
    Float(float value_):ASTBase("float"), value(value_) {}
};

class Whitespace: public ASTBase {
public:
    Whitespace(): ASTBase("whitespace") {}
};

class Annotation: public ASTBase {
public:
    std::string annotation;
    Annotation(std::string annotation_):ASTBase("annotation"), annotation(annotation_) {}
};

class Statement: public ASTBase {
public:
    Statement(std::string name): ASTBase(name) {}
    virtual ~Statement();
};

class DBStatement: public Statement {
public:  
    DBStatement(): Statement("dbstatement") {}

    ~DBStatement() {}
};

class IOStatement: public Statement {
public:
    IOStatement(): Statement("iostatement") {}

    ~IOStatement() {}
};

class TABLEStatement: public Statement {
public:
    TABLEStatement(): Statement("dbstatement") {}

    ~TABLEStatement() {}
};

class ALTERStatement: public Statement {
public:
    ALTERStatement(): Statement("dbstatement") {}

    ~ALTERStatement() {}
};

class Program: public ASTBase {
public:
    std::vector<Statement*>* statements;
    Program(std::vector<Statement*>* statements_): ASTBase("program") {
        statements = statements_;
    }
    ~Program() {
        int siz = statements->size();
        for (int i = 0; i < siz; i++) {
            delete statements->at(i);
        }
        delete statements;
    }
};

#endif