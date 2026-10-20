#include <iostream>
#include <cstdio>
#include "parser.h"
#include "code_generator.h"
using namespace std;

int main(int argc, char* argv[]) {
    cout << "========================================\n";
    cout << " Recursive-Descent Compiler \n";
    cout << "========================================\n\n";

    if (argc > 1) {
        FILE* file = fopen(argv[1], "r");
        if (!file) {
            cerr << "Error: Could not open file '" << argv[1] << "'\n";
            return 1;
        }
        yyin = file;
    } else {
        yyin = stdin;
    }

    // 1. Parse, Type-Check, and Emit TAC
    parseProgram();

    // 2. Translate TAC into Target Assembly
    TargetCodeGenerator codeGen;
    codeGen.generateAssembly(tac_instructions, "output.s");

    if (argc > 1 && yyin) {
        fclose(yyin);
    }

    return 0;
}