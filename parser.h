#ifndef PARSER_H
#define PARSER_H

#include <cstdio>
#include <string>
#pragma once
#include <vector>
#include <string>

// Expose the global TAC instructions container across files
extern std::vector<std::string> tac_instructions;

enum TokenType {
    TOK_EOF = 0, 
    TOK_INT,
    TOK_FLOAT,
    TOK_ID,
    TOK_NUMBER,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULTIPLY,
    TOK_DIVIDE,
    TOK_ASSIGN,
    TOK_SEMICOLON,
    TOK_LPAREN,
    TOK_RPAREN
};

extern FILE* yyin;
extern int yylex();
extern char* yytext;
extern int yylineno;

const char* tokenName(TokenType token);
void parseProgram();

#endif