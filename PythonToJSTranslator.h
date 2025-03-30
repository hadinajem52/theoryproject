#pragma once
#include <string>
#include "lexer.h"
#include "parser.h"
#include "code_generator.h"

// Forward declarations if needed
class SemanticAnalyzer;

class PythonToJSTranslator {
public:
    bool run(const std::string& inputFile, const std::string& outputFile);
    
private:
    std::string readFile(const std::string& filename);
};

// Simple semantic analyzer class that we assume exists
class SemanticAnalyzer {
public:
    bool analyze(const ASTNode* ast) {
        // Simplified implementation
        return ast != nullptr;
    }
};
