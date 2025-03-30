#include "code_generator.h"
#include <stdexcept>
#include <fstream>

void CodeGenerator::generate(const AST* ast, const std::string& outputFile) {
    std::ofstream outFile(outputFile);
    
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputFile);
    }
    
    if (!ast) {
        throw std::runtime_error("Cannot generate code from null AST");
    }
    
    // Continue with code generation
    // ...existing code...
}

void CodeGenerator::generateStatement(const StatementAST* ast) {
    if (!ast) {
        throw std::runtime_error("Cannot generate code from null StatementAST");
    }
    
    // ...existing code...
}