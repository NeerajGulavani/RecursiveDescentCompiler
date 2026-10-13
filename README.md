# C++ Recursive-Descent Compiler with Semantic Analysis & TAC Generator

A lightweight, robust compiler backend written in C++17 featuring lexical analysis via Flex, recursive-descent parsing, dynamic symbol table management, static type checking, and Three-Address Code (TAC) intermediate code generation.

Includes an interactive local web dashboard built with Python/Flask to visualize symbol table state, compilation errors, and TAC output in real time.

---

## 🌟 Key Features

* **Lexical & Syntactic Analysis:** Tokenization powered by Flex (`lexer.l`) with a custom C++ recursive-descent parser.
* **Symbol Table Architecture (`symbol_table.h`):** Efficient `std::unordered_map` storage mapping identifiers to type, scope, and variable metadata.
* **Semantic Analysis & Error Handling:**
  * **Redeclaration Protection:** Intercepts duplicate variable declarations within the same scope.
  * **Undeclared Usage Protection:** Flags references to non-existent or out-of-scope variables.
  * **Strict Type Checking:** Compile-time type mismatch detection on assignments (e.g., blocking `float`-to-`int` implicit narrowing).
* **Intermediate Code Generation (TAC):** Linear Three-Address Code translation with automatic temporary variable (`t1`, `t2`, ...) generation.
* **Localhost Studio Dashboard:** Python/Flask wrapper offering a web visualizer for source code input, TAC inspection, and live symbol table registry viewing.

---

## 📁 Repository Structure

```text
├── lexer.l              # Flex scanner rules & token definitions
├── parser.cpp           # C++ Recursive-Descent Parser
├── symbol_table.h       # Symbol Table & type checking metadata structures
├── app.py               # Flask local web server interface
├── templates/
│   └── index.html       # Visual studio UI layout
├── static/              # Dashboard styling & scripts
└── README.md            # Project documentation
```

---

## 🚀 Getting Started

### Prerequisites

* **C++ Compiler:** `g++` (supports C++17 or higher)
* **Lexer Generator:** `flex`
* **Python Engine:** Python 3.8+ (for running the Flask Dashboard)
* **Flask Package:** `pip install flask`

### Build & Run via Terminal

1. **Generate the Lexer:**
   ```bash
   flex lexer.l
   ```
   *This command processes `lexer.l` and outputs the scanner C file `lex.yy.c`.*

2. **Compile the Backend to Create the Executable Binary (`compiler`):**
   ```bash
   g++ -std=c++17 lex.yy.c parser.cpp -o compiler
   ```
   *This links the generated lexer with the recursive-descent parser to build the `./compiler` executable binary.*

3. **Run the Compiler with a Source File:**
   ```bash
   ./compiler input.txt
   ```

---

## 💻 Running the Localhost Dashboard

To interactively run source code and view TAC generation alongside the live Symbol Table:

1. **Build the compiler binary first** (Step 1 and Step 2 above).
2. **Launch the Flask Server:**
   ```bash
   python3 app.py
   ```
3. **Open Browser:** Navigate to `http://localhost:5000` to view the IDE.

---

## 🗺️ Roadmap to 100% Implementation

- [ ] **Error Recovery:** Panic-mode parser synchronization for multi-error diagnostics.
- [ ] **Target Code Generation:** Final backend pass translating TAC into assembly code.

---

## 👤 Author

* **Neeraj Gulavani** – Computer Science Engineering Student
