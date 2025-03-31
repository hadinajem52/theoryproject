#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <unordered_map>
#include "ast.h"
#include "ast_extended.h"

class JSCodeGenerator {
public:
    JSCodeGenerator();
    
    // Main method to generate JavaScript code from an AST
    std::string generate(ASTNode* ast);
    
    // Settings for code generation
    struct Settings {
        bool useConstForVariables = true;    // Use const/let instead of var
        bool useArrowFunctions = true;       // Use arrow functions where appropriate
        bool useTemplateStrings = true;      // Use template strings for string formatting
        bool addTypeComments = false;        // Add type hints as comments
        int indentSize = 2;                  // Number of spaces per indent level
    };
    
    void setSettings(const Settings& settings);
    Settings getSettings() const;
    
private:
    Settings settings;
    int indentLevel = 0;
    std::stringstream output;
    
    // Tracking state
    std::vector<std::string> currentFunctionStack;
    std::unordered_map<std::string, std::vector<std::string>> importedModules;
    
    // Utility methods
    void indent();
    void dedent();
    std::string getIndent() const;
    void emit(const std::string& code);
    void emitLine(const std::string& code);
    void emitNewLine();
    
    // Visitor methods for nodes
    void visitNode(ASTNode* node);
    void visitProgram(Program* node);
    void visitBlock(Block* node);
    void visitFunctionDeclaration(FunctionDeclaration* node);
    void visitClassDeclaration(ClassDeclaration* node);
    void visitIfStatement(IfStatement* node);
    void visitWhileStatement(WhileStatement* node);
    void visitForStatement(ForStatement* node);
    void visitReturnStatement(ReturnStatement* node);
    void visitBreakStatement(BreakStatement* node);
    void visitContinueStatement(ContinueStatement* node);
    void visitImportStatement(ImportStatement* node);
    void visitAssignmentStatement(AssignmentStatement* node);
    void visitExpressionStatement(ExpressionStatement* node);
    
    // Expression visitor methods
    std::string generateExpression(Expression* node);
    std::string generateIdentifier(Identifier* node);
    std::string generateLiteral(Literal* node);
    std::string generateBinaryExpression(BinaryExpression* node);
    std::string generateUnaryExpression(UnaryExpression* node);
    std::string generateCallExpression(CallExpression* node);
    std::string generateMemberExpression(MemberExpression* node);
    std::string generateSubscriptExpression(SubscriptExpression* node);
    std::string generateListExpression(ListExpression* node);
    std::string generateDictExpression(DictExpression* node);
    std::string generateFStringLiteral(FStringLiteral* node); // New method for f-strings
    
    // Helper methods
    std::string getFunctionParameterList(const std::vector<FunctionDeclaration::Parameter>& params);
    std::string translatePythonBuiltIn(const std::string& name);
    bool isSpecialMethod(const std::string& name);
    std::string getJavaScriptOperator(BinaryExpression::Operator op);
    std::string getJavaScriptOperator(UnaryExpression::Operator op);
};
