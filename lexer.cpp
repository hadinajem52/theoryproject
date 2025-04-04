#include "lexer.h"
#include <cctype>
#include <algorithm>
#include <iostream>

Lexer::Lexer(const std::string& code) 
    : sourceCode(code), pos(0), line(1), column(1), 
      currentState({State::START, "START"}) {
    indentStack.push(0); // Start with no indentation
    initAutomaton();
}

void Lexer::initAutomaton() {
    // Define states
    states = {
        {State::START, "START"},
        {State::IDENTIFIER, "IDENTIFIER"},
        {State::NUMBER, "NUMBER"},
        {State::STRING, "STRING"},
        {State::OPERATOR, "OPERATOR"},
        {State::COMMENT, "COMMENT"},
        {State::WHITESPACE, "WHITESPACE"},
        {State::NEWLINE, "NEWLINE"},
        {State::INDENTATION, "INDENTATION"}
    };
    
    // Define transitions
    transitions = {
        // From START state
        {{State::START, "START"}, {State::IDENTIFIER, "IDENTIFIER"}, "isalpha or _"},
        {{State::START, "START"}, {State::NUMBER, "NUMBER"}, "isdigit"},
        {{State::START, "START"}, {State::STRING, "STRING"}, "\" or '"},
        {{State::START, "START"}, {State::OPERATOR, "OPERATOR"}, "operator chars"},
        {{State::START, "START"}, {State::COMMENT, "COMMENT"}, "# encountered"},
        {{State::START, "START"}, {State::WHITESPACE, "WHITESPACE"}, "space or tab"},
        {{State::START, "START"}, {State::NEWLINE, "NEWLINE"}, "\\n or \\r"},
        
        // From IDENTIFIER state
        {{State::IDENTIFIER, "IDENTIFIER"}, {State::IDENTIFIER, "IDENTIFIER"}, "isalnum or _"},
        {{State::IDENTIFIER, "IDENTIFIER"}, {State::START, "START"}, "other"},
        
        // From NUMBER state
        {{State::NUMBER, "NUMBER"}, {State::NUMBER, "NUMBER"}, "isdigit or ."},
        {{State::NUMBER, "NUMBER"}, {State::START, "START"}, "other"},
        
        // From STRING state
        {{State::STRING, "STRING"}, {State::STRING, "STRING"}, "any except closing quote"},
        {{State::STRING, "STRING"}, {State::START, "START"}, "closing quote"},
        
        // From OPERATOR state
        {{State::OPERATOR, "OPERATOR"}, {State::OPERATOR, "OPERATOR"}, "another operator char"},
        {{State::OPERATOR, "OPERATOR"}, {State::START, "START"}, "other"},
        
        // From COMMENT state
        {{State::COMMENT, "COMMENT"}, {State::START, "START"}, "EOL or EOF reached"},
        
        // From WHITESPACE state
        {{State::WHITESPACE, "WHITESPACE"}, {State::WHITESPACE, "WHITESPACE"}, "space or tab"},
        {{State::WHITESPACE, "WHITESPACE"}, {State::START, "START"}, "other"},
        
        // From NEWLINE state
        {{State::NEWLINE, "NEWLINE"}, {State::INDENTATION, "INDENTATION"}, "after newline"},
        
        // From INDENTATION state
        {{State::INDENTATION, "INDENTATION"}, {State::START, "START"}, "after processing indentation"}
    };
}

void Lexer::recordTransition(const State& from, const State& to, const std::string& input) {
    executionTrace.emplace_back(from, to, input);
    currentState = to;
}

void Lexer::resetTrace() {
    executionTrace.clear();
    currentState = {State::START, "START"};
}

std::vector<Lexer::TraceStep> Lexer::getExecutionTrace() const {
    return executionTrace;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    pos = 0;
    line = 1;
    column = 1;
    indentStack = std::stack<int>();
    indentStack.push(0);
    currentState = {State::START, "START"};
    resetTrace();
    
    while (pos < sourceCode.length()) {
        char current = peek();
        State prevState = currentState;
        
        // Handle different character types
        if (current == '#') {
            // Update current state to COMMENT before handling
            State commentState = {State::COMMENT, "COMMENT"};
            recordTransition(prevState, commentState, "#");
            
            tokens.push_back(handleComment());
            
            // Return to start state
            State startState = {State::START, "START"};
            recordTransition(commentState, startState, "end of comment");
        } else if (isalpha(current) || current == '_') {
            // Check for f-string before handling as identifier
            if ((current == 'f' || current == 'F') && pos + 1 < sourceCode.length() && 
                (sourceCode[pos + 1] == '"' || sourceCode[pos + 1] == '\'')) {
                // Update current state to STRING before handling
                State stringState = {State::STRING, "STRING"};
                recordTransition(prevState, stringState, "f");
                
                Token token = handleFString();
                tokens.push_back(token);
                
                // Return to start state
                State startState = {State::START, "START"};
                recordTransition(stringState, startState, token.value);
            } else {
                // Update current state to IDENTIFIER before handling
                State identifierState = {State::IDENTIFIER, "IDENTIFIER"};
                recordTransition(prevState, identifierState, std::string(1, current));
                
                Token token = handleIdentifier();
                tokens.push_back(token);
                
                // Return to start state
                State startState = {State::START, "START"};
                recordTransition(identifierState, startState, token.value);
            }
        } else if (isdigit(current)) {
            // Update current state to NUMBER before handling
            State numberState = {State::NUMBER, "NUMBER"};
            recordTransition(prevState, numberState, std::string(1, current));
            
            Token token = handleNumber();
            tokens.push_back(token);
            
            // Return to start state
            State startState = {State::START, "START"};
            recordTransition(numberState, startState, token.value);
        } else if (current == '"' || current == '\'') {
            // Update current state to STRING before handling
            State stringState = {State::STRING, "STRING"};
            recordTransition(prevState, stringState, std::string(1, current));
            
            Token token = handleString();
            tokens.push_back(token);
            
            // Return to start state
            State startState = {State::START, "START"};
            recordTransition(stringState, startState, token.value);
        } else if (ispunct(current) && current != '#') {
            // Update current state to OPERATOR before handling
            State operatorState = {State::OPERATOR, "OPERATOR"};
            recordTransition(prevState, operatorState, std::string(1, current));
            
            Token token = handleOperator();
            tokens.push_back(token);
            
            // Return to start state
            State startState = {State::START, "START"};
            recordTransition(operatorState, startState, token.value);
        } else if (current == '\n') {
            // Update current state to NEWLINE before handling
            State newlineState = {State::NEWLINE, "NEWLINE"};
            recordTransition(prevState, newlineState, "\\n");
            
            tokens.push_back(Token(Token::NEWLINE, "\\n", line, column));
            advance();
            line++;
            column = 1;
            
            // Update current state to INDENTATION before handling
            State indentState = {State::INDENTATION, "INDENTATION"};
            recordTransition(newlineState, indentState, "after newline");
            
            // Handle indentation for the next line
            std::string nextLine;
            size_t tempPos = pos;
            while (tempPos < sourceCode.length() && sourceCode[tempPos] != '\n') {
                nextLine += sourceCode[tempPos++];
            }
            
            std::vector<Token> indentTokens = handleIndentation(nextLine);
            tokens.insert(tokens.end(), indentTokens.begin(), indentTokens.end());
            
            // Return to start state
            State startState = {State::START, "START"};
            recordTransition(indentState, startState, "after indentation");
        } else if (isspace(current)) {
            // Update current state to WHITESPACE before handling
            State whitespaceState = {State::WHITESPACE, "WHITESPACE"};
            recordTransition(prevState, whitespaceState, "whitespace");
            
            skipWhitespace();
            
            // Return to start state
            State startState = {State::START, "START"};
            recordTransition(whitespaceState, startState, "end of whitespace");
        } else {
            // Unrecognized character - stay in START state
            tokens.push_back(Token(Token::ERROR, std::string(1, current), line, column));
            recordTransition(prevState, prevState, "error: " + std::string(1, current));
            advance();
        }
    }
    
    // Handle any remaining dedents at end of file
    while (indentStack.size() > 1) { // Keep the first 0 indentation level
        indentStack.pop();
        tokens.push_back(Token(Token::DEDENT, "", line, column));
    }
    
    // Add EOF token
    tokens.push_back(Token(Token::END_OF_FILE, "", line, column));
    
    return tokens;
}

char Lexer::peek(int offset) const {
    if (pos + offset >= sourceCode.length()) {
        return '\0'; // End of input
    }
    return sourceCode[pos + offset];
}

char Lexer::advance() {
    if (pos >= sourceCode.length()) {
        return '\0'; // End of input
    }
    
    char c = sourceCode[pos++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (peek() != expected) {
        return false;
    }
    advance();
    return true;
}

void Lexer::skipWhitespace() {
    while (std::isspace(peek()) && peek() != '\n' && peek() != '\r') {
        advance();
    }
}

Token Lexer::handleIdentifier() {
    int startPos = pos;
    int startLine = line;
    int startColumn = column;
    
    // Consume the identifier or keyword
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    
    // Extract the identifier text
    std::string identifier = sourceCode.substr(startPos, pos - startPos);
    
    // Check if it's a keyword
    Token::Type type = Token::getKeywordType(identifier);
    
    return Token(type, identifier, startLine, startColumn);
}

Token Lexer::handleNumber() {
    int startPos = pos;
    int startLine = line;
    int startColumn = column;
    bool hasDecimal = false;
    
    // Consume digits
    while (std::isdigit(peek()) || peek() == '.') {
        if (peek() == '.') {
            if (hasDecimal) {
                break; // Second decimal point - not part of the number
            }
            hasDecimal = true;
        }
        advance();
    }
    
    // Extract the number text
    std::string number = sourceCode.substr(startPos, pos - startPos);
    
    // Determine if it's an integer or floating-point
    Token::Type type = hasDecimal ? Token::LITERAL_FLOAT : Token::LITERAL_INT;
    
    return Token(type, number, startLine, startColumn);
}

Token Lexer::handleString() {
    int startLine = line;
    int startColumn = column;
    char quoteType = advance(); // Either ' or "
    std::string str;
    
    // Check for triple quotes
    bool isTripleQuoted = false;
    if (peek() == quoteType && peek(1) == quoteType) {
        isTripleQuoted = true;
        advance(); // consume second quote
        advance(); // consume third quote
    }
    
    // Consume the string content
    while ((peek() != quoteType || 
           (isTripleQuoted && (peek(1) != quoteType || peek(2) != quoteType))) && 
           peek() != '\0') {
        
        if (peek() == '\\') {
            advance(); // Skip the backslash
            switch (peek()) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\'': str += '\''; break;
                case '\"': str += '\"'; break;
                case '\\': str += '\\'; break;
                default: str += peek(); break;
            }
        } else if (peek() == '\n') {
            // Allow actual newlines in triple-quoted strings
            if (isTripleQuoted) {
                str += '\n';
                line++;
                column = 1;
            } else {
                return Token(Token::ERROR, "Unterminated string", startLine, startColumn);
            }
        } else {
            str += peek();
        }
        advance();
    }
    
    // Check if the string was closed properly
    if (peek() != quoteType) {
        return Token(Token::ERROR, "Unterminated string", startLine, startColumn);
    }
    
    advance(); // Consume the closing quote
    
    // If triple-quoted, consume the other two closing quotes
    if (isTripleQuoted) {
        if (peek() != quoteType || peek(1) != quoteType) {
            return Token(Token::ERROR, "Unterminated triple-quoted string", startLine, startColumn);
        }
        advance(); // second quote
        advance(); // third quote
    }
    
    return Token(Token::LITERAL_STRING, str, startLine, startColumn);
}

Token Lexer::handleFString() {
    int startLine = line;
    int startColumn = column;
    
    // Consume the 'f' or 'F' prefix
    advance();
    
    char quoteType = advance(); // Either ' or "
    std::string str;
    
    // Check for triple quotes
    bool isTripleQuoted = false;
    if (peek() == quoteType && peek(1) == quoteType) {
        isTripleQuoted = true;
        advance(); // consume second quote
        advance(); // consume third quote
    }
    
    // Consume the string content
    while ((peek() != quoteType || 
           (isTripleQuoted && (peek(1) != quoteType || peek(2) != quoteType))) && 
           peek() != '\0') {
        
        if (peek() == '\\') {
            advance(); // Skip the backslash
            switch (peek()) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\'': str += '\''; break;
                case '\"': str += '\"'; break;
                case '\\': str += '\\'; break;
                default: str += peek(); break;
            }
        } else if (peek() == '\n') {
            // Allow actual newlines in triple-quoted strings
            if (isTripleQuoted) {
                str += '\n';
                line++;
                column = 1;
            } else {
                return Token(Token::ERROR, "Unterminated f-string", startLine, startColumn);
            }
        } else {
            str += peek();
        }
        advance();
    }
    
    // Check if the string was closed properly
    if (peek() != quoteType) {
        return Token(Token::ERROR, "Unterminated f-string", startLine, startColumn);
    }
    
    advance(); // Consume the closing quote
    
    // If triple-quoted, consume the other two closing quotes
    if (isTripleQuoted) {
        if (peek() != quoteType || peek(1) != quoteType) {
            return Token(Token::ERROR, "Unterminated triple-quoted f-string", startLine, startColumn);
        }
        advance(); // second quote
        advance(); // third quote
    }
    
    return Token(Token::LITERAL_FSTRING, str, startLine, startColumn);
}

Token Lexer::handleOperator() {
    int startLine = line;
    int startColumn = column;
    char c = advance();
    std::string op(1, c);
    
    // Two-character operators
    if ((c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '=' || 
         c == '<' || c == '>' || c == '&' || c == '|' || c == '^') && peek() == '=') {
        op += advance();
    } else if (c == '*' && peek() == '*') {
        op += advance(); // Power operator **
    } else if (c == '/' && peek() == '/') {
        op += advance(); // Integer division //
    } else if (c == '<' && peek() == '<') {
        op += advance(); // Left shift <<
    } else if (c == '>' && peek() == '>') {
        op += advance(); // Right shift >>
    } else if (c == '!' && peek() == '=') {
        op += advance(); // Not equals !=
    }
    
    // Map operator string to token type
    Token::Type type;
    
    if (op == "+") type = Token::OP_PLUS;
    else if (op == "-") type = Token::OP_MINUS;
    else if (op == "*") type = Token::OP_MULTIPLY;
    else if (op == "/") type = Token::OP_DIVIDE;
    else if (op == "%") type = Token::OP_MODULO;
    else if (op == "**") type = Token::OP_POWER;
    else if (op == "<<") type = Token::OP_LSHIFT;
    else if (op == ">>") type = Token::OP_RSHIFT;
    else if (op == "&") type = Token::OP_BITAND;
    else if (op == "|") type = Token::OP_BITOR;
    else if (op == "^") type = Token::OP_BITXOR;
    else if (op == "~") type = Token::OP_BITNOT;
    else if (op == "=") type = Token::OP_ASSIGN;
    else if (op == "+=") type = Token::OP_PLUS_ASSIGN;
    else if (op == "-=") type = Token::OP_MINUS_ASSIGN;
    else if (op == "*=") type = Token::OP_MULTIPLY_ASSIGN;
    else if (op == "/=") type = Token::OP_DIVIDE_ASSIGN;
    else if (op == "%=") type = Token::OP_MODULO_ASSIGN;
    else if (op == "**=") type = Token::OP_POWER_ASSIGN;
    else if (op == "==") type = Token::OP_EQUALS;
    else if (op == "!=") type = Token::OP_NOT_EQUALS;
    else if (op == "<") type = Token::OP_LESS;
    else if (op == ">") type = Token::OP_GREATER;
    else if (op == "<=") type = Token::OP_LESS_EQUAL;
    else if (op == ">=") type = Token::OP_GREATER_EQUAL;
    else if (op == "(") type = Token::SEP_LPAREN;
    else if (op == ")") type = Token::SEP_RPAREN;
    else if (op == "[") type = Token::SEP_LBRACKET;
    else if (op == "]") type = Token::SEP_RBRACKET;
    else if (op == "{") type = Token::SEP_LBRACE;
    else if (op == "}") type = Token::SEP_RBRACE;
    else if (op == ",") type = Token::SEP_COMMA;
    else if (op == ":") type = Token::SEP_COLON;
    else if (op == ".") type = Token::SEP_DOT;
    else if (op == ";") type = Token::SEP_SEMICOLON;
    else type = Token::ERROR;
    
    return Token(type, op, startLine, startColumn);
}

Token Lexer::handleComment() {
    std::string value;
    value += advance(); // consume the '#'
    
    // Consume characters until end of line or end of file
    while (peek() != '\n' && peek() != '\0') {
        value += advance();
    }
    
    return Token(Token::COMMENT, value, line, column - static_cast<int>(value.length()));
}

std::vector<Token> Lexer::handleIndentation(const std::string& lineContent) {
    std::vector<Token> tokens;
    
    // Count the indentation level (number of spaces or equivalent tabs)
    int indent = 0;
    for (size_t i = 0; i < lineContent.length(); ++i) {
        if (lineContent[i] == ' ') {
            indent++;
        } else if (lineContent[i] == '\t') {
            // Assuming tab = 4 spaces (can be adjusted)
            indent += 4;
        } else {
            break;
        }
    }
    
    // If this is a blank line or comment-only line, don't change indentation
    bool isBlankOrComment = true;
    for (size_t i = indent; i < lineContent.length(); ++i) {
        if (lineContent[i] == '#') {
            break; // Rest of line is comment
        }
        if (!std::isspace(lineContent[i])) {
            isBlankOrComment = false;
            break;
        }
    }
    
    if (isBlankOrComment) {
        return tokens;
    }
    
    // Compare with the current indentation level (top of the stack)
    if (indent > indentStack.top()) {
        // Indentation increased - generate INDENT token
        indentStack.push(indent);
        tokens.push_back(Token(Token::INDENT, std::string(indent, ' '), this->line, 1));
    } else if (indent < indentStack.top()) {
        // Indentation decreased - generate DEDENT tokens until we match
        while (!indentStack.empty() && indent < indentStack.top()) {
            indentStack.pop();
            tokens.push_back(Token(Token::DEDENT, "", this->line, 1));
        }
        
        // If we don't find a matching indentation level, it's an error
        if (indentStack.empty() || indent != indentStack.top()) {
            // Inconsistent indentation - this should be handled as an error
            tokens.push_back(Token(Token::ERROR, "Inconsistent indentation", this->line, 1));
        }
    }
    
    return tokens;
}

std::vector<Lexer::State> Lexer::getStates() const {
    return states;
}

std::vector<Lexer::Transition> Lexer::getTransitions() const {
    return transitions;
}
