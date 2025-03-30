#include "ast_extended.h"
#include <sstream>

std::string AssignmentStatement::toString() const {
    return "AssignmentStatement(" + target->toString() + " = " + value->toString() + ")";
}

std::string Block::toString() const {
    std::stringstream ss;
    ss << "Block {\n";
    for (const auto& stmt : statements) {
        ss << "  " << stmt->toString() << "\n";
    }
    ss << "}";
    return ss.str();
}

std::string FunctionDeclaration::toString() const {
    std::stringstream ss;
    ss << "FunctionDeclaration(" << name << ", [";
    
    bool first = true;
    for (const auto& param : parameters) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        
        ss << param.name;
        if (param.defaultValue) {
            ss << "=" << param.defaultValue->toString();
        }
    }
    
    ss << "], " << body->toString() << ")";
    return ss.str();
}

std::string CallExpression::toString() const {
    std::stringstream ss;
    ss << "CallExpression(" << callee->toString() << ", [";
    
    bool first = true;
    for (const auto& arg : arguments) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << arg->toString();
    }
    
    ss << "])";
    return ss.str();
}

std::string IfStatement::toString() const {
    std::stringstream ss;
    ss << "IfStatement(if " << ifBranch.condition->toString() 
       << " then " << ifBranch.body->toString();
    
    for (const auto& elif : elifBranches) {
        ss << " elif " << elif.condition->toString() 
           << " then " << elif.body->toString();
    }
    
    if (elseBranch) {
        ss << " else " << elseBranch->toString();
    }
    
    ss << ")";
    return ss.str();
}

std::string WhileStatement::toString() const {
    return "WhileStatement(" + condition->toString() + ", " + body->toString() + ")";
}

std::string ForStatement::toString() const {
    return "ForStatement(" + variable->toString() + " in " + 
           iterable->toString() + ", " + body->toString() + ")";
}

std::string ReturnStatement::toString() const {
    std::stringstream ss;
    ss << "ReturnStatement(";
    if (value) {
        ss << value->toString();
    }
    ss << ")";
    return ss.str();
}

std::string BreakStatement::toString() const {
    return "BreakStatement()";
}

std::string ContinueStatement::toString() const {
    return "ContinueStatement()";
}

std::string ClassDeclaration::toString() const {
    std::stringstream ss;
    ss << "ClassDeclaration(" << name;
    
    if (!baseClasses.empty()) {
        ss << " extends [";
        bool first = true;
        for (const auto& base : baseClasses) {
            if (!first) {
                ss << ", ";
            }
            first = false;
            ss << base;
        }
        ss << "]";
    }
    
    ss << ", " << body->toString() << ")";
    return ss.str();
}

std::string MemberExpression::toString() const {
    return "MemberExpression(" + object->toString() + "." + property + ")";
}

std::string SubscriptExpression::toString() const {
    return "SubscriptExpression(" + object->toString() + "[" + index->toString() + "])";
}

std::string ListExpression::toString() const {
    std::stringstream ss;
    ss << "ListExpression([";
    
    bool first = true;
    for (const auto& elem : elements) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << elem->toString();
    }
    
    ss << "])";
    return ss.str();
}

std::string DictExpression::toString() const {
    std::stringstream ss;
    ss << "DictExpression({";
    
    bool first = true;
    for (const auto& entry : entries) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << entry.key->toString() << ": " << entry.value->toString();
    }
    
    ss << "})";
    return ss.str();
}

std::string ImportStatement::toString() const {
    switch (type) {
        case IMPORT_MODULE: {
            return "import " + module;
        }
        case IMPORT_FROM: {
            std::string result = "from " + module + " import ";
            bool first = true;
            for (const auto& name : items) { 
                if (!first) result += ", ";
                result += name;
                first = false;
            }
            return result;
        }
        case IMPORT_ALIAS: {
            return "import " + module + " as " + alias;
        }
        default:
            return "Unknown import type";
    }
}
