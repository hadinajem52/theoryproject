#include "code_generator.h"
#include <stdexcept>
#include <fstream>

void CodeGenerator::generate(const AST* ast, const std::string& outputFile) {
    if (!ast) {
        throw std::runtime_error("Cannot generate code from null AST");
    }
    
    std::ofstream outFile(outputFile);
    
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputFile);
    }
    
    try {
        // Continue with code generation
        // ...existing code...
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Code generation error: ") + e.what());
<<<<<<< HEAD
    } catch (...) {
        throw std::runtime_error("Unknown error occurred during code generation");
=======
>>>>>>> e6b947508adc9caf112ff5e077ff371472f5b8e1
    }
}

void CodeGenerator::generateStatement(const StatementAST* ast) {
    if (!ast) {
        throw std::runtime_error("Cannot generate code from null StatementAST");
    }
    
    try {
        // Generate statement code
        // ...existing code...
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Statement generation error: ") + e.what());
    }
}