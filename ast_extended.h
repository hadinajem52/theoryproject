#pragma once
#include "ast.h"
#include <string>
#include <vector>
#include <memory>

// Variable assignment
class AssignmentStatement : public Statement {
public:
    AssignmentStatement(std::unique_ptr<Expression> target, std::unique_ptr<Expression> value)
        : target(std::move(target)), value(std::move(value)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> target;
    std::unique_ptr<Expression> value;
};

// Block of statements with a particular indentation level
class Block : public Statement {
public:
    Block(std::vector<std::unique_ptr<Statement>> statements)
        : statements(std::move(statements)) {}
        
    std::string toString() const override;
    
    std::vector<std::unique_ptr<Statement>> statements;
};

// Function declaration
class FunctionDeclaration : public Statement {
public:
    struct Parameter {
        std::string name;
        std::unique_ptr<Expression> defaultValue; // Optional
    };
    
    FunctionDeclaration(const std::string& name, 
                        std::vector<Parameter> parameters,
                        std::unique_ptr<Block> body)
        : name(name), parameters(std::move(parameters)), body(std::move(body)) {}
        
    std::string toString() const override;
    
    std::string name;
    std::vector<Parameter> parameters;
    std::unique_ptr<Block> body;
};

// Function/method call
class CallExpression : public Expression {
public:
    CallExpression(std::unique_ptr<Expression> callee, 
                  std::vector<std::unique_ptr<Expression>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> callee;
    std::vector<std::unique_ptr<Expression>> arguments;
};

// If statement
class IfStatement : public Statement {
public:
    struct Branch {
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Block> body;
    };
    
    IfStatement(Branch ifBranch, 
               std::vector<Branch> elifBranches,
               std::unique_ptr<Block> elseBranch)
        : ifBranch(std::move(ifBranch)), 
          elifBranches(std::move(elifBranches)),
          elseBranch(std::move(elseBranch)) {}
          
    std::string toString() const override;
    
    Branch ifBranch;
    std::vector<Branch> elifBranches;
    std::unique_ptr<Block> elseBranch; // May be null
};

// While loop
class WhileStatement : public Statement {
public:
    WhileStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Block> body)
        : condition(std::move(condition)), body(std::move(body)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;
};

// For loop
class ForStatement : public Statement {
public:
    ForStatement(std::unique_ptr<Expression> variable, 
                std::unique_ptr<Expression> iterable,
                std::unique_ptr<Block> body)
        : variable(std::move(variable)), 
          iterable(std::move(iterable)),
          body(std::move(body)) {}
          
    std::string toString() const override;
    
    std::unique_ptr<Expression> variable;
    std::unique_ptr<Expression> iterable;
    std::unique_ptr<Block> body;
};

// Return statement
class ReturnStatement : public Statement {
public:
    ReturnStatement(std::unique_ptr<Expression> value = nullptr)
        : value(std::move(value)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> value; // May be null
};

// Break statement
class BreakStatement : public Statement {
public:
    std::string toString() const override;
};

// Continue statement
class ContinueStatement : public Statement {
public:
    std::string toString() const override;
};

// Class declaration
class ClassDeclaration : public Statement {
public:
    ClassDeclaration(const std::string& name, 
                    std::vector<std::string> baseClasses,
                    std::unique_ptr<Block> body)
        : name(name), baseClasses(std::move(baseClasses)), body(std::move(body)) {}
        
    std::string toString() const override;
    
    std::string name;
    std::vector<std::string> baseClasses;
    std::unique_ptr<Block> body;
};

// Property access (obj.prop)
class MemberExpression : public Expression {
public:
    MemberExpression(std::unique_ptr<Expression> object, const std::string& property)
        : object(std::move(object)), property(property) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> object;
    std::string property;
};

// Subscript access (array[index])
class SubscriptExpression : public Expression {
public:
    SubscriptExpression(std::unique_ptr<Expression> object, std::unique_ptr<Expression> index)
        : object(std::move(object)), index(std::move(index)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> object;
    std::unique_ptr<Expression> index;
};

// List literal [1, 2, 3]
class ListExpression : public Expression {
public:
    ListExpression(std::vector<std::unique_ptr<Expression>> elements)
        : elements(std::move(elements)) {}
        
    std::string toString() const override;
    
    std::vector<std::unique_ptr<Expression>> elements;
};

class DictExpression : public Expression {
public:
    struct KeyValuePair {
        std::unique_ptr<Expression> key;
        std::unique_ptr<Expression> value;
    };
    
    DictExpression(std::vector<KeyValuePair> entries)
        : entries(std::move(entries)) {}
        
    std::string toString() const override;
    
    std::vector<KeyValuePair> entries;
};

// Import statement
class ImportStatement : public Statement {
public:
    enum ImportType {
        IMPORT_MODULE,     // import module
        IMPORT_FROM,       // from module import item
        IMPORT_ALIAS       // import module as alias
    };
    
    ImportStatement(ImportType type, const std::string& module, 
                   const std::vector<std::string>& items = {},
                   const std::string& alias = "")
        : type(type), module(module), items(items), alias(alias) {}
        
    std::string toString() const override;
    
    ImportType type;
    std::string module;
    std::vector<std::string> items;
    std::string alias;
};

// F-String Literal representation
class FStringLiteral : public Expression {
public:
    FStringLiteral(const std::string& value) : value(value) {}
    
    std::string toString() const override {
        return "f\"" + value + "\"";
    }
    
    std::string getValue() const { return value; }
    
private:
    std::string value;
};
