#include "code_generator.h"
#include <stdexcept>
#include <fstream>

// This file can be removed since CodeGenerator is an abstract class
// and JSCodeGenerator is the actual implementation we're using.
// If you want to keep it for future reference, here's a corrected version:

/*
std::string CodeGenerator::generate(ASTNode* ast) {
    if (!ast) {
        throw std::runtime_error("Cannot generate code from null AST");
    }
    
    // This is an abstract method, so it shouldn't have implementation
    // The derived classes should implement this method
    throw std::runtime_error("CodeGenerator::generate() called on abstract base class");
}

void CodeGenerator::writeToFile(const std::string& outputFile, const std::string& code) {
    std::ofstream outFile(outputFile);
    
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open output file: " + outputFile);
    }
    
    outFile << code;
    
    if (!outFile) {
        throw std::runtime_error("Error occurred while writing to output file: " + outputFile);
    }
}
*/