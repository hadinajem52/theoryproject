#include "js_code_generator.h"
#include <iostream>
#include <algorithm>
#include <cctype>

JSCodeGenerator::JSCodeGenerator() {}

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
    
    // Start code generation
    visitNode(ast);
    
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
}

void JSCodeGenerator::dedent() {
    if (indentLevel > 0) {
        indentLevel--;
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
    } else {
        emitLine("// Unknown node type");
    }
}

void JSCodeGenerator::visitProgram(Program* node) {
    emitLine("// JavaScript code generated from Python");
    emitNewLine();
    
    for (auto& stmt : node->statements) {
        visitNode(stmt.get());
    }
}

void JSCodeGenerator::visitBlock(Block* node) {
    for (auto& stmt : node->statements) {
        visitNode(stmt.get());
    }
}

void JSCodeGenerator::visitFunctionDeclaration(FunctionDeclaration* node) {
    currentFunctionStack.push_back(node->name);
    
    // Generate function declaration
    std::string functionHeader;
    
    if (!currentFunctionStack.empty() && currentFunctionStack.size() > 1 && 
        isSpecialMethod(node->name)) {
        // Class method
        functionHeader = node->name + "(" + getFunctionParameterList(node->parameters) + ") {";
    } else {
        // Regular function
        functionHeader = "function " + node->name + "(" + getFunctionParameterList(node->parameters) + ") {";
    }
    
    emitLine(functionHeader);
    
    // Generate function body
    indent();
    visitNode(node->body.get());
    dedent();
    
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
    
    // Add constructor if needed
    bool hasConstructor = false;
    for (auto& stmt : node->body->statements) {
        if (auto funcDecl = dynamic_cast<FunctionDeclaration*>(stmt.get())) {
            if (funcDecl->name == "__init__") {
                hasConstructor = true;
                break;
            }
        }
    }
    
    if (!hasConstructor) {
        indent();
        emitLine("constructor() {");
        emitLine("}");
        dedent();
    }
    
    // Generate class body
    indent();
    visitNode(node->body.get());
    dedent();
    
    emitLine("}");
    emitNewLine();
}

void JSCodeGenerator::visitIfStatement(IfStatement* node) {
    // Generate if condition
    emitLine("if (" + generateExpression(node->ifBranch.condition.get()) + ") {");
    
    // Generate if body
    indent();
    visitNode(node->ifBranch.body.get());
    dedent();
    
    // Generate elif branches
    for (auto& elifBranch : node->elifBranches) {
        emitLine("} else if (" + generateExpression(elifBranch.condition.get()) + ") {");
        
        indent();
        visitNode(elifBranch.body.get());
        dedent();
    }
    
    // Generate else branch
    if (node->elseBranch) {
        emitLine("} else {");
        
        indent();
        visitNode(node->elseBranch.get());
        dedent();
    }
    
    emitLine("}");
}

void JSCodeGenerator::visitWhileStatement(WhileStatement* node) {
    emitLine("while (" + generateExpression(node->condition.get()) + ") {");
    
    indent();
    visitNode(node->body.get());
    dedent();
    
    emitLine("}");
}

void JSCodeGenerator::visitForStatement(ForStatement* node) {
    std::string loopVar = generateExpression(node->variable.get());
    std::string iterable = generateExpression(node->iterable.get());
    
    // Convert Python for-in to JavaScript for-of
    emitLine("for (let " + loopVar + " of " + iterable + ") {");
    
    indent();
    visitNode(node->body.get());
    dedent();
    
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
        // Assume variable declaration for simple identifiers
        // In JavaScript we'd use let or const, but in general case we'll use let
        emitLine("let " + target + " = " + value + ";");
    } else {
        // For other targets (member expressions, subscript expressions)
        emitLine(target + " = " + value + ";");
    }
}

void JSCodeGenerator::visitExpressionStatement(ExpressionStatement* node) {
    emitLine(generateExpression(node->expression.get()) + ";");
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
    
    // Handle special cases like print() -> console.log()
    if (callee == "print") {
        callee = "console.log";
    }
    
    std::string args;
    for (size_t i = 0; i < node->arguments.size(); i++) {
        if (i > 0) args += ", ";
        args += generateExpression(node->arguments[i].get());
    }
    
    return callee + "(" + args + ")";
}

std::string JSCodeGenerator::generateMemberExpression(MemberExpression* node) {
    std::string object = generateExpression(node->object.get());
    std::string property = node->property;
    
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
    std::string elements;
    for (size_t i = 0; i < node->elements.size(); i++) {
        if (i > 0) elements += ", ";
        elements += generateExpression(node->elements[i].get());
    }
    
    return "[" + elements + "]";
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
            // For expression parts, add ${expression}
            result += "${" + generateExpression(part.expression.get()) + "}";
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

std::string JSCodeGenerator::getFunctionParameterList(const std::vector<FunctionDeclaration::Parameter>& params) {
    std::string paramList;
    
    // In Python, the first parameter of a method is 'self', which is equivalent to 'this' in JavaScript
    // We don't want to include it in the parameter list
    bool isClassMethod = !currentFunctionStack.empty() && currentFunctionStack.size() > 1;
    
    for (size_t i = 0; i < params.size(); i++) {
        // Skip 'self' parameter in methods
        if (i == 0 && isClassMethod && (params[i].name == "self" || params[i].name == "cls")) {
            continue;
        }
        
        if (i > 0 || (i == 0 && isClassMethod && (params[0].name == "self" || params[0].name == "cls"))) {
            paramList += ", ";
        }
        
        paramList += params[i].name;
        
        if (params[i].defaultValue) {
            // Handle default parameters
            paramList += " = " + generateExpression(params[i].defaultValue.get());
        }
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
        {"range", "/* Use Array(n).fill().map((_, i) => i) */"},
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
