#include "js_generator.h"

std::string JSCodeGenerator::generate(ASTNode* node) {
    output.str("");
    output.clear();
    indentLevel = 0;
    
    node->accept(*this);
    
    return output.str();
}

void JSCodeGenerator::visitProgram(Program* node) {
    for (const auto& stmt : node->statements) {
        stmt->accept(*this);
        output << std::endl;
    }
}

void JSCodeGenerator::visitExpressionStatement(ExpressionStatement* node) {
    output << getIndent();
    node->expression->accept(*this);
    output << ";";
}

void JSCodeGenerator::visitBinaryExpression(BinaryExpression* node) {
    output << "(";
    node->left->accept(*this);
    
    switch (node->op) {
        case BinaryExpression::ADD: output << " + "; break;
        case BinaryExpression::SUBTRACT: output << " - "; break;
        case BinaryExpression::MULTIPLY: output << " * "; break;
        case BinaryExpression::DIVIDE: output << " / "; break;
        case BinaryExpression::MODULO: output << " % "; break;
        case BinaryExpression::POWER: output << " ** "; break;
        case BinaryExpression::EQUAL: output << " === "; break;
        case BinaryExpression::NOT_EQUAL: output << " !== "; break;
        case BinaryExpression::LESS: output << " < "; break;
        case BinaryExpression::GREATER: output << " > "; break;
        case BinaryExpression::LESS_EQUAL: output << " <= "; break;
        case BinaryExpression::GREATER_EQUAL: output << " >= "; break;
        case BinaryExpression::AND: output << " && "; break;
        case BinaryExpression::OR: output << " || "; break;
        case BinaryExpression::BITAND: output << " & "; break;
        case BinaryExpression::BITOR: output << " | "; break;
        case BinaryExpression::BITXOR: output << " ^ "; break;
        case BinaryExpression::LSHIFT: output << " << "; break;
        case BinaryExpression::RSHIFT: output << " >> "; break;
    }
    
    node->right->accept(*this);
    output << ")";
}

void JSCodeGenerator::visitUnaryExpression(UnaryExpression* node) {
    switch (node->op) {
        case UnaryExpression::NEGATE: output << "-"; break;
        case UnaryExpression::NOT: output << "!"; break;
        case UnaryExpression::BITWISE_NOT: output << "~"; break;
    }
    
    output << "(";
    node->operand->accept(*this);
    output << ")";
}

void JSCodeGenerator::visitLiteral(Literal* node) {
    switch (node->type) {
        case Literal::STRING:
            output << node->value;
            break;
        case Literal::BOOLEAN:
            output << (node->value == "true" ? "true" : "false");
            break;
        case Literal::NONE:
            output << "null";
            break;
        default:
            output << node->value;
            break;
    }
}

void JSCodeGenerator::visitIdentifier(Identifier* node) {
    // In JavaScript, 'self' becomes 'this'
    if (node->name == "self") {
        output << "this";
    } else {
        output << node->name;
    }
}

void JSCodeGenerator::visitMemberExpression(MemberExpression* node) {
    // Handle property access with special case for self -> this conversion
    if (auto idObj = dynamic_cast<Identifier*>(node->object.get())) {
        if (idObj->name == "self") {
            output << "this";
        } else {
            output << idObj->name;
        }
    } else {
        node->object->accept(*this);
    }
    
    output << "." << node->property;
}

void JSCodeGenerator::indent() {
    indentLevel += 2;
}

void JSCodeGenerator::dedent() {
    indentLevel = std::max(0, indentLevel - 2);
}

std::string JSCodeGenerator::getIndent() const {
    return std::string(indentLevel, ' ');
}

std::string JSCodeGenerator::generateExpression(Expression* expr) {
    std::stringstream tempOutput;
    std::stringstream* oldOutput = &output;
    output = tempOutput;
    
    expr->accept(*this);
    
    std::string result = output.str();
    output = *oldOutput;
    
    return result;
}
