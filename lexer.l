%{
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
%}

%option nounput noyywrap yylineno

%%
"int"           { return TOK_INT; }
"float"         { return TOK_FLOAT; }
[a-zA-Z_][a-zA-Z0-9_]* { return TOK_ID; }
[0-9]+(\.[0-9]+)?      { return TOK_NUMBER; }
"+"             { return TOK_PLUS; }
"-"             { return TOK_MINUS; }
"*"             { return TOK_MULTIPLY; }
"/"             { return TOK_DIVIDE; }
"="             { return TOK_ASSIGN; }
";"             { return TOK_SEMICOLON; }
"("             { return TOK_LPAREN; }
")"             { return TOK_RPAREN; }

[ \t\r\n]+      { /* Ignore all whitespace and newlines */ }

.               { 
                  fprintf(stderr, "Lexical Error at line %d: Unrecognized character '%s'\n", yylineno, yytext);
                  exit(1);
                }
%%