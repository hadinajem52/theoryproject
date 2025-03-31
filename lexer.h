#pragma once
#include <string>
#include <vector>
#include <stack>
#include "token.h"

class Lexer {
public:
    Lexer(const std::string& code);
    
    std::vector<Token> tokenize();
    
    // Methods to support visualization
    struct State {
        enum StateType {
            START, IDENTIFIER, NUMBER, STRING, OPERATOR, COMMENT,
            WHITESPACE, NEWLINE, INDENTATION
        };
        
        StateType type;
        std::string name;
    };
    
    struct Transition {
        State from;
        State to;
        std::string condition;
    };
    
    // Get automaton structure for visualization
    std::vector<State> getStates() const;
    std::vector<Transition> getTransitions() const;
    
    // Get current state for visualization
    State getCurrentState() const { return currentState; }
    
private:
    std::string sourceCode;
    size_t pos;
    int line;
    int column;
    std::stack<int> indentStack;
    State currentState;
    
    // Helper methods for tokenization
    char peek(int offset = 0) const;
    char advance();
    bool match(char expected);
    void skipWhitespace();
    
    // Token recognition methods
    Token handleIdentifier();
    Token handleNumber();
    Token handleString();
    Token handleFString(); // New method to handle f-strings
    Token handleOperator();
    Token handleComment();
    std::vector<Token> handleIndentation(const std::string& line);
    
    // Automaton states and transitions for visualization
    std::vector<State> states;
    std::vector<Transition> transitions;
    
    // Initialize automaton states and transitions
    void initAutomaton();
};
