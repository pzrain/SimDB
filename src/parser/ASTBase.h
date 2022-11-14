#ifndef __ASTBASE_H__
#define __ASTBASE_H__
#include <iostream>
#include <cstring>
#include "antlr/SQLBaseVisitor.h"

class ASTBase{
private:
    std::string name;
public:
    ASTBase(std::string name_) {
        name = name_;
    }

    virtual ~ASTBase() {}

    virtual void toString() = 0;

    virtual void accept(SQLBaseVisitor* visitor) = 0;
};

#endif