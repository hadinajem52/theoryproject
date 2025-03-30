#include "PythonToJSTranslator.h"
#include <iostream>
#include <fstream>
#include <string>

bool PythonToJSTranslator::run(const std::string& inputFile, const std::string& outputFile) {
    // Read input file
    std::string sourceCode = readFile(inputFile);
    if (sourceCode.empty()) {
        std::cerr << "Error: Could not read input file or file is empty: " << inputFile << std::endl;
        return false;
    }

    // Lexical analysis
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();
    
    // Print tokens for debugging
    std::cout << "Tokens:\n";
    for (const auto& token : tokens) {
        if (token.type != Token::WHITESPACE && token.type != Token::COMMENT) {
            std::cout << token.toString() << std::endl;
        }
    }

    // Syntax analysis
    Parser parser(tokens);
    std::unique_ptr<ASTNode> ast = parser.parse();
    
    // Check if parsing was successful
    if (!ast) {
        std::cerr << "Error: Parsing failed - could not generate AST" << std::endl;
        return false;
    }

    // Semantic analysis
    SemanticAnalyzer analyzer;
    if (!analyzer.analyze(ast.get())) {
        std::cerr << "Error: Semantic analysis failed" << std::endl;
        return false;
    }

    // Code generation
    CodeGenerator generator;
    try {
        generator.generate(ast.get(), outputFile);
        std::cout << "Successfully translated " << inputFile << " to " << outputFile << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error during code generation: " << e.what() << std::endl;
        return false;
    }
}

std::string PythonToJSTranslator::readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file: " << filename << std::endl;
        return "";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()));
    return content;
}
