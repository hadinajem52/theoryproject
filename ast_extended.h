#pragma once
#include "ast.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

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
    
    std::unique_ptr<Expression> clone() const override {
        auto clonedCallee = callee->clone();
        std::vector<std::unique_ptr<Expression>> clonedArgs;
        for (const auto& arg : arguments) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_unique<CallExpression>(std::move(clonedCallee), std::move(clonedArgs));
    }
    
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
    
    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<MemberExpression>(object->clone(), property);
    }
    
    std::unique_ptr<Expression> object;
    std::string property;
};

// Subscript access (array[index])
class SubscriptExpression : public Expression {
public:
    SubscriptExpression(std::unique_ptr<Expression> object, std::unique_ptr<Expression> index)
        : object(std::move(object)), index(std::move(index)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<SubscriptExpression>(object->clone(), index->clone());
    }
    
    std::unique_ptr<Expression> object;
    std::unique_ptr<Expression> index;
};

// Slice access (array[start:end:step])
class SliceExpression : public Expression {
public:
    SliceExpression(std::unique_ptr<Expression> object, 
                   std::unique_ptr<Expression> start,
                   std::unique_ptr<Expression> end,
                   std::unique_ptr<Expression> step = nullptr)
        : object(std::move(object)), start(std::move(start)), 
          end(std::move(end)), step(std::move(step)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<SliceExpression>(
            object->clone(),
            start ? start->clone() : nullptr,
            end ? end->clone() : nullptr,
            step ? step->clone() : nullptr
        );
    }
    
    std::unique_ptr<Expression> object;
    std::unique_ptr<Expression> start; // May be nullptr (e.g., [:end])
    std::unique_ptr<Expression> end;   // May be nullptr (e.g., [start:])
    std::unique_ptr<Expression> step;  // May be nullptr (e.g., [start:end])
};

// List literal [1, 2, 3]
class ListExpression : public Expression {
public:
    ListExpression(std::vector<std::unique_ptr<Expression>> elements)
        : elements(std::move(elements)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> clone() const override {
        std::vector<std::unique_ptr<Expression>> clonedElements;
        for (const auto& element : elements) {
            clonedElements.push_back(element->clone());
        }
        return std::make_unique<ListExpression>(std::move(clonedElements));
    }
    
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
    
    std::unique_ptr<Expression> clone() const override {
        std::vector<KeyValuePair> clonedEntries;
        for (const auto& entry : entries) {
            KeyValuePair clonedEntry;
            clonedEntry.key = entry.key->clone();
            clonedEntry.value = entry.value->clone();
            clonedEntries.push_back(std::move(clonedEntry));
        }
        return std::make_unique<DictExpression>(std::move(clonedEntries));
    }
    
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
    struct Part {
        bool isExpression;
        std::string text;                      // Used when isExpression is false
        std::unique_ptr<Expression> expression; // Used when isExpression is true
        
        Part(const std::string& text) : isExpression(false), text(text) {}
        Part(std::unique_ptr<Expression> expr) : isExpression(true), expression(std::move(expr)) {}
    };
    
    FStringLiteral(const std::string& value);
    
    std::string toString() const override;
    
    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<FStringLiteral>(rawValue);
    }
    
    std::string getRawValue() const { return rawValue; }
    const std::vector<Part>& getParts() const { return parts; }
    
private:
    std::string rawValue;
    std::vector<Part> parts;
    
    void parseContent(const std::string& content);
    std::string extractExpressionText(const std::string& content, size_t& pos);
};

// List comprehension [expr for var in iterable if condition]
class ListComprehension : public Expression {
public:
    ListComprehension(
        std::unique_ptr<Expression> expression,
        std::unique_ptr<Expression> variable,
        std::unique_ptr<Expression> iterable,
        std::unique_ptr<Expression> condition = nullptr
    ) : expression(std::move(expression)),
        variable(std::move(variable)),
        iterable(std::move(iterable)),
        condition(std::move(condition)) {}
        
    std::string toString() const override {
        std::stringstream ss;
        ss << "ListComprehension([" << expression->toString() << " for ";
        ss << variable->toString() << " in " << iterable->toString();
        if (condition) {
            ss << " if " << condition->toString();
        }
        ss << "])";
        return ss.str();
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ListComprehension>(
            expression->clone(),
            variable->clone(),
            iterable->clone(),
            condition ? condition->clone() : nullptr
        );
    }

    std::unique_ptr<Expression> expression;
    std::unique_ptr<Expression> variable;
    std::unique_ptr<Expression> iterable;
    std::unique_ptr<Expression> condition;
};

// Try-Except Statement for exception handling
class TryExceptStatement : public Statement {
public:
    struct CatchBlock {
        std::string exceptionType;
        std::string variable;
        std::unique_ptr<Block> body;
    };
    
    TryExceptStatement(
        std::unique_ptr<Block> tryBlock,
        std::vector<CatchBlock> catchBlocks,
        std::unique_ptr<Block> finallyBlock = nullptr
    ) : tryBlock(std::move(tryBlock)),
        catchBlocks(std::move(catchBlocks)),
        finallyBlock(std::move(finallyBlock)) {}
        
    std::string toString() const override {
        std::stringstream ss;
        ss << "TryExceptStatement(try=" << tryBlock->toString();
        for (const auto& cb : catchBlocks) {
            ss << ", except " << cb.exceptionType;
            if (!cb.variable.empty()) {
                ss << " as " << cb.variable;
            }
            ss << "=" << cb.body->toString();
        }
        if (finallyBlock) {
            ss << ", finally=" << finallyBlock->toString();
        }
        ss << ")";
        return ss.str();
    }
    
    std::unique_ptr<Block> tryBlock;
    std::vector<CatchBlock> catchBlocks;
    std::unique_ptr<Block> finallyBlock;
};

// Lambda expression (lambda x: x * 2)
class LambdaExpression : public Expression {
public:
    struct Parameter {
        std::string name;
        std::unique_ptr<Expression> defaultValue; // Optional
    };
    
    LambdaExpression(std::vector<Parameter> parameters, std::unique_ptr<Expression> body)
        : parameters(std::move(parameters)), body(std::move(body)) {}
        
    std::string toString() const override;
    
    std::unique_ptr<Expression> clone() const override {
        std::vector<Parameter> clonedParams;
        for (const auto& param : parameters) {
            Parameter clonedParam;
            clonedParam.name = param.name;
            clonedParam.defaultValue = param.defaultValue ? param.defaultValue->clone() : nullptr;
            clonedParams.push_back(std::move(clonedParam));
        }
        return std::make_unique<LambdaExpression>(std::move(clonedParams), body->clone());
    }
    
    std::vector<Parameter> parameters;
    std::unique_ptr<Expression> body;
};
