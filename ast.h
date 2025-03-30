#pragma once
#include <string>
#include <vector>
#include <memory>

// Base class for all AST nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual std::string toString() const = 0;
};

// Type alias for compatibility with code generator
using AST = ASTNode;

// Program is the root node of the AST
class Program : public ASTNode {
public:
    Program(std::vector<std::unique_ptr<ASTNode>> statements)
        : statements(std::move(statements)) {}
        
    std::string toString() const override;
    
    std::vector<std::unique_ptr<ASTNode>> statements;
};

// Expression is the base class for all expressions
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

// Statement is the base class for all statements
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// Type alias for compatibility with code generator
using StatementAST = Statement;

// Literal represents constant values
class Literal : public Expression {
public:
    enum LiteralType {
        INTEGER, FLOAT, STRING, BOOLEAN, NONE
    };
    
    Literal(LiteralType type, const std::string& value)
        : type(type), value(value) {}
        
    std::string toString() const override;
    
    LiteralType type;
    std::string value;
};

// Identifier represents variable names
class Identifier : public Expression {
public:
    Identifier(const std::string& name) : name(name) {}
    
    std::string toString() const override;
    
    std::string name;
};

// BinaryExpression represents operations with two operands
class BinaryExpression : public Expression {
public:
    enum Operator {
        ADD, SUBTRACT, MULTIPLY, DIVIDE, MODULO, POWER,
        EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
        AND, OR, BITAND, BITOR, BITXOR, LSHIFT, RSHIFT
    };
    
    BinaryExpression(Operator op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}
        
    std::string toString() const override;
    
    Operator op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
};

// UnaryExpression represents operations with one operand
class UnaryExpression : public Expression {
public:
    enum Operator {
        NEGATE, NOT, BITWISE_NOT
    };
    
    UnaryExpression(Operator op, std::unique_ptr<Expression> operand)
        : op(op), operand(std::move(operand)) {}
        
    std::string toString() const override;
    
    Operator op;
    std::unique_ptr<Expression> operand;
};

// ExpressionStatement represents a statement consisting of a single expression
class ExpressionStatement : public Statement {
public:
    ExpressionStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> expression;
};

// Additional AST node types will be defined here (e.g., FunctionDeclaration, IfStatement, etc.)
