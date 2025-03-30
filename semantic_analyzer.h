#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stack>
#include "ast.h"
#include "ast_extended.h"

// Forward declarations
class ASTNode;
class Expression;
class Statement;

// Represents a symbol in the symbol table
struct Symbol {
    enum Type {
        VARIABLE,
        FUNCTION,
        CLASS,
        MODULE,
        PARAMETER
    };
    
    Type type;
    std::string name;
    bool initialized = false;
    bool used = false;
    int declarationLine = 0;
    std::string scope;
    
    // For function symbols
    std::vector<std::string> parameterNames;
};

// Represents a scope in the symbol table
class Scope {
public:
    Scope(const std::string& name, Scope* parent = nullptr) 
        : name(name), parent(parent) {}
    
    std::string getName() const { return name; }
    Scope* getParent() const { return parent; }
    
    // Symbol management
    bool addSymbol(const Symbol& symbol);
    Symbol* findSymbol(const std::string& name);
    Symbol* findSymbolInCurrentScope(const std::string& name);
    
    // Get all symbols in this scope
    std::vector<Symbol> getSymbols() const;
    
private:
    std::string name;
    Scope* parent;
    std::unordered_map<std::string, Symbol> symbols;
};

// Main semantic analyzer class
class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    
    // Main method to analyze an AST
    void analyze(ASTNode* node);
    
    // Get error messages
    std::vector<std::string> getErrors() const;
    
    // Visualization interface
    struct SemanticState {
        std::string currentScope;
        std::string currentPhase;
        std::vector<std::string> symbolStackView;
    };
    
    SemanticState getCurrentState() const;
    
private:
    // Error collection
    void error(const std::string& message, int line = 0);
    
    // Scope management
    void enterScope(const std::string& name);
    void exitScope();
    
    // Symbol table operations
    bool declareSymbol(Symbol::Type type, const std::string& name, int line = 0);
    bool useSymbol(const std::string& name, int line = 0);
    Symbol* findSymbol(const std::string& name);
    
    // Node visitors - each handles a specific node type
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
    
    // Expression visitors
    void visitExpression(Expression* node);
    void visitIdentifier(Identifier* node);
    void visitLiteral(Literal* node);
    void visitBinaryExpression(BinaryExpression* node);
    void visitUnaryExpression(UnaryExpression* node);
    void visitCallExpression(CallExpression* node);
    void visitMemberExpression(MemberExpression* node);
    void visitSubscriptExpression(SubscriptExpression* node);
    void visitListExpression(ListExpression* node);
    void visitDictExpression(DictExpression* node);
    
    // Helper methods
    void visitNode(ASTNode* node);
    bool isInsideLoop() const;
    bool isInsideFunction() const;
    
    // Current state
    std::stack<Scope> scopeStack;
    std::vector<std::string> errors;
    bool inLoop = false;
    bool inFunction = false;
    std::string currentPhase;
};
