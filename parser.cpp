#include "parser.h"
#include "symbol_table.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;



// Define the global TAC instructions container
std::vector<std::string> tac_instructions;

// --- FORWARD DECLARATIONS ---
static void nextToken();
static string expression();
static string term();
static string factor();

static int syntax_error_count = 0;

static int currentToken = TOK_EOF;
static string currentLexeme;

// Global Symbol Table instance for the compiler frontend
static SymbolTable symbolTables;

// TAC temporary counter
static int tempCounter = 1;

static string newTemp() {
    return "t" + to_string(tempCounter++);
}

static void emitTAC(const string& result, const string& arg1, const string& op, const string& arg2) {
    string tac_line = "";
    if (op.empty()) {
        // Assignment format: x = arg1
        tac_line = result + " = " + arg1;
        cout << "  [TAC] " << tac_line << "\n";
    } else {
        // Binary operation format: t1 = arg1 op arg2
        tac_line = result + " = " + arg1 + " " + op + " " + arg2;
        cout << "  [TAC] " << tac_line << "\n";
    }
    // Store it in our vector for Target Code Generation!
    tac_instructions.push_back(tac_line);
}

static void synchronize() {
    syntax_error_count++;
    // Panic-mode recovery: consume tokens until ';' or EOF
    while (currentToken != TOK_EOF) {
        if (currentToken == TOK_SEMICOLON) {
            nextToken(); // Skip the semicolon and resume
            return;
        }
        nextToken();
    }
}

static void nextToken() {
    int token = yylex();

    if (token <= 0) {
        currentToken = TOK_EOF;
        currentLexeme = "<EOF>";
    } else {
        currentToken = token;
        currentLexeme = (yytext != nullptr) ? yytext : "";
    }
}

const char* tokenName(TokenType token) {
    switch (token) {
        case TOK_INT:       return "int";
        case TOK_FLOAT:     return "float";
        case TOK_ID:        return "identifier";
        case TOK_NUMBER:    return "number";
        case TOK_PLUS:      return "+";
        case TOK_MINUS:     return "-";
        case TOK_MULTIPLY:  return "*";
        case TOK_DIVIDE:    return "/";
        case TOK_ASSIGN:    return "=";
        case TOK_SEMICOLON: return ";";
        case TOK_LPAREN:    return "(";
        case TOK_RPAREN:    return ")";
        case TOK_EOF:       return "end of file";
        default:            return "unknown token";
    }
}

[[noreturn]] static void syntaxError(const string& message) {
    cout << "\n----------------------------------------\n";
    cout << " SYNTAX / SEMANTIC ERROR (Line " << yylineno << ")\n";
    cout << " Message: " << message << "\n";
    cout << "----------------------------------------\n";
    throw runtime_error(message);
}

static void match(TokenType expected) {
    if (currentToken != expected) {
        syntaxError(
            string("expected '") + tokenName(expected) +
            "' but found '" + currentLexeme + "'"
        );
    }
    nextToken();
}

// Forward declarations with string return type for TAC evaluation
static string expression();
static string term();
static string factor();

static string factor() {
    string val = currentLexeme;
    if (currentToken == TOK_ID) {
        string idName = currentLexeme;
        // Novelty Semantic Check: Ensure variable is declared before use
        if (symbolTables.lookup(idName) == nullptr) {
            syntaxError("Semantic Error: Variable '" + idName + "' used before declaration");
        }
        match(TOK_ID);
    }
    else if (currentToken == TOK_NUMBER) {
        match(TOK_NUMBER);
    }
    else if (currentToken == TOK_LPAREN) {
        match(TOK_LPAREN);
        val = expression();
        match(TOK_RPAREN);
    }
    else {
        syntaxError(
            string("expected identifier, number, or '(' but found '") +
            currentLexeme + "'"
        );
    }
    return val;
}

static string termPrime(string lhs) {
    while (currentToken == TOK_MULTIPLY || currentToken == TOK_DIVIDE) {
        string op = currentLexeme;
        if (currentToken == TOK_MULTIPLY) {
            match(TOK_MULTIPLY);
        } else {
            match(TOK_DIVIDE);
        }
        string rhs = factor();
        string temp = newTemp();
        emitTAC(temp, lhs, op, rhs);
        lhs = temp;
    }
    return lhs;
}

static string term() {
    string lhs = factor();
    return termPrime(lhs);
}

static string expressionPrime(string lhs) {
    while (currentToken == TOK_PLUS || currentToken == TOK_MINUS) {
        string op = currentLexeme;
        if (currentToken == TOK_PLUS) {
            match(TOK_PLUS);
        } else {
            match(TOK_MINUS);
        }
        string rhs = term();
        string temp = newTemp();
        emitTAC(temp, lhs, op, rhs);
        lhs = temp;
    }
    return lhs;
}

static string expression() {
    string lhs = term();
    return expressionPrime(lhs);
}

static void declaration() {
    VarType varType = TYPE_UNKNOWN;

    if (currentToken == TOK_INT) {
        varType = TYPE_INT;
        match(TOK_INT);
    }
    else if (currentToken == TOK_FLOAT) {
        varType = TYPE_FLOAT;
        match(TOK_FLOAT);
    }
    else {
        syntaxError("expected type 'int' or 'float'");
    }

    string varName = currentLexeme;
    match(TOK_ID);

    // Novelty Semantic Check: Prevent duplicate variable redeclarations
    if (!symbolTables.insert(varName, varType)) {
        syntaxError("Semantic Error: Redeclaration of variable '" + varName + "'");
    }

    // Check if there is an initializer (e.g., int a = 5;)
    if (currentToken == TOK_ASSIGN) {
        match(TOK_ASSIGN);
        string exprResult = expression();
        // Emit TAC for the initialization assignment
        emitTAC(varName, exprResult, "", "");
    }

    if (currentToken != TOK_SEMICOLON) {
        syntaxError(
            "expected ';' after declaration, but found '" +
            currentLexeme + "'"
        );
    }

    match(TOK_SEMICOLON);
}

static void assignment() {
    string varName = currentLexeme;

    // 1. Semantic Check: Ensure variable is declared and get its symbol entry
    Symbol* sym = symbolTables.lookup(varName);
    if (sym == nullptr) {
        syntaxError("Semantic Error: Use of undeclared variable '" + varName + "'");
    }
    VarType targetType = sym->type;

    match(TOK_ID);
    match(TOK_ASSIGN);

    // 2. Capture RHS type clues before evaluating the expression
    bool isFloatLiteral = (currentToken == TOK_NUMBER && currentLexeme.find('.') != string::npos);
    
    Symbol* rhsSym = (currentToken == TOK_ID) ? symbolTables.lookup(currentLexeme) : nullptr;
    bool isFloatVar = (rhsSym != nullptr && rhsSym->type == TYPE_FLOAT);

    // Evaluate the expression and get its result/temporary
    string exprResult = expression();

    // 3. Strict Type-Checking Rule: Block float-to-int unsafe assignments
    if (targetType == TYPE_INT && (isFloatLiteral || isFloatVar)) {
        syntaxError("Semantic Type Error: Cannot assign 'float' value to 'int' variable '" + varName + "'");
    }

    if (currentToken != TOK_SEMICOLON) {
        syntaxError(
            "expected ';' after assignment, but found '" +
            currentLexeme + "'"
        );
    }
    match(TOK_SEMICOLON);

    // Emit TAC for the assignment
    emitTAC(varName, exprResult, "", "");
}

static void statement() {
    try {
        if (currentToken == TOK_INT || currentToken == TOK_FLOAT) {
            declaration();
        }
        else if (currentToken == TOK_ID) {
            assignment();
        }
        else {
            syntaxError(
                string("expected declaration or assignment, but found '") +
                currentLexeme + "'"
            );
        }
    } catch (const exception& e) {
        // Recover and continue parsing subsequent statements instead of crashing
        synchronize();
    }
}

static void statementList() {
    while (currentToken != TOK_EOF) {
        statement();
    }
}

void parseProgram() {
    nextToken(); // Fetch initial token

    if (currentToken == TOK_EOF) {
        syntaxError("Input file is empty");
    }

    statementList();

    if (syntax_error_count > 0) {
        cout << "\n========================================\n";
        cout << " PARSING COMPLETED WITH " << syntax_error_count << " ERROR(S) RECOVERED\n";
        cout << "========================================\n";
    } else {
        // Print Symbol Table on successful parse with zero errors
        symbolTables.printTable();

        cout << "\n========================================\n";
        cout << " PARSING & SEMANTIC ANALYSIS SUCCESSFUL\n";
        cout << "========================================\n";
    }
}