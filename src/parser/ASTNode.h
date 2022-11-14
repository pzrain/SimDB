#ifndef __ASTNODE_H__
#define __ASTNODE_H__
#include <vector>
#include <cstring>
#include "antlr/SQLBaseVisitor.h"

class Identifier {
public:
    std::string ident;
    Identifier(std::string ident_): ident(ident_) {}
};

class Integer {
public:
    int value;
    Integer(int value_): value(value_) {}
};

class String {
public:
    std::string value;
    String(std::string value_): value(value_) {}
}

class Float {
public:
    float value;
    Float(float value_): value(value_) {}
}

class Whitespace {
public:
    Whitespace() {}
}

class Annotation {
public:
    std::string annotation;
    Annotation(std::string annotation_): annotation(annotation_) {}
}

class Statement {
public:
    virtual ~Statement();
};

class DBStatement: public Statement {
public:  
    ~DBStatement() {}
};

class IOStatement: public Statement {
public:
    ~IOStatement() {}
}

class TABLEStatement: public Statement {
public:
    ~TABLEStatement() {}
}

class ALTERStatement: public Statement {
public:
    ~ALTERStatement() {}
}

class Program {
public:
    std::vector<Statement*>* statements;
    Program(std::vector<Statement*>* statements_) {
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