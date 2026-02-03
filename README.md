# TheoryProject - Python to JavaScript Translator

A C++ compiler implementation that translates Python code to JavaScript, featuring lexical analysis, parsing, semantic analysis, code generation, and interactive automata visualization.

## Features

- **Lexical Analysis**: Tokenizes Python source code into a stream of tokens
- **Syntax Analysis**: Builds an Abstract Syntax Tree (AST) from tokens using recursive descent parsing
- **Semantic Analysis**: Validates code semantics and type checking
- **Code Generation**: Generates equivalent JavaScript code from the AST
- **Automata Visualization**: Interactive visualization of lexer and parser state machines using SFML graphics

## Architecture

The translator implements a multi-phase compilation pipeline:

1. **Lexer** (`lexer.cpp/h`) - Performs lexical analysis to tokenize source code
2. **Parser** (`parser.cpp/h`) - Builds an AST using recursive descent parsing
3. **Semantic Analyzer** (`semantic_analyzer.cpp/h`) - Validates semantic correctness
4. **Code Generator** (`js_code_generator.cpp/h`) - Generates JavaScript output
5. **Automata Visualizer** (`automata_visualizer.cpp/h`) - Visualizes compilation phases

### Supported Python Features

- Variables and assignments
- Functions and classes
- Control flow (if/elif/else, while, for loops)
- Exception handling (try/except)
- Import statements
- Data structures (lists, dictionaries)
- Operators (arithmetic, logical, comparison)
- String formatting (including f-strings)

## Prerequisites

- **CMake** 3.10 or higher
- **C++17** compatible compiler (GCC, Clang, or MSVC)
- **SFML 2.5** or higher (for visualization features)

### Installing SFML

**Ubuntu/Debian:**
```bash
sudo apt-get install libsfml-dev
```

**macOS (using Homebrew):**
```bash
brew install sfml
```

**Windows:**
Download SFML from [https://www.sfml-dev.org/download.php](https://www.sfml-dev.org/download.php)

## Building the Project

### Using CMake

1. Clone the repository:
```bash
git clone https://github.com/hadinajem52/theoryproject.git
cd theoryproject
```

2. Create a build directory:
```bash
mkdir -p build
cd build
```

3. Configure and build:
```bash
cmake ..
cmake --build .
```

This will create the `TheoryProject` executable.

## Usage

### Basic Translation

Translate a Python file to JavaScript:

```bash
./TheoryProject input.py output.js
```

### With Visualization

Run the translator with interactive automata visualization:

```bash
./TheoryProject input.py output.js --visualize
```

This will open a window showing the state transitions of the lexer and parser as they process the input.

### Using CMake Target

```bash
cmake --build . --target run_visualize
```

## Example

Given a Python file `example.py`:

```python
class MagicalCreature:
    def __init__(self, name, power):
        self.name = name
        self.power = power
    
    def speak(self):
        return f"I am {self.name}, master of {self.power}!"

creature = MagicalCreature("Zephyr", "fire")
print(creature.speak())
```

The translator will generate equivalent JavaScript code in `output.js`.

## Project Structure

```
theoryproject/
├── CMakeLists.txt              # Build configuration
├── main.cpp                    # Main entry point
├── token.h/cpp                 # Token definitions
├── lexer.h/cpp                 # Lexical analyzer
├── parser.h/cpp                # Syntax parser
├── ast.h/cpp                   # Basic AST node definitions
├── ast_extended.h/cpp          # Extended AST nodes
├── semantic_analyzer.h/cpp     # Semantic analysis
├── js_code_generator.h/cpp     # JavaScript code generator
├── automata_visualizer.h/cpp   # SFML-based visualization
├── PythonToJSTranslator.h/cpp  # Main translator class
└── build/                      # Build directory
    └── example.py              # Example Python input
```

## Development

### Compiler Flags

The project uses strict compiler warnings:
- GCC/Clang: `-Wall -Wextra -pedantic`
- MSVC: `/W4`

### Build Modes

- **Debug**: Includes debug symbols (`-g`)
- **Release**: Optimized build (`-O3`)

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## License

This project is provided as-is for educational purposes.

## Acknowledgments

Built as a compiler theory educational project demonstrating the phases of compilation from lexical analysis to code generation.
