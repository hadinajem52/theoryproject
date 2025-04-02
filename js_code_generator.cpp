#include "js_code_generator.h"
#include <iostream>
#include <algorithm>
#include <cctype>

JSCodeGenerator::JSCodeGenerator() {
    // Initialize the scope stack with global scope
    scopeStack.push_back({});
    
    // Initialize the set of variables that have been marked as mutable
    mutableVariables = std::unordered_set<std::string>();
}

std::string JSCodeGenerator::generate(ASTNode* ast) {
    if (!ast) {
        return "// Error: Null AST provided";
    }
    
    // Clear the output
    output.str("");
    output.clear();
    indentLevel = 0;
    currentFunctionStack.clear();
    importedModules.clear();
    blockStack.clear();
    
    // Start code generation
    visitNode(ast);
    
    // Validate indentation at the end to ensure it's balanced
    validateIndentation();
    
    return output.str();
}

void JSCodeGenerator::setSettings(const Settings& newSettings) {
    settings = newSettings;
}

JSCodeGenerator::Settings JSCodeGenerator::getSettings() const {
    return settings;
}

void JSCodeGenerator::indent() {
    indentLevel++;
    // Add a guard to prevent excessive indentation which might indicate a nesting issue
    if (indentLevel > 20) {  // 20 is arbitrary, but reasonable for most code
        std::cerr << "Warning: Excessive indentation level " << indentLevel << std::endl;
    }
}

void JSCodeGenerator::dedent() {
    if (indentLevel > 0) {
        indentLevel--;
    } else {
        // Log if we try to dedent below zero, which indicates a nesting issue
        std::cerr << "Error: Attempted to dedent below zero" << std::endl;
    }
}

// Add a method to ensure indentation is balanced
void JSCodeGenerator::validateIndentation() {
    if (indentLevel != 0) {
        std::cerr << "Warning: Unbalanced indentation. Level: " << indentLevel << std::endl;
        // Reset to avoid cascading formatting errors
        indentLevel = 0;
    }
}

std::string JSCodeGenerator::getIndent() const {
    std::string indent;
    for (int i = 0; i < indentLevel * settings.indentSize; i++) {
        indent += " ";
    }
    return indent;
}

void JSCodeGenerator::emit(const std::string& code) {
    output << code;
}

void JSCodeGenerator::emitLine(const std::string& code) {
    output << getIndent() << code << "\n";
}

void JSCodeGenerator::emitNewLine() {
    output << "\n";
}

void JSCodeGenerator::enterBlock(BlockType type) {
    blockStack.push_back(type);
    // Add a new scope when entering a block
    scopeStack.push_back({});
    indent();
}

void JSCodeGenerator::exitBlock() {
    if (!blockStack.empty()) {
        blockStack.pop_back();
    }
    // Remove the scope when exiting a block
    if (scopeStack.size() > 1) {
        scopeStack.pop_back();
    }
    dedent();
}

void JSCodeGenerator::visitNode(ASTNode* node) {
    if (!node) return;
    
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
    } else if (auto tryExceptStmtNode = dynamic_cast<TryExceptStatement*>(node)) {
        visitTryExceptStatement(tryExceptStmtNode);
    } else {
        emitLine("// Unknown node type");
    }
}

void JSCodeGenerator::visitProgram(Program* node) {
    // Reset all state
    output.str("");
    output.clear();
    indentLevel = 0;
    blockStack.clear();
    currentFunctionStack.clear();
    importedModules.clear();
    
    emitLine("// JavaScript code generated from Python");
    emitNewLine();
    
    // Process each statement at the top level
    for (auto& stmt : node->statements) {
        // Process each statement independently at the top level
        visitNode(stmt.get());
        
        // Add newline after top-level declarations for better readability
        if (dynamic_cast<FunctionDeclaration*>(stmt.get()) ||
            dynamic_cast<ClassDeclaration*>(stmt.get()) ||
            dynamic_cast<ImportStatement*>(stmt.get())) {
            emitNewLine();
        }
    }
    
    // Final validation
    validateIndentation();
}

void JSCodeGenerator::visitBlock(Block* node) {
    // Important: Save current indent level before processing block
    int savedIndentLevel = indentLevel;
    
    for (auto& stmt : node->statements) {
        visitNode(stmt.get());
    }
    
    // Restore indent level after processing all statements in block
    indentLevel = savedIndentLevel;
}

void JSCodeGenerator::visitFunctionDeclaration(FunctionDeclaration* node) {
    currentFunctionStack.push_back(node->name);
    
    // Generate function declaration
    std::string functionHeader;
    
    if (!currentFunctionStack.empty() && currentFunctionStack.size() > 1) {
        // Class method
        if (node->name == "__init__") {
            // Convert Python's __init__ to JavaScript's constructor
            functionHeader = "constructor(" + getFunctionParameterList(node->parameters, true) + ") {";
        } else if (isSpecialMethod(node->name)) {
            // Handle other special methods
            functionHeader = node->name + "(" + getFunctionParameterList(node->parameters, true) + ") {";
        } else {
            // Regular class method
            functionHeader = node->name + "(" + getFunctionParameterList(node->parameters, true) + ") {";
        }
    } else {
        // Regular function - keep all parameters
        functionHeader = "function " + node->name + "(" + getFunctionParameterList(node->parameters, false) + ") {";
    }
    
    emitLine(functionHeader);
    
    // Enter function block with proper tracking
    enterBlock(FUNCTION);
    visitNode(node->body.get());
    exitBlock();
    
    emitLine("}");
    emitNewLine();
    
    currentFunctionStack.pop_back();
}

void JSCodeGenerator::visitClassDeclaration(ClassDeclaration* node) {
    std::string classHeader = "class " + node->name;
    
    if (!node->baseClasses.empty()) {
        classHeader += " extends " + node->baseClasses[0];
    }
    
    classHeader += " {";
    emitLine(classHeader);
    
    // Enter class block with proper tracking
    enterBlock(CLASS);
    
    // Collect all methods first to identify constructor
    bool hasConstructor = false;
    std::vector<FunctionDeclaration*> methods;
    
    for (auto& stmt : node->body->statements) {
        if (auto funcDecl = dynamic_cast<FunctionDeclaration*>(stmt.get())) {
            if (funcDecl->name == "__init__") {
                hasConstructor = true;
            }
            methods.push_back(funcDecl);
        }
    }
    
    // Generate constructor first if it exists
    for (auto method : methods) {
        if (method->name == "__init__") {
            // Push class name and method name for context
            currentFunctionStack.push_back(node->name);
            currentFunctionStack.push_back("constructor");
            
            emitLine("constructor(" + getFunctionParameterList(method->parameters, true) + ") {");
            
            // Generate constructor body
            enterBlock(FUNCTION);
            visitNode(method->body.get());
            exitBlock();
            
            emitLine("}");
            emitNewLine();
            
            // Reset context
            currentFunctionStack.pop_back(); // method name
            currentFunctionStack.pop_back(); // class name
        }
    }
    
    // Generate other methods (importantly, outside the constructor)
    for (auto method : methods) {
        if (method->name != "__init__") {
            // Push class name and method name for context
            currentFunctionStack.push_back(node->name);
            currentFunctionStack.push_back(method->name);
            
            std::string methodName = method->name;
            if (isSpecialMethod(methodName)) {
                // Map special Python methods to JS equivalents
                static const std::unordered_map<std::string, std::string> methodMap = {
                    {"__str__", "toString"},
                    {"__repr__", "toString"},
                    {"__len__", "length"},
                    {"__get__", "get"},
                    {"__set__", "set"}
                };
                
                auto it = methodMap.find(methodName);
                if (it != methodMap.end()) {
                    methodName = it->second;
                }
            }
            
            // Generate method signature
            emitLine(methodName + "(" + getFunctionParameterList(method->parameters, true) + ") {");
            
            // Generate method body
            enterBlock(FUNCTION);
            visitNode(method->body.get());
            exitBlock();
            
            emitLine("}");
            emitNewLine();
            
            // Reset context
            currentFunctionStack.pop_back(); // method name
            currentFunctionStack.pop_back(); // class name
        }
    }
    
    // Process non-method statements in class body
    for (auto& stmt : node->body->statements) {
        if (!dynamic_cast<FunctionDeclaration*>(stmt.get())) {
            visitNode(stmt.get());
        }
    }
    
    exitBlock();
    emitLine("}");
}

void JSCodeGenerator::visitIfStatement(IfStatement* node) {
    // Generate if condition
    emitLine("if (" + generateExpression(node->ifBranch.condition.get()) + ") {");
    
    // Enter and process if body
    enterBlock(IF);
    visitNode(node->ifBranch.body.get());
    exitBlock();
    
    // Generate elif branches
    for (auto& elifBranch : node->elifBranches) {
        emitLine("} else if (" + generateExpression(elifBranch.condition.get()) + ") {");
        
        enterBlock(IF);
        visitNode(elifBranch.body.get());
        exitBlock();
    }
    
    // Generate else branch
    if (node->elseBranch) {
        emitLine("} else {");
        
        enterBlock(IF);
        visitNode(node->elseBranch.get());
        exitBlock();
    }
    
    emitLine("}");
}

void JSCodeGenerator::visitWhileStatement(WhileStatement* node) {
    emitLine("while (" + generateExpression(node->condition.get()) + ") {");
    
    enterBlock(WHILE);
    visitNode(node->body.get());
    exitBlock();
    
    emitLine("}");
}

void JSCodeGenerator::visitForStatement(ForStatement* node) {
    std::string loopVar = generateExpression(node->variable.get());
    std::string iterable = generateExpression(node->iterable.get());
    
    // Convert Python for-in to JavaScript for-of
    emitLine("for (let " + loopVar + " of " + iterable + ") {");
    
    enterBlock(FOR);
    visitNode(node->body.get());
    exitBlock();
    
    emitLine("}");
}

void JSCodeGenerator::visitReturnStatement(ReturnStatement* node) {
    if (node->value) {
        emitLine("return " + generateExpression(node->value.get()) + ";");
    } else {
        emitLine("return;");
    }
}

void JSCodeGenerator::visitBreakStatement(BreakStatement* node) {
    emitLine("break;");
}

void JSCodeGenerator::visitContinueStatement(ContinueStatement* node) {
    emitLine("continue;");
}

void JSCodeGenerator::visitImportStatement(ImportStatement* node) {
    switch (node->type) {
        case ImportStatement::IMPORT_MODULE:
            // Convert Python import to JavaScript import
            emitLine("// Python: import " + node->module);
            emitLine("import * as " + node->module + " from '" + node->module + "';");
            break;
            
        case ImportStatement::IMPORT_FROM:
            // Convert Python from-import to JavaScript named import
            emitLine("// Python: from " + node->module + " import " + 
                    (node->items.empty() ? "" : node->items[0]));
            
            if (node->items.size() == 1) {
                emitLine("import { " + node->items[0] + " } from '" + node->module + "';");
            } else {
                std::string items;
                for (size_t i = 0; i < node->items.size(); i++) {
                    if (i > 0) items += ", ";
                    items += node->items[i];
                }
                emitLine("import { " + items + " } from '" + node->module + "';");
            }
            break;
            
        case ImportStatement::IMPORT_ALIAS:
            // Convert Python aliased import to JavaScript import with alias
            emitLine("// Python: import " + node->module + " as " + node->alias);
            emitLine("import * as " + node->alias + " from '" + node->module + "';");
            break;
    }
    
    // Track imported modules for potential polyfills or helper functions
    for (const auto& item : node->items) {
        importedModules[node->module].push_back(item);
    }
    
    emitNewLine();
}

void JSCodeGenerator::visitAssignmentStatement(AssignmentStatement* node) {
    // Generate target
    std::string target = generateExpression(node->target.get());
    std::string value = generateExpression(node->value.get());
    
    // Check if this is a declaration or assignment
    if (auto idTarget = dynamic_cast<Identifier*>(node->target.get())) {
        std::string varName = idTarget->name;
        
        // Check if the variable is already declared in the current or parent scopes
        bool isDeclared = false;
        for (const auto& scope : scopeStack) {
            if (scope.find(varName) != scope.end()) {
                isDeclared = true;
                break;
            }
        }
        
        if (isDeclared) {
            // Variable is already declared, just assign to it
            emitLine(target + " = " + value + ";");
            
            // Mark this variable as mutable since it's being reassigned
            mutableVariables.insert(varName);
        } else {
            // New variable, add it to the current scope
            scopeStack.back().insert(varName);
            
            // Determine whether to use const or let
            // Use let if:
            // 1. The variable name suggests it will be mutable (counter, i, index, etc.)
            // 2. We're inside a loop block
            // 3. settings.useConstForVariables is false
            bool shouldBeMutable = false;
            
            // Check if we're in a loop block
            for (auto blockType : blockStack) {
                if (blockType == FOR || blockType == WHILE) {
                    shouldBeMutable = true;
                    break;
                }
            }
            
            // Check if the variable name suggests it will be mutated
            static const std::vector<std::string> mutableNames = {
                "counter", "count", "i", "j", "k", "idx", "index", "temp", "tmp"
            };
            
            for (const auto& name : mutableNames) {
                if (varName.find(name) != std::string::npos) {
                    shouldBeMutable = true;
                    break;
                }
            }
            
            // Use appropriate declaration keyword
            std::string declarationKeyword;
            if (shouldBeMutable || !settings.useConstForVariables) {
                declarationKeyword = "let";
                mutableVariables.insert(varName); // Remember this variable is mutable
            } else {
                declarationKeyword = "const";
            }
            
            emitLine(declarationKeyword + " " + target + " = " + value + ";");
        }
    } else {
        // For other targets (member expressions, subscript expressions)
        emitLine(target + " = " + value + ";");
    }
}

void JSCodeGenerator::visitExpressionStatement(ExpressionStatement* node) {
    emitLine(generateExpression(node->expression.get()) + ";");
}

void JSCodeGenerator::visitTryExceptStatement(TryExceptStatement* node) {
    // Start try block
    emitLine("try {");
    
    // Generate try body with proper block tracking
    enterBlock(TRY);
    visitNode(node->tryBlock.get());
    exitBlock();
    
    // Generate catch blocks
    for (const auto& catchBlock : node->catchBlocks) {
        std::string catchVar = catchBlock.variable.empty() ? "e" : catchBlock.variable;
        
        // Simplified exception handling - don't add type checking unless necessary
        if (catchBlock.exceptionType.empty() || catchBlock.exceptionType == "Exception") {
            // Generic catch block (catches all exceptions)
            emitLine("} catch (" + catchVar + ") {");
        } else {
            // Only add type checking for specific exception types
            std::string jsExceptionType = translatePythonExceptionType(catchBlock.exceptionType);
            emitLine("} catch (" + catchVar + ") {");
            
            // Only include type checking if the exception is something other than the generic Error
            if (jsExceptionType != "Error") {
                enterBlock(TRY);
                emitLine("if (!(" + catchVar + " instanceof " + jsExceptionType + ")) {");
                emitLine("  throw " + catchVar + "; // Re-throw if not the right type");
                emitLine("}");
                exitBlock();
            }
        }
        
        // Generate catch block body
        enterBlock(TRY);
        visitNode(catchBlock.body.get());
        exitBlock();
    }
    
    // Generate finally block if it exists
    if (node->finallyBlock) {
        emitLine("} finally {");
        
        enterBlock(TRY);
        visitNode(node->finallyBlock.get());
        exitBlock();
    }
    
    emitLine("}");
}

std::string JSCodeGenerator::generateExpression(Expression* node) {
    if (!node) return "undefined";
    
    if (auto idNode = dynamic_cast<Identifier*>(node)) {
        return generateIdentifier(idNode);
    } else if (auto literalNode = dynamic_cast<Literal*>(node)) {
        return generateLiteral(literalNode);
    } else if (auto binaryExprNode = dynamic_cast<BinaryExpression*>(node)) {
        return generateBinaryExpression(binaryExprNode);
    } else if (auto unaryExprNode = dynamic_cast<UnaryExpression*>(node)) {
        return generateUnaryExpression(unaryExprNode);
    } else if (auto callExprNode = dynamic_cast<CallExpression*>(node)) {
        return generateCallExpression(callExprNode);
    } else if (auto memberExprNode = dynamic_cast<MemberExpression*>(node)) {
        return generateMemberExpression(memberExprNode);
    } else if (auto subscriptExprNode = dynamic_cast<SubscriptExpression*>(node)) {
        return generateSubscriptExpression(subscriptExprNode);
    } else if (auto listExprNode = dynamic_cast<ListExpression*>(node)) {
        return generateListExpression(listExprNode);
    } else if (auto listCompNode = dynamic_cast<ListComprehension*>(node)) {
        return generateListComprehension(listCompNode);
    } else if (auto dictExprNode = dynamic_cast<DictExpression*>(node)) {
        return generateDictExpression(dictExprNode);
    } else if (auto fstringNode = dynamic_cast<FStringLiteral*>(node)) {
        return generateFStringLiteral(fstringNode);
    }
    
    return "/* Unknown expression */";
}

std::string JSCodeGenerator::generateIdentifier(Identifier* node) {
    // Translate Python built-ins to JavaScript equivalents
    std::string name = node->name;
    
    // Always replace self with this
    if (name == "self") {
        return "this";
    }
    
    // Special handling for Python built-ins
    return translatePythonBuiltIn(name);
}

std::string JSCodeGenerator::generateLiteral(Literal* node) {
    switch (node->type) {
        case Literal::INTEGER:
        case Literal::FLOAT:
            return node->value;
            
        case Literal::STRING:
            // Use double quotes for JS strings
            return "\"" + node->value + "\"";
            
        case Literal::BOOLEAN:
            return node->value;
            
        case Literal::NONE:
            return "null";
            
        default:
            return "undefined";
    }
}

std::string JSCodeGenerator::generateBinaryExpression(BinaryExpression* node) {
    std::string left = generateExpression(node->left.get());
    std::string right = generateExpression(node->right.get());
    std::string op = getJavaScriptOperator(node->op);
    
    return "(" + left + " " + op + " " + right + ")";
}

std::string JSCodeGenerator::generateUnaryExpression(UnaryExpression* node) {
    std::string operand = generateExpression(node->operand.get());
    std::string op = getJavaScriptOperator(node->op);
    
    return op + "(" + operand + ")";
}

std::string JSCodeGenerator::generateCallExpression(CallExpression* node) {
    std::string callee = generateExpression(node->callee.get());
    
    // Check if this appears to be a class constructor call
    bool isConstructorCall = false;
    if (auto identCallee = dynamic_cast<Identifier*>(node->callee.get())) {
        // Simple heuristic: constructor calls usually have PascalCase names
        isConstructorCall = isLikelyConstructor(identCallee->name);
    }
    
    // Handle special Python built-ins
    if (callee == "print") {
        callee = "console.log";
        isConstructorCall = false;
    } else if (callee == "range") {
        // Convert Python's range to JavaScript Array.from with mapping
        if (node->arguments.size() == 1) {
            // range(stop)
            std::string stop = generateExpression(node->arguments[0].get());
            return "Array.from({length: " + stop + "}, (_, i) => i)";
        } else if (node->arguments.size() >= 2) {
            // range(start, stop[, step])
            std::string start = generateExpression(node->arguments[0].get());
            std::string stop = generateExpression(node->arguments[1].get());
            std::string step = (node->arguments.size() > 2) ? 
                              generateExpression(node->arguments[2].get()) : "1";
            
            return "Array.from({length: Math.ceil((" + stop + " - " + start + ") / " + step + ")}, " +
                   "(_, i) => " + start + " + i * " + step + ")";
        }
        isConstructorCall = false;
    }
    
    // Generate arguments
    std::string args;
    for (size_t i = 0; i < node->arguments.size(); i++) {
        if (i > 0) args += ", ";
        args += generateExpression(node->arguments[i].get());
    }
    
    // Add 'new' for constructor calls
    if (isConstructorCall) {
        return "new " + callee + "(" + args + ")";
    } else {
        return callee + "(" + args + ")";
    }
}

std::string JSCodeGenerator::generateMemberExpression(MemberExpression* node) {
    std::string object = generateExpression(node->object.get());
    std::string property = node->property;
    
    // Ensure self is translated to this
    if (object == "self") {
        object = "this";
    }
    
    // Handle special Python-to-JS method translations
    if (property == "__len__") {
        return object + ".length";
    } else if (property == "__str__") {
        return object + ".toString";
    }
    
    // Default case
    return object + "." + property;
}

std::string JSCodeGenerator::generateSubscriptExpression(SubscriptExpression* node) {
    std::string object = generateExpression(node->object.get());
    std::string index = generateExpression(node->index.get());
    
    return object + "[" + index + "]";
}

std::string JSCodeGenerator::generateListExpression(ListExpression* node) {
    // Standard list literal
    std::string elements;
    for (size_t i = 0; i < node->elements.size(); i++) {
        if (i > 0) elements += ", ";
        elements += generateExpression(node->elements[i].get());
    }
    
    return "[" + elements + "]";
}

std::string JSCodeGenerator::generateListComprehension(ListComprehension* node) {
    std::string iterable = generateExpression(node->iterable.get());
    std::string variable = generateExpression(node->variable.get());
    std::string expression = generateExpression(node->expression.get());
    
    // Implementation of Python list comprehension as JavaScript Array.map
    std::string result;
    
    if (node->condition) {
        // If there's a condition, use filter before map
        std::string condition = generateExpression(node->condition.get());
        result = iterable + ".filter(" + variable + " => " + condition + ").map(" + 
                variable + " => " + expression + ")";
    } else {
        // Simple mapping without filtering
        result = iterable + ".map(" + variable + " => " + expression + ")";
    }
    
    return result;
}

std::string JSCodeGenerator::generateDictExpression(DictExpression* node) {
    std::string entries;
    for (size_t i = 0; i < node->entries.size(); i++) {
        if (i > 0) entries += ", ";
        
        std::string key = generateExpression(node->entries[i].key.get());
        std::string value = generateExpression(node->entries[i].value.get());
        
        // Handle string keys properly
        if (auto keyLiteral = dynamic_cast<Literal*>(node->entries[i].key.get())) {
            if (keyLiteral->type == Literal::STRING) {
                entries += key + ": " + value;
                continue;
            }
        }
        
        // For non-string keys, we need to use computed property syntax
        entries += "[" + key + "]: " + value;
    }
    
    return "{" + entries + "}";
}

std::string JSCodeGenerator::generateFStringLiteral(FStringLiteral* node) {
    if (!node) return "``";
    
    std::string result = "`";  // JavaScript template literals use backticks
    
    // Process each part of the f-string
    for (const auto& part : node->getParts()) {
        if (part.isExpression) {
            // Always translate 'self' to 'this' in all contexts
            std::string exprStr = generateExpression(part.expression.get());
            
            // Replace any remaining instances of 'self' with 'this'
            size_t pos = 0;
            while ((pos = exprStr.find("self.", pos)) != std::string::npos) {
                exprStr.replace(pos, 5, "this.");
                pos += 5;
            }
            
            if (exprStr == "self") {
                exprStr = "this";
            }
            
            result += "${" + exprStr + "}";
        } else {
            // For text parts, escape backticks and add the text directly
            std::string escapedText = part.text;
            // Replace backticks with escaped backticks
            size_t pos = 0;
            while ((pos = escapedText.find('`', pos)) != std::string::npos) {
                escapedText.replace(pos, 1, "\\`");
                pos += 2;
            }
            
            // Replace ${...} with \${...} to prevent JavaScript interpretation
            pos = 0;
            while ((pos = escapedText.find("${", pos)) != std::string::npos) {
                escapedText.replace(pos, 2, "\\${");
                pos += 3;
            }
            
            result += escapedText;
        }
    }
    
    result += "`";
    return result;
}

std::string JSCodeGenerator::getFunctionParameterList(
    const std::vector<FunctionDeclaration::Parameter>& params,
    bool skipSelf) {
    
    std::string paramList;
    bool isFirst = true;
    
    for (size_t i = 0; i < params.size(); i++) {
        // Skip 'self' or 'cls' parameter in methods if skipSelf is true
        if (skipSelf && i == 0 && (params[i].name == "self" || params[i].name == "cls")) {
            continue;
        }
        
        if (!isFirst) {
            paramList += ", ";
        }
        
        paramList += params[i].name;
        
        if (params[i].defaultValue) {
            // Handle default parameters
            paramList += " = " + generateExpression(params[i].defaultValue.get());
        }
        
        isFirst = false;
    }
    
    return paramList;
}

std::string JSCodeGenerator::translatePythonBuiltIn(const std::string& name) {
    // Map Python built-ins to JavaScript equivalents
    static const std::unordered_map<std::string, std::string> builtins = {
        {"True", "true"},
        {"False", "false"},
        {"None", "null"},
        {"len", "/* Use .length property instead */"},
        {"str", "String"},
        {"int", "parseInt"},
        {"float", "parseFloat"},
        {"dict", "Object"},
        {"list", "Array"},
        {"print", "console.log"},
        {"self", "this"}
    };
    
    auto it = builtins.find(name);
    if (it != builtins.end()) {
        return it->second;
    }
    
    return name;
}

bool JSCodeGenerator::isSpecialMethod(const std::string& name) {
    // Check if this is a special method that needs special handling in JavaScript
    static const std::unordered_map<std::string, std::string> specialMethods = {
        {"__init__", "constructor"},
        {"__str__", "toString"},
        {"__repr__", "toString"},
        {"__len__", "length"},
        {"__get__", "get"},
        {"__set__", "set"}
    };
    
    auto it = specialMethods.find(name);
    if (it != specialMethods.end()) {
        return true;
    }
    
    return false;
}

bool JSCodeGenerator::isLikelyConstructor(const std::string& name) const {
    return !name.empty() && std::isupper(name[0]);
}

std::string JSCodeGenerator::translatePythonExceptionType(const std::string& pythonType) {
    // Map Python exception types to JavaScript equivalents
    static const std::unordered_map<std::string, std::string> exceptionMap = {
        {"Exception", "Error"},
        {"ValueError", "Error"},
        {"TypeError", "TypeError"},
        {"IndexError", "RangeError"},
        {"KeyError", "Error"},
        {"ZeroDivisionError", "Error"},
        {"RuntimeError", "Error"},
        {"IOError", "Error"},
        {"OSError", "Error"},
        {"FileNotFoundError", "Error"},
        {"ImportError", "Error"},
        {"SyntaxError", "SyntaxError"},
        {"NameError", "ReferenceError"},
        {"AttributeError", "TypeError"}
    };
    
    auto it = exceptionMap.find(pythonType);
    return it != exceptionMap.end() ? it->second : "Error";
}

std::string JSCodeGenerator::getJavaScriptOperator(BinaryExpression::Operator op) {
    switch (op) {
        case BinaryExpression::ADD: return "+";
        case BinaryExpression::SUBTRACT: return "-";
        case BinaryExpression::MULTIPLY: return "*";
        case BinaryExpression::DIVIDE: return "/";
        case BinaryExpression::MODULO: return "%";
        case BinaryExpression::POWER: return "**";
        case BinaryExpression::EQUAL: return "===";
        case BinaryExpression::NOT_EQUAL: return "!==";
        case BinaryExpression::LESS: return "<";
        case BinaryExpression::GREATER: return ">";
        case BinaryExpression::LESS_EQUAL: return "<=";
        case BinaryExpression::GREATER_EQUAL: return ">=";
        case BinaryExpression::AND: return "&&";
        case BinaryExpression::OR: return "||";
        case BinaryExpression::BITAND: return "&";
        case BinaryExpression::BITOR: return "|";
        case BinaryExpression::BITXOR: return "^";
        case BinaryExpression::LSHIFT: return "<<";
        case BinaryExpression::RSHIFT: return ">>";
        default: return "?";
    }
}

std::string JSCodeGenerator::getJavaScriptOperator(UnaryExpression::Operator op) {
    switch (op) {
        case UnaryExpression::NEGATE: return "-";
        case UnaryExpression::NOT: return "!";
        case UnaryExpression::BITWISE_NOT: return "~";
        default: return "?";
    }
}
