#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cctype>

class TargetCodeGenerator {
private:
    std::unordered_map<std::string, int> var_offsets;
    int next_slot = 8;

    // Assigns a unique stack offset for every unique variable or temporary
    int getSlot(const std::string& name) {
        if (var_offsets.find(name) == var_offsets.end()) {
            var_offsets[name] = next_slot;
            next_slot += 8; // Move down 8 bytes for each new variable/temp
        }
        return var_offsets[name];
    }

    // Checks if a string represents a numeric literal
    bool isNumber(const std::string& s) {
        if (s.empty()) return false;
        size_t start = (s[0] == '-' || s[0] == '+') ? 1 : 0;
        if (start == s.size()) return false;
        bool hasDecimal = false;
        for (size_t i = start; i < s.size(); ++i) {
            if (s[i] == '.') {
                if (hasDecimal) return false;
                hasDecimal = true;
            } else if (!std::isdigit(s[i])) {
                return false;
            }
        }
        return true;
    }

public:
    void generateAssembly(const std::vector<std::string>& tac_instructions, const std::string& output_filename) {
        var_offsets.clear();
        next_slot = 8;

        std::ofstream out(output_filename);
        if (!out.is_open()) {
            std::cerr << "[Error] Could not open " << output_filename << " for writing target assembly.\n";
            return;
        }

        // Standard x86-64 Assembly Header
        out << "; ==========================================\n";
        out << "; Generated Assembly from Compiler TAC Backend\n";
        out << "; Target Architecture: x86-64 (Intel Syntax)\n";
        out << "; ==========================================\n\n";
        out << ".global main\n";
        out << ".text\n\n";
        out << "main:\n";
        out << "    push rbp\n";
        out << "    mov rbp, rsp\n";
        out << "    sub rsp, 128  ; Allocate stack space for variables/temporaries\n\n";

        int line_num = 1;
        for (const auto& tac : tac_instructions) {
            out << "    ; [TAC Line " << line_num++ << "] " << tac << "\n";

            std::stringstream ss(tac);
            std::string result, equals, arg1, op, arg2;
            ss >> result >> equals;

            if (equals == "=") {
                if (!(ss >> arg1 >> op >> arg2)) {
                    // Case 1: Simple assignment e.g., a = 5 or x = t1
                    if (isNumber(arg1)) {
                        out << "    mov rax, " << arg1 << "       ; load constant value\n";
                    } else {
                        out << "    mov rax, [rbp - " << getSlot(arg1) << "]       ; load value from '" << arg1 << "'\n";
                    }
                    
                    int dest_slot = getSlot(result);
                    out << "    mov [rbp - " << dest_slot << "], rax      ; store into variable slot '" << result << "'\n";
                } else {
                    // Case 2: Binary operation e.g., t1 = a + b
                    if (isNumber(arg1)) {
                        out << "    mov rax, " << arg1 << "       ; load constant left operand\n";
                    } else {
                        out << "    mov rax, [rbp - " << getSlot(arg1) << "]       ; load left operand '" << arg1 << "'\n";
                    }

                    // Prepare right operand representation for x86
                    std::string right_op = isNumber(arg2) ? arg2 : ("[rbp - " + std::to_string(getSlot(arg2)) + "]");
                    
                    if (op == "+") {
                        out << "    add rax, " << right_op << "   ; add right operand\n";
                    } else if (op == "-") {
                        out << "    sub rax, " << right_op << "   ; subtract right operand\n";
                    } else if (op == "*") {
                        out << "    imul rax, " << right_op << "  ; multiply right operand\n";
                    } else if (op == "/") {
                        out << "    cqo\n";
                        out << "    mov rcx, " << right_op << "\n";
                        out << "    idiv rcx          ; divide by right operand\n";
                    }
                    
                    int dest_slot = getSlot(result);
                    out << "    mov [rbp - " << dest_slot << "], rax      ; save operation result into temporary '" << result << "'\n";
                }
            }
            out << "\n";
        }

        // Assembly Footer / Return routine
        out << "    ; Function Epilogue & Exit\n";
        out << "    mov rax, 0              ; return 0 status\n";
        out << "    mov rsp, rbp\n";
        out << "    pop rbp\n";
        out << "    ret\n";

        out.close();
        std::cout << "\n[Target Code Gen] Successfully compiled TAC into assembly -> " << output_filename << "\n";
    }
};