CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

all: compiler

lex.yy.c: lexer.l
	flex lexer.l

compiler: lex.yy.c parser.cpp parser.h symbol_table.h code_generator.h main.cpp
	$(CXX) $(CXXFLAGS) parser.cpp main.cpp lex.yy.c -lfl -o compiler

clean:
	rm -f lex.yy.c compiler output.s