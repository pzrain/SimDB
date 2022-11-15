#include <string>
#include "antlr4/SQLLexer.h"
#include "MySQLVisitor.h"
#include <iostream>

std::string parse(std::string SQL) {
    antlr4::ANTLRInputStream sInputStream(SQL);
    // Lexer
    SQLLexer iLexer(&sInputStream);
    antlr4::CommonTokenStream sTokenStream(&iLexer);
    // Parser
    SQLParser iParser(&sTokenStream);
    auto iTree = iParser.program();
    return "Hello World!";
}

int main() {
    std::cout << parse("CREATE DATABASE mysql;") << std::endl;
    return 0;
}