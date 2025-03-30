#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "ast_extended.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "js_code_generator.h"
#include "automata_visualizer.h"

// Main application class
class PythonToJSTranslator {
private:
    std::string inputFile;
    std::string outputFile;
    bool visualize;

public:
    PythonToJSTranslator(const std::string& input, const std::string& output, bool vis = false)
        : inputFile(input), outputFile(output), visualize(vis) {}

    bool run() {
        // Read Python code from file
        std::string pythonCode = readFile(inputFile);
        if (pythonCode.empty()) {
            return false;
        }

        // Phase 1: Lexical Analysis
        Lexer lexer(pythonCode);
        std::vector<Token> tokens = lexer.tokenize();
        
        // Phase 2: Syntax Analysis
        Parser parser(tokens);
        std::unique_ptr<ASTNode> ast = parser.parse();
        
        // Phase 3: Semantic Analysis
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast.get());
        
        // Phase 4: Code Generation
        JSCodeGenerator codeGen;
        std::string jsCode = codeGen.generate(ast.get());
        
        // Write the JavaScript code to output file
        if (!writeFile(outputFile, jsCode)) {
            return false;
        }
        
        // Phase 5: Visualization (if enabled)
        if (visualize) {
            AutomataVisualizer visualizer;
            visualizer.initialize();
            visualizer.visualizeLexer(lexer);
            visualizer.visualizeParser(parser);
            visualizer.run();
        }
        
        return true;
    }

private:
    std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return "";
        }
        return std::string((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
    }
    
    bool writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not write to file " << filename << std::endl;
            return false;
        }
        file << content;
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <input.py> <output.js> [--visualize]" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    bool visualize = (argc > 3 && std::string(argv[3]) == "--visualize");
    
    PythonToJSTranslator translator(inputFile, outputFile, visualize);
    
    if (translator.run()) {
        std::cout << "Translation completed successfully!" << std::endl;
        return 0;
    } else {
        std::cerr << "Translation failed." << std::endl;
        return 1;
    }
}
