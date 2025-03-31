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

// FStringLiteral implementation
FStringLiteral::FStringLiteral(const std::string& value) : rawValue(value) {
    parseContent(value);
}

std::string FStringLiteral::toString() const {
    std::stringstream ss;
    ss << "FStringLiteral(";
    
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) ss << " + ";
        
        if (parts[i].isExpression) {
            ss << "{" << parts[i].expression->toString() << "}";
        } else {
            ss << "\"" << parts[i].text << "\"";
        }
    }
    
    ss << ")";
    return ss.str();
}

void FStringLiteral::parseContent(const std::string& content) {
    size_t pos = 0;
    std::string textPart;
    
    while (pos < content.length()) {
        // Find the next expression start ('{')
        size_t exprStart = content.find('{', pos);
        
        if (exprStart == std::string::npos) {
            // No more expressions, add remaining text
            textPart = content.substr(pos);
            if (!textPart.empty()) {
                parts.push_back(Part(textPart));
            }
            break;
        }
        
        // Check if this is an escaped '{' (i.e., '{{')
        if (exprStart + 1 < content.length() && content[exprStart + 1] == '{') {
            // Add text up to the first '{' and include the second '{' in the next search
            textPart = content.substr(pos, exprStart - pos + 1);
            if (!textPart.empty()) {
                parts.push_back(Part(textPart));
            }
            pos = exprStart + 2;
            continue;
        }
        
        // Add text part before expression
        if (exprStart > pos) {
            textPart = content.substr(pos, exprStart - pos);
            parts.push_back(Part(textPart));
        }
        
        // Extract expression text between '{' and '}'
        pos = exprStart + 1; // Move past '{'
        std::string exprText = extractExpressionText(content, pos);
        
        // Create a placeholder expression for now
        // In a full implementation, this would parse the expression text into an AST
        parts.push_back(Part(std::make_unique<Identifier>(exprText)));
        
        // pos is now after the closing '}'
    }
}

std::string FStringLiteral::extractExpressionText(const std::string& content, size_t& pos) {
    size_t start = pos;
    int braceDepth = 1; // We've already consumed one '{'
    
    while (pos < content.length() && braceDepth > 0) {
        char c = content[pos++];
        if (c == '{') {
            braceDepth++;
        } else if (c == '}') {
            braceDepth--;
        }
    }
    
    // If we didn't find a closing brace, back up
    if (braceDepth > 0) {
        pos = content.length();
    }
    
    // Extract the expression text (excluding the closing '}')
    return content.substr(start, (pos - start) - 1);
}
