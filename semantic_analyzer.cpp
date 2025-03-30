#include "semantic_analyzer.h"
#include <sstream>
#include <iostream>

// Scope implementation
bool Scope::addSymbol(const Symbol& symbol) {
    // Check if symbol already exists in current scope
    if (symbols.find(symbol.name) != symbols.end()) {
        return false;
    }
    
    symbols[symbol.name] = symbol;
    return true;
}

Symbol* Scope::findSymbol(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    
    // If not found in current scope, check parent scope
    if (parent) {
        return parent->findSymbol(name);
    }
    
    return nullptr;
}

Symbol* Scope::findSymbolInCurrentScope(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<Symbol> Scope::getSymbols() const {
    std::vector<Symbol> result;
    for (const auto& pair : symbols) {
        result.push_back(pair.second);
    }
    return result;
}

// SemanticAnalyzer implementation
SemanticAnalyzer::SemanticAnalyzer() {
    // Initialize with global scope
    scopeStack.push(Scope("global"));
}

void SemanticAnalyzer::analyze(ASTNode* node) {
    if (!node) {
        error("Null AST node encountered");
        return;
    }
    
    // Start analysis
    try {
        visitNode(node);
        
        // Check for unused variables as a final step
        std::vector<Symbol> symbols = scopeStack.top().getSymbols();
        for (const auto& symbol : symbols) {
            if (!symbol.used && symbol.type == Symbol::VARIABLE) {
                std::stringstream ss;
                ss << "Warning: Variable '" << symbol.name << "' declared but never used";
                error(ss.str(), symbol.declarationLine);
            }
        }
    } catch (const std::exception& e) {
        error(std::string("Analysis error: ") + e.what());
    }
}

std::vector<std::string> SemanticAnalyzer::getErrors() const {
    return errors;
}

void SemanticAnalyzer::error(const std::string& message, int line) {
    std::stringstream ss;
    if (line > 0) {
        ss << "Line " << line << ": ";
    }
    ss << message;
    errors.push_back(ss.str());
}

void SemanticAnalyzer::enterScope(const std::string& name) {
    std::string fullScopeName = scopeStack.top().getName() + "." + name;
    scopeStack.push(Scope(fullScopeName, &scopeStack.top()));
}

void SemanticAnalyzer::exitScope() {
    if (scopeStack.size() > 1) { // Keep at least the global scope
        scopeStack.pop();
    }
}

bool SemanticAnalyzer::declareSymbol(Symbol::Type type, const std::string& name, int line) {
    Symbol symbol;
    symbol.type = type;
    symbol.name = name;
    symbol.declarationLine = line;
    symbol.scope = scopeStack.top().getName();
    
    if (type == Symbol::FUNCTION || type == Symbol::CLASS) {
        symbol.initialized = true; // Functions and classes are always initialized
    }
    
    if (!scopeStack.top().addSymbol(symbol)) {
        std::stringstream ss;
        ss << "Error: Redefinition of '" << name << "'";
        error(ss.str(), line);
        return false;
    }
    
    return true;
}

bool SemanticAnalyzer::useSymbol(const std::string& name, int line) {
    Symbol* symbol = findSymbol(name);
    if (!symbol) {
        std::stringstream ss;
        ss << "Error: Use of undeclared identifier '" << name << "'";
        error(ss.str(), line);
        return false;
    }
    
    if (!symbol->initialized && symbol->type == Symbol::VARIABLE) {
        std::stringstream ss;
        ss << "Warning: Variable '" << name << "' used before initialization";
        error(ss.str(), line);
    }
    
    symbol->used = true;
    return true;
}

Symbol* SemanticAnalyzer::findSymbol(const std::string& name) {
    return scopeStack.top().findSymbol(name);
}

void SemanticAnalyzer::visitNode(ASTNode* node) {
    // Set current phase for visualization
    currentPhase = "Analyzing " + std::string(typeid(*node).name());
    
    // Use dynamic_cast to determine the type of node
    if (auto programNode = dynamic_cast<Program*>(node)) {
        visitProgram(programNode);
    } else if (auto blockNode = dynamic_cast<Block*>(node)) {
        visitBlock(blockNode);
    } else if (auto funcDeclNode = dynamic_cast<FunctionDeclaration*>(node)) {
        visitFunctionDeclaration(funcDeclNode);
    } else if (auto classDeclNode = dynamic_cast<ClassDeclaration*>(node)) {
        visitClassDeclaration(classDeclNode);
    } else if (auto ifStmtNode = dynamic_cast<IfStatement*>(node)) {
        visitIfStatement(ifStmtNode);
    } else if (auto whileStmtNode = dynamic_cast<WhileStatement*>(node)) {
        visitWhileStatement(whileStmtNode);
    } else if (auto forStmtNode = dynamic_cast<ForStatement*>(node)) {
        visitForStatement(forStmtNode);
    } else if (auto returnStmtNode = dynamic_cast<ReturnStatement*>(node)) {
        visitReturnStatement(returnStmtNode);
    } else if (auto breakStmtNode = dynamic_cast<BreakStatement*>(node)) {
        visitBreakStatement(breakStmtNode);
    } else if (auto continueStmtNode = dynamic_cast<ContinueStatement*>(node)) {
        visitContinueStatement(continueStmtNode);
    } else if (auto importStmtNode = dynamic_cast<ImportStatement*>(node)) {
        visitImportStatement(importStmtNode);
    } else if (auto assignmentStmtNode = dynamic_cast<AssignmentStatement*>(node)) {
        visitAssignmentStatement(assignmentStmtNode);
    } else if (auto exprStmtNode = dynamic_cast<ExpressionStatement*>(node)) {
        visitExpressionStatement(exprStmtNode);
    } else if (auto exprNode = dynamic_cast<Expression*>(node)) {
        visitExpression(exprNode);
    } else {
        error("Unknown node type encountered");
    }
}

void SemanticAnalyzer::visitProgram(Program* node) {
    for (auto& stmt : node->statements) {
        visitNode(stmt.get());
    }
}

void SemanticAnalyzer::visitBlock(Block* node) {
    for (auto& stmt : node->statements) {
        visitNode(stmt.get());
    }
}

void SemanticAnalyzer::visitFunctionDeclaration(FunctionDeclaration* node) {
    // Register the function in the current scope
    declareSymbol(Symbol::FUNCTION, node->name, 0);
    
    // Update symbol with parameter information
    Symbol* funcSymbol = scopeStack.top().findSymbolInCurrentScope(node->name);
    if (funcSymbol) {
        for (const auto& param : node->parameters) {
            funcSymbol->parameterNames.push_back(param.name);
        }
    }
    
    // Create a new scope for the function body
    enterScope(node->name);
    bool prevInFunction = inFunction;
    inFunction = true;
    
    // Register parameters as variables in the function scope
    for (const auto& param : node->parameters) {
        Symbol paramSymbol;
        paramSymbol.type = Symbol::PARAMETER;
        paramSymbol.name = param.name;
        paramSymbol.initialized = true; // Parameters are always initialized
        paramSymbol.used = false;
        paramSymbol.scope = scopeStack.top().getName();
        
        if (!scopeStack.top().addSymbol(paramSymbol)) {
            error("Duplicate parameter name: " + param.name);
        }
    }
    
    // Process the function body
    visitNode(node->body.get());
    
    // Restore previous function state and exit function scope
    inFunction = prevInFunction;
    exitScope();
}

void SemanticAnalyzer::visitClassDeclaration(ClassDeclaration* node) {
    // Register the class in the current scope
    declareSymbol(Symbol::CLASS, node->name, 0);
    
    // Create a new scope for the class body
    enterScope(node->name);
    
    // Implicitly declare 'self' as a parameter for methods in this class
    Symbol selfSymbol;
    selfSymbol.type = Symbol::VARIABLE;
    selfSymbol.name = "self";
    selfSymbol.initialized = true;
    selfSymbol.used = false; // We won't check if 'self' is used
    selfSymbol.scope = scopeStack.top().getName();
    scopeStack.top().addSymbol(selfSymbol);
    
    // Process the class body
    visitNode(node->body.get());
    
    // Exit class scope
    exitScope();
}

void SemanticAnalyzer::visitIfStatement(IfStatement* node) {
    // Check the if condition
    visitNode(node->ifBranch.condition.get());
    
    // Process the if body with a new scope
    enterScope("if");
    visitNode(node->ifBranch.body.get());
    exitScope();
    
    // Process elif branches
    for (size_t i = 0; i < node->elifBranches.size(); i++) {
        visitNode(node->elifBranches[i].condition.get());
        
        enterScope("elif_" + std::to_string(i));
        visitNode(node->elifBranches[i].body.get());
        exitScope();
    }
    
    // Process the else branch
    if (node->elseBranch) {
        enterScope("else");
        visitNode(node->elseBranch.get());
        exitScope();
    }
}

void SemanticAnalyzer::visitWhileStatement(WhileStatement* node) {
    // Check the condition
    visitNode(node->condition.get());
    
    // Process the body with a new scope
    enterScope("while");
    bool prevInLoop = inLoop;
    inLoop = true;
    
    visitNode(node->body.get());
    
    inLoop = prevInLoop;
    exitScope();
}

void SemanticAnalyzer::visitForStatement(ForStatement* node) {
    // Check the iterable
    visitNode(node->iterable.get());
    
    // Process the body with a new scope
    enterScope("for");
    bool prevInLoop = inLoop;
    inLoop = true;
    
    // Handle the loop variable - if it's an identifier, it's implicitly declared
    if (auto idVar = dynamic_cast<Identifier*>(node->variable.get())) {
        Symbol forVarSymbol;
        forVarSymbol.type = Symbol::VARIABLE;
        forVarSymbol.name = idVar->name;
        forVarSymbol.initialized = true; // Loop variables are initialized by the for loop
        forVarSymbol.used = false;
        forVarSymbol.scope = scopeStack.top().getName();
        
        scopeStack.top().addSymbol(forVarSymbol);
    } else {
        // Handle unpacking or more complex loop variables
        visitNode(node->variable.get());
    }
    
    visitNode(node->body.get());
    
    inLoop = prevInLoop;
    exitScope();
}

void SemanticAnalyzer::visitReturnStatement(ReturnStatement* node) {
    if (!isInsideFunction()) {
        error("Return statement outside of function");
    }
    
    if (node->value) {
        visitNode(node->value.get());
    }
}

void SemanticAnalyzer::visitBreakStatement(BreakStatement* node) {
    if (!isInsideLoop()) {
        error("Break statement outside of loop");
    }
}

void SemanticAnalyzer::visitContinueStatement(ContinueStatement* node) {
    if (!isInsideLoop()) {
        error("Continue statement outside of loop");
    }
}

void SemanticAnalyzer::visitImportStatement(ImportStatement* node) {
    switch (node->type) {
        case ImportStatement::IMPORT_MODULE:
            // Simply register the module name
            declareSymbol(Symbol::MODULE, node->module, 0);
            break;
            
        case ImportStatement::IMPORT_FROM:
            // Register each imported item from the module
            for (const auto& item : node->items) {
                declareSymbol(Symbol::VARIABLE, item, 0);
            }
            break;
            
        case ImportStatement::IMPORT_ALIAS:
            // Register the alias
            declareSymbol(Symbol::MODULE, node->alias, 0);
            break;
    }
}

void SemanticAnalyzer::visitAssignmentStatement(AssignmentStatement* node) {
    // Process right side first
    visitNode(node->value.get());
    
    // Handle different types of assignment targets
    if (auto idTarget = dynamic_cast<Identifier*>(node->target.get())) {
        // Simple variable assignment - if it doesn't exist, declare it
        Symbol* symbol = scopeStack.top().findSymbolInCurrentScope(idTarget->name);
        if (!symbol) {
            declareSymbol(Symbol::VARIABLE, idTarget->name, 0);
            symbol = scopeStack.top().findSymbolInCurrentScope(idTarget->name);
        }
        
        if (symbol) {
            symbol->initialized = true;
        }
    } else if (auto memberTarget = dynamic_cast<MemberExpression*>(node->target.get())) {
        // Handle object property assignment (obj.prop = value)
        visitNode(memberTarget->object.get());
    } else if (auto subscriptTarget = dynamic_cast<SubscriptExpression*>(node->target.get())) {
        // Handle array/dict element assignment (obj[key] = value)
        visitNode(subscriptTarget->object.get());
        visitNode(subscriptTarget->index.get());
    } else {
        // Other assignment targets (like unpacking assignments)
        visitNode(node->target.get());
    }
}

void SemanticAnalyzer::visitExpressionStatement(ExpressionStatement* node) {
    visitNode(node->expression.get());
}

void SemanticAnalyzer::visitExpression(Expression* node) {
    // Process expression nodes based on their specific type
    if (auto idNode = dynamic_cast<Identifier*>(node)) {
        visitIdentifier(idNode);
    } else if (auto literalNode = dynamic_cast<Literal*>(node)) {
        visitLiteral(literalNode);
    } else if (auto binaryExprNode = dynamic_cast<BinaryExpression*>(node)) {
        visitBinaryExpression(binaryExprNode);
    } else if (auto unaryExprNode = dynamic_cast<UnaryExpression*>(node)) {
        visitUnaryExpression(unaryExprNode);
    } else if (auto callExprNode = dynamic_cast<CallExpression*>(node)) {
        visitCallExpression(callExprNode);
    } else if (auto memberExprNode = dynamic_cast<MemberExpression*>(node)) {
        visitMemberExpression(memberExprNode);
    } else if (auto subscriptExprNode = dynamic_cast<SubscriptExpression*>(node)) {
        visitSubscriptExpression(subscriptExprNode);
    } else if (auto listExprNode = dynamic_cast<ListExpression*>(node)) {
        visitListExpression(listExprNode);
    } else if (auto dictExprNode = dynamic_cast<DictExpression*>(node)) {
        visitDictExpression(dictExprNode);
    } else {
        error("Unknown expression type");
    }
}

void SemanticAnalyzer::visitIdentifier(Identifier* node) {
    useSymbol(node->name, 0);
}

void SemanticAnalyzer::visitLiteral(Literal* node) {
    // Nothing to analyze for literals
}

void SemanticAnalyzer::visitBinaryExpression(BinaryExpression* node) {
    visitNode(node->left.get());
    visitNode(node->right.get());
}

void SemanticAnalyzer::visitUnaryExpression(UnaryExpression* node) {
    visitNode(node->operand.get());
}

void SemanticAnalyzer::visitCallExpression(CallExpression* node) {
    // First, visit the callee
    visitNode(node->callee.get());
    
    // Then, check arguments
    for (auto& arg : node->arguments) {
        visitNode(arg.get());
    }
    
    // If the callee is an identifier, we can do more specific checking
    if (auto idCallee = dynamic_cast<Identifier*>(node->callee.get())) {
        Symbol* funcSymbol = findSymbol(idCallee->name);
        if (funcSymbol && funcSymbol->type == Symbol::FUNCTION) {
            // Check argument count (if this is a direct function call)
            if (node->arguments.size() != funcSymbol->parameterNames.size()) {
                std::stringstream ss;
                ss << "Function '" << idCallee->name << "' called with " 
                   << node->arguments.size() << " arguments, but expects " 
                   << funcSymbol->parameterNames.size();
                error(ss.str());
            }
        }
    }
}

void SemanticAnalyzer::visitMemberExpression(MemberExpression* node) {
    visitNode(node->object.get());
    // No need to check property name as it's just a string
}

void SemanticAnalyzer::visitSubscriptExpression(SubscriptExpression* node) {
    visitNode(node->object.get());
    visitNode(node->index.get());
}

void SemanticAnalyzer::visitListExpression(ListExpression* node) {
    for (auto& element : node->elements) {
        visitNode(element.get());
    }
}

void SemanticAnalyzer::visitDictExpression(DictExpression* node) {
    for (auto& entry : node->entries) {
        visitNode(entry.key.get());
        visitNode(entry.value.get());
    }
}

bool SemanticAnalyzer::isInsideLoop() const {
    return inLoop;
}

bool SemanticAnalyzer::isInsideFunction() const {
    return inFunction;
}

SemanticAnalyzer::SemanticState SemanticAnalyzer::getCurrentState() const {
    SemanticState state;
    state.currentScope = scopeStack.top().getName();
    state.currentPhase = currentPhase;
    
    // Build a view of the symbol stack
    std::stack<Scope> tempStack = scopeStack;
    while (!tempStack.empty()) {
        std::string scopeName = tempStack.top().getName();
        std::vector<Symbol> symbols = tempStack.top().getSymbols();
        
        std::stringstream ss;
        ss << "Scope " << scopeName << ": ";
        bool first = true;
        for (const auto& symbol : symbols) {
            if (!first) ss << ", ";
            first = false;
            ss << symbol.name << "(" << (symbol.initialized ? "I" : "")
               << (symbol.used ? "U" : "") << ")";
        }
        
        state.symbolStackView.push_back(ss.str());
        tempStack.pop();
    }
    
    return state;
}
