#include "token.h"

// Static keyword map initialization
const std::unordered_map<std::string, Token::Type> Token::keywordMap = {
    {"def", KEYWORD_DEF},
    {"if", KEYWORD_IF},
    {"else", KEYWORD_ELSE},
    {"elif", KEYWORD_ELIF},
    {"while", KEYWORD_WHILE},
    {"for", KEYWORD_FOR},
    {"in", KEYWORD_IN},
    {"return", KEYWORD_RETURN},
    {"import", KEYWORD_IMPORT},
    {"from", KEYWORD_FROM},
    {"pass", KEYWORD_PASS},
    {"break", KEYWORD_BREAK},
    {"continue", KEYWORD_CONTINUE},
    {"class", KEYWORD_CLASS},
    {"None", KEYWORD_NONE},
    {"True", KEYWORD_TRUE},
    {"False", KEYWORD_FALSE},
    {"and", KEYWORD_AND},
    {"or", KEYWORD_OR},
    {"not", KEYWORD_NOT},
    {"lambda", KEYWORD_LAMBDA}  // Add lambda keyword mapping
};

Token::Type Token::getKeywordType(const std::string& keyword) {
    auto it = keywordMap.find(keyword);
    if (it != keywordMap.end()) {
        return it->second;
    }
    return IDENTIFIER;
}

std::string Token::typeToString(Type type) {
    switch (type) {
        // Keywords
        case KEYWORD_DEF: return "KEYWORD_DEF";
        case KEYWORD_IF: return "KEYWORD_IF";
        case KEYWORD_ELSE: return "KEYWORD_ELSE";
        case KEYWORD_ELIF: return "KEYWORD_ELIF";
        case KEYWORD_WHILE: return "KEYWORD_WHILE";
        case KEYWORD_FOR: return "KEYWORD_FOR";
        case KEYWORD_IN: return "KEYWORD_IN";
        case KEYWORD_RETURN: return "KEYWORD_RETURN";
        case KEYWORD_IMPORT: return "KEYWORD_IMPORT";
        case KEYWORD_FROM: return "KEYWORD_FROM";
        case KEYWORD_PASS: return "KEYWORD_PASS";
        case KEYWORD_BREAK: return "KEYWORD_BREAK";
        case KEYWORD_CONTINUE: return "KEYWORD_CONTINUE";
        case KEYWORD_CLASS: return "KEYWORD_CLASS";
        case KEYWORD_NONE: return "KEYWORD_NONE";
        case KEYWORD_TRUE: return "KEYWORD_TRUE";
        case KEYWORD_FALSE: return "KEYWORD_FALSE";
        case KEYWORD_AND: return "KEYWORD_AND";
        case KEYWORD_OR: return "KEYWORD_OR";
        case KEYWORD_NOT: return "KEYWORD_NOT";
        case KEYWORD_LAMBDA: return "KEYWORD_LAMBDA";  // Add case for KEYWORD_LAMBDA
        
        // Identifier
        case IDENTIFIER: return "IDENTIFIER";
        
        // Literals
        case LITERAL_INT: return "LITERAL_INT";
        case LITERAL_FLOAT: return "LITERAL_FLOAT";
        case LITERAL_STRING: return "LITERAL_STRING";
        case LITERAL_BYTES: return "LITERAL_BYTES";
        
        // Operators
        case OP_PLUS: return "OP_PLUS";
        case OP_MINUS: return "OP_MINUS";
        case OP_MULTIPLY: return "OP_MULTIPLY";
        case OP_DIVIDE: return "OP_DIVIDE";
        case OP_MODULO: return "OP_MODULO";
        case OP_POWER: return "OP_POWER";
        case OP_LSHIFT: return "OP_LSHIFT";
        case OP_RSHIFT: return "OP_RSHIFT";
        case OP_BITAND: return "OP_BITAND";
        case OP_BITOR: return "OP_BITOR";
        case OP_BITXOR: return "OP_BITXOR";
        case OP_BITNOT: return "OP_BITNOT";
        case OP_ASSIGN: return "OP_ASSIGN";
        case OP_PLUS_ASSIGN: return "OP_PLUS_ASSIGN";
        case OP_MINUS_ASSIGN: return "OP_MINUS_ASSIGN";
        case OP_MULTIPLY_ASSIGN: return "OP_MULTIPLY_ASSIGN";
        case OP_DIVIDE_ASSIGN: return "OP_DIVIDE_ASSIGN";
        case OP_MODULO_ASSIGN: return "OP_MODULO_ASSIGN";
        case OP_POWER_ASSIGN: return "OP_POWER_ASSIGN";
        case OP_EQUALS: return "OP_EQUALS";
        case OP_NOT_EQUALS: return "OP_NOT_EQUALS";
        case OP_LESS: return "OP_LESS";
        case OP_GREATER: return "OP_GREATER";
        case OP_LESS_EQUAL: return "OP_LESS_EQUAL";
        case OP_GREATER_EQUAL: return "OP_GREATER_EQUAL";
        
        // Separators
        case SEP_LPAREN: return "SEP_LPAREN";
        case SEP_RPAREN: return "SEP_RPAREN";
        case SEP_LBRACKET: return "SEP_LBRACKET";
        case SEP_RBRACKET: return "SEP_RBRACKET";
        case SEP_LBRACE: return "SEP_LBRACE";
        case SEP_RBRACE: return "SEP_RBRACE";
        case SEP_COMMA: return "SEP_COMMA";
        case SEP_COLON: return "SEP_COLON";
        case SEP_DOT: return "SEP_DOT";
        case SEP_SEMICOLON: return "SEP_SEMICOLON";
        
        // Special tokens
        case INDENT: return "INDENT";
        case DEDENT: return "DEDENT";
        case NEWLINE: return "NEWLINE";
        case COMMENT: return "COMMENT";
        case WHITESPACE: return "WHITESPACE";
        
        // End markers
        case END_OF_FILE: return "END_OF_FILE";
        case ERROR: return "ERROR";
        
        default: return "UNKNOWN";
    }
}
