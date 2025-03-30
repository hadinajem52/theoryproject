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

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    pos = 0;
    line = 1;
    column = 1;
    indentStack = std::stack<int>();
    indentStack.push(0);
    currentState = {State::START, "START"};
    
    while (pos < sourceCode.length()) {
        char current = peek();
        
        // Handle different character types
        if (current == '#') {
            tokens.push_back(handleComment());
        } else if (isalpha(current) || current == '_') {
            tokens.push_back(handleIdentifier());
        } else if (isdigit(current)) {
            tokens.push_back(handleNumber());
        } else if (current == '"' || current == '\'') {
            tokens.push_back(handleString());
        } else if (ispunct(current) && current != '#') {
            tokens.push_back(handleOperator());
        } else if (current == '\n') {
            tokens.push_back(Token(Token::NEWLINE, "\\n", line, column));
            advance();
            line++;
            column = 1;
            
            // Handle indentation for the next line
            std::string nextLine;
            size_t tempPos = pos;
            while (tempPos < sourceCode.length() && sourceCode[tempPos] != '\n') {
                nextLine += sourceCode[tempPos++];
            }
            
            std::vector<Token> indentTokens = handleIndentation(nextLine);
            tokens.insert(tokens.end(), indentTokens.begin(), indentTokens.end());
        } else if (isspace(current)) {
            skipWhitespace();
        } else {
            // Unrecognized character
            tokens.push_back(Token(Token::ERROR, std::string(1, current), line, column));
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

char Lexer::peek() const {
    if (pos >= sourceCode.length()) {
        return '\0'; // End of input
    }
    return sourceCode[pos];
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
    
    // Consume the string content
    while (peek() != quoteType && peek() != '\0' && peek() != '\n') {
        if (peek() == '\\' && pos + 1 < sourceCode.length()) {
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
    
    return Token(Token::LITERAL_STRING, str, startLine, startColumn);
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
    currentState = {State::COMMENT, "COMMENT"};
    
    std::string value;
    value += advance(); // consume the '#'
    
    // Consume characters until end of line or end of file
    while (peek() != '\n' && peek() != '\0') {
        value += advance();
    }
    
    // Return to start state
    currentState = {State::START, "START"};
    
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
