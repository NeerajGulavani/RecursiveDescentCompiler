#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <iostream>

enum VarType { TYPE_INT, TYPE_FLOAT, TYPE_UNKNOWN };

struct Symbol {
    std::string name;
    VarType type;
    int scopeLevel;
};

class SymbolTable {
private:
    std::unordered_map<std::string, Symbol> table;

public:
    bool insert(const std::string& name, VarType type) {
        if (table.find(name) != table.end()) {
            return false; // Redeclaration error
        }
        table[name] = {name, type, 0};
        return true;
    }

    Symbol* lookup(const std::string& name) {
        auto it = table.find(name);
        if (it != table.end()) {
            return &(it->second);
        }
        return nullptr; // Undeclared error
    }

   void printTable() {
        for (const auto& pair : table) {
            std::string typeStr = (pair.second.type == TYPE_INT) ? "int" : "float";
            // Emit structured rows for UI parsing cleanly
            std::cout << "[TABLE_ROW] " << pair.first << " | " << typeStr << " | Global\n";
        }
    }
};

#endif