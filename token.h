#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class Token {
public:
    enum Type {
        // Keywords
        KEYWORD_DEF, KEYWORD_IF, KEYWORD_ELSE, KEYWORD_ELIF, KEYWORD_WHILE, 
        KEYWORD_FOR, KEYWORD_IN, KEYWORD_RETURN, KEYWORD_IMPORT, KEYWORD_FROM,
        KEYWORD_PASS, KEYWORD_BREAK, KEYWORD_CONTINUE, KEYWORD_CLASS, KEYWORD_NONE,
        KEYWORD_TRUE, KEYWORD_FALSE, KEYWORD_AND, KEYWORD_OR, KEYWORD_NOT,
        
        // Identifiers
        IDENTIFIER,
        
        // Literals
        LITERAL_INT, LITERAL_FLOAT, LITERAL_STRING, LITERAL_BYTES, LITERAL_FSTRING,
        
        // Operators
        OP_PLUS, OP_MINUS, OP_MULTIPLY, OP_DIVIDE, OP_MODULO, OP_POWER,
        OP_LSHIFT, OP_RSHIFT, OP_BITAND, OP_BITOR, OP_BITXOR, OP_BITNOT,
        OP_ASSIGN, OP_PLUS_ASSIGN, OP_MINUS_ASSIGN, OP_MULTIPLY_ASSIGN,
        OP_DIVIDE_ASSIGN, OP_MODULO_ASSIGN, OP_POWER_ASSIGN,
        OP_EQUALS, OP_NOT_EQUALS, OP_LESS, OP_GREATER, OP_LESS_EQUAL, OP_GREATER_EQUAL,
        
        // Separators
        SEP_LPAREN, SEP_RPAREN, SEP_LBRACKET, SEP_RBRACKET, SEP_LBRACE, SEP_RBRACE,
        SEP_COMMA, SEP_COLON, SEP_DOT, SEP_SEMICOLON,
        
        // Special tokens
        INDENT, DEDENT, NEWLINE, COMMENT, WHITESPACE,
        
        // End markers
        END_OF_FILE, ERROR
    };
    
    Token(Type type, const std::string& value, int line, int column)
        : type(type), value(value), line(line), column(column) {}
    
    // Copy constructor
    Token(const Token& other) = default;
    
    // Move constructor
    Token(Token&& other) = default;
    
    // Copy assignment
    Token& operator=(const Token& other) = default;
    
    // Move assignment
    Token& operator=(Token&& other) = default;
    
    bool isKeyword() const {
        return type >= KEYWORD_DEF && type <= KEYWORD_NOT;
    }
    
    bool isOperator() const {
        return type >= OP_PLUS && type <= OP_GREATER_EQUAL;
    }
    
    bool isSeparator() const {
        return type >= SEP_LPAREN && type <= SEP_SEMICOLON;
    }
    
    bool isLiteral() const {
        return type >= LITERAL_INT && type <= LITERAL_BYTES;
    }
    
    std::string toString() const {
        return typeToString(type) + " '" + value + "' at " + 
               std::to_string(line) + ":" + std::to_string(column);
    }
    
    static std::string typeToString(Type type);
    static Type getKeywordType(const std::string& keyword);
    
    Type type;
    std::string value;
    int line;
    int column;
    
private:
    static const std::unordered_map<std::string, Type> keywordMap;
};
