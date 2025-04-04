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
        bool isAccepting = false;  // Added property
        bool isInitial = false;    // Added property
        
        State(StateType t = START, const std::string& n = "") 
            : type(t), name(n) {}
    };
    
    struct Transition {
        State from;
        State to;
        std::string condition;
    };
    
    // Trace recording for visualization
    struct TraceStep {
        State from;
        State to;
        std::string input;
        
        TraceStep(const State& f, const State& t, const std::string& i)
            : from(f), to(t), input(i) {}
    };
    
    // Get automaton structure for visualization
    std::vector<State> getStates() const;
    std::vector<Transition> getTransitions() const;
    std::vector<TraceStep> getExecutionTrace() const;
    
    // Get current state for visualization
    State getCurrentState() const { return currentState; }
    
    // Reset trace for a new analysis
    void resetTrace();
    
private:
    std::string sourceCode;
    size_t pos;
    int line;
    int column;
    std::stack<int> indentStack;
    State currentState;
    
    // Execution trace for visualization
    std::vector<TraceStep> executionTrace;
    
    // Helper method to record a transition
    void recordTransition(const State& from, const State& to, const std::string& input);
    
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
