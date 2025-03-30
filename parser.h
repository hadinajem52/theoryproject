#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "token.h"
#include "ast.h"
#include "ast_extended.h"
#include <stdexcept>  // Ensure this is here and not commented out

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    
    // Main parsing method
    std::unique_ptr<ASTNode> parse();
    
    // Visualization support
    struct State {
        enum StateType {
            PROGRAM, STATEMENT, EXPRESSION, FUNCTION_DECLARATION,
            CLASS_DECLARATION, IF_STATEMENT, WHILE_STATEMENT, FOR_STATEMENT,
            IMPORT_STATEMENT, ASSIGNMENT, CALL_EXPRESSION, ERROR
        };
        
        StateType type;
        std::string name;
    };
    
    struct Transition {
        State from;
        State to;
        std::string condition;
    };
    
    // Get parsing states and transitions for visualization
    std::vector<State> getStates() const;
    std::vector<Transition> getTransitions() const;
    State getCurrentState() const;
    
private:
    const std::vector<Token>& tokens;
    size_t current;
    State currentState;
    
    // Helper methods
    Token peek() const;
    Token advance();
    bool check(Token::Type type) const;
    bool match(Token::Type type);
    bool match(const std::vector<Token::Type>& types);
    Token consume(Token::Type type, const std::string& message);
    void synchronize();
    
    // Parsing methods
    std::unique_ptr<Program> parseProgram();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseDeclaration();
    std::unique_ptr<FunctionDeclaration> parseFunctionDeclaration();
    std::unique_ptr<ClassDeclaration> parseClassDeclaration();
    std::unique_ptr<ImportStatement> parseImportStatement();
    std::vector<FunctionDeclaration::Parameter> parseFunctionParameters();
    std::unique_ptr<Statement> parseVarDeclaration();
    std::unique_ptr<Statement> parseIfStatement();
    std::unique_ptr<Statement> parseWhileStatement();
    std::unique_ptr<Statement> parseForStatement();
    std::unique_ptr<Statement> parseReturnStatement();
    std::unique_ptr<Block> parseBlock();
    std::unique_ptr<Statement> parseExpressionStatement();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseAssignment();
    std::unique_ptr<Expression> parseLogicalOr();
    std::unique_ptr<Expression> parseLogicalAnd();
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseTerm();
    std::unique_ptr<Expression> parseFactor();
    std::unique_ptr<Expression> parseUnary();
    std::unique_ptr<Expression> parseCall();
    std::unique_ptr<Expression> parsePrimary();
    std::unique_ptr<ListExpression> parseListLiteral();
    std::unique_ptr<DictExpression> parseDictLiteral();
    
    // Error reporting
    class ParseError : public std::runtime_error {
    public:
        ParseError(const std::string& message) : std::runtime_error(message) {}
    };
    
    ParseError error(const Token& token, const std::string& message);
    
    // Automaton states and transitions for visualization
    std::vector<State> states;
    std::vector<Transition> transitions;
    
    // Initialize the automaton states and transitions
    void initAutomaton();
};
