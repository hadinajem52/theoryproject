#include "ast.h"
#include <sstream>

std::string Program::toString() const {
    std::stringstream ss;
    ss << "Program {\n";
    for (const auto& stmt : statements) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

std::string Literal::toString() const {
    std::stringstream ss;
    ss << "Literal(";
    switch (type) {
        case INTEGER: ss << "INTEGER"; break;
        case FLOAT: ss << "FLOAT"; break;
        case STRING: ss << "STRING"; break;
        case BOOLEAN: ss << "BOOLEAN"; break;
        case NONE: ss << "NONE"; break;
    }
    ss << ", " << value << ")";
    return ss.str();
}

std::string Identifier::toString() const {
    return "Identifier(" + name + ")";
}

std::string BinaryExpression::toString() const {
    std::stringstream ss;
    ss << "BinaryExpression(";
    
    switch (op) {
        case ADD: ss << "ADD"; break;
        case SUBTRACT: ss << "SUBTRACT"; break;
        case MULTIPLY: ss << "MULTIPLY"; break;
        case DIVIDE: ss << "DIVIDE"; break;
        case MODULO: ss << "MODULO"; break;
        case POWER: ss << "POWER"; break;
        case EQUAL: ss << "EQUAL"; break;
        case NOT_EQUAL: ss << "NOT_EQUAL"; break;
        case LESS: ss << "LESS"; break;
        case GREATER: ss << "GREATER"; break;
        case LESS_EQUAL: ss << "LESS_EQUAL"; break;
        case GREATER_EQUAL: ss << "GREATER_EQUAL"; break;
        case AND: ss << "AND"; break;
        case OR: ss << "OR"; break;
        case BITAND: ss << "BITAND"; break;
        case BITOR: ss << "BITOR"; break;
        case BITXOR: ss << "BITXOR"; break;
        case LSHIFT: ss << "LSHIFT"; break;
        case RSHIFT: ss << "RSHIFT"; break;
    }
    
    ss << ", " << left->toString() << ", " << right->toString() << ")";
    return ss.str();
}

std::string UnaryExpression::toString() const {
    std::stringstream ss;
    ss << "UnaryExpression(";
    
    switch (op) {
        case NEGATE: ss << "NEGATE"; break;
        case NOT: ss << "NOT"; break;
        case BITWISE_NOT: ss << "BITWISE_NOT"; break;
    }
    
    ss << ", " << operand->toString() << ")";
    return ss.str();
}

std::string ExpressionStatement::toString() const {
    return "ExpressionStatement(" + expression->toString() + ")";
}
