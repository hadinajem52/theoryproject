#pragma once
#include <string>
#include <vector>
#include <map>
#include "ast.h"

// Forward declarations for all AST node types
class Program;
class Block;
class FunctionDeclaration;
class ClassDeclaration;
class IfStatement;
class WhileStatement;
class ForStatement;
class ReturnStatement;
class BreakStatement;
class ContinueStatement;
class ImportStatement;
class AssignmentStatement;
class ExpressionStatement;
class Expression;
class Identifier;
class Literal;
class BinaryExpression;
class UnaryExpression;
class CallExpression;
class MemberExpression;
class SubscriptExpression;
class ListExpression;
class DictExpression;

class CodeGenerator {
public:
    virtual ~CodeGenerator() = default;
    
    // Main generation method
    virtual std::string generate(ASTNode* ast) = 0;
    
protected:
    // Visitor pattern methods for different node types
    virtual void visitNode(ASTNode* node) = 0;
    virtual void visitProgram(Program* node) = 0;
    virtual void visitBlock(Block* node) = 0;
    virtual void visitFunctionDeclaration(FunctionDeclaration* node) = 0;
    virtual void visitClassDeclaration(ClassDeclaration* node) = 0;
    virtual void visitIfStatement(IfStatement* node) = 0;
    virtual void visitWhileStatement(WhileStatement* node) = 0;
    virtual void visitForStatement(ForStatement* node) = 0;
    virtual void visitReturnStatement(ReturnStatement* node) = 0;
    virtual void visitBreakStatement(BreakStatement* node) = 0;
    virtual void visitContinueStatement(ContinueStatement* node) = 0;
    virtual void visitImportStatement(ImportStatement* node) = 0;
    virtual void visitAssignmentStatement(AssignmentStatement* node) = 0;
    virtual void visitExpressionStatement(ExpressionStatement* node) = 0;
    
    // Expression visitor methods
    virtual std::string generateExpression(Expression* node) = 0;
    virtual std::string generateIdentifier(Identifier* node) = 0;
    virtual std::string generateLiteral(Literal* node) = 0;
    virtual std::string generateBinaryExpression(BinaryExpression* node) = 0;
    virtual std::string generateUnaryExpression(UnaryExpression* node) = 0;
    virtual std::string generateCallExpression(CallExpression* node) = 0;
    virtual std::string generateMemberExpression(MemberExpression* node) = 0;
    virtual std::string generateSubscriptExpression(SubscriptExpression* node) = 0;
    virtual std::string generateListExpression(ListExpression* node) = 0;
    virtual std::string generateDictExpression(DictExpression* node) = 0;
    
    // Common code generation utilities
    virtual void emitLine(const std::string& line) = 0;
    virtual void emitNewLine() = 0;
    virtual void indent() = 0;
    virtual void dedent() = 0;
    
    // Output stream handling
    virtual void writeToFile(const std::string& outputFile) = 0;
};
