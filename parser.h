#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "token.h"
#include "ast.h"
#include "ast_extended.h"
#include <stdexcept>

class Parser {
public:
    // State enum definition
    enum StateType {
        PROGRAM, STATEMENT, EXPRESSION, FUNCTION_DECLARATION,
        CLASS_DECLARATION, IF_STATEMENT, WHILE_STATEMENT, FOR_STATEMENT,
        IMPORT_STATEMENT, ASSIGNMENT, CALL_EXPRESSION, ERROR
    };
    
    struct State {
        StateType type;
        std::string name;
        
        bool operator==(const State& other) const {
            return type == other.type && name == other.name;
        }
    };
    
    struct Transition {
        State from;
        State to;
        std::string condition;
    };
    
    Parser(const std::vector<Token>& tokens);
    
    // Main parsing method
    std::unique_ptr<ASTNode> parse();
    
    // Get parsing states and transitions for visualization
    std::vector<State> getStates() const;
    std::vector<Transition> getTransitions() const;
    State getCurrentState() const;
    
    // Error checking methods
    bool hasParseError() const;
    std::string getErrorMessage() const;
    
private:
    const std::vector<Token>& tokens;
    size_t current;
    State currentState;
    
    // Error tracking
    bool hasError;
    std::string errorMessage;
    
    // Automaton states and transitions for visualization
    std::vector<State> states;
    std::vector<Transition> transitions;
    
    // Initialize the automaton states and transitions
    void initAutomaton();
    
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
    std::unique_ptr<Statement> parseTryStatement();
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
    std::unique_ptr<Expression> parseListLiteral();
    std::unique_ptr<DictExpression> parseDictLiteral();
    
    // Error reporting
    class ParseError : public std::runtime_error {
    public:
        ParseError(const std::string& message) : std::runtime_error(message) {}
    };
    
    ParseError error(const Token& token, const std::string& message);
};

// Define a hash function for the State struct if needed for unordered_map
namespace std {
    template<>
    struct hash<Parser::State> {
        size_t operator()(const Parser::State& state) const {
            return hash<int>()(static_cast<int>(state.type)) ^ hash<string>()(state.name);
        }
    };
}
