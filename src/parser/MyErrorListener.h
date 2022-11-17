#ifndef __MY_PARSE_ERROR_LISTENER_H__
#define __MY_PARSE_ERROR_LISTENER_H__

#include "antlr4-runtime.h"
#include <cstdio>
using namespace antlr4;

class MyANTLRParserErrorListener : public ANTLRErrorListener {
private:
    int* errorNum;
public:

    MyANTLRParserErrorListener(int* errorNum_): errorNum(errorNum_) {}

    void syntaxError(Recognizer *recognizer, Token *offendingSymbol, size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {
        printf("[Parser Error] %s\n", msg.c_str());
        (*errorNum)++;
    }

    void reportAmbiguity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, atn::ATNConfigSet *configs) override {}

    void reportAttemptingFullContext(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, const antlrcpp::BitSet &conflictingAlts, atn::ATNConfigSet *configs) override {}

    void reportContextSensitivity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, size_t prediction, atn::ATNConfigSet *configs) override {}
};

class MyANTLRLexerErrorListener : public ANTLRErrorListener {
private:
    int* errorNum;
public:

    MyANTLRLexerErrorListener(int* errorNum_): errorNum(errorNum_) {}

    void syntaxError(Recognizer *recognizer, Token *offendingSymbol, size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {
        printf("[Lexer Error] %s\n", msg.c_str());
        (*errorNum)++;
    }

    void reportAmbiguity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, atn::ATNConfigSet *configs) override {}

    void reportAttemptingFullContext(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, const antlrcpp::BitSet &conflictingAlts, atn::ATNConfigSet *configs) override {}

    void reportContextSensitivity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, size_t prediction, atn::ATNConfigSet *configs) override {}
};

#endif