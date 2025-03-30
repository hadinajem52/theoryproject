#include "parser.h"
#include <iostream>
#include <sstream>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0), currentState({State::PROGRAM, "PROGRAM"}) {
    initAutomaton();
}

void Parser::initAutomaton() {
    // Define states
    states = {
        {State::PROGRAM, "PROGRAM"},
        {State::STATEMENT, "STATEMENT"},
        {State::EXPRESSION, "EXPRESSION"},
        {State::FUNCTION_DECLARATION, "FUNCTION_DECLARATION"},
        {State::CLASS_DECLARATION, "CLASS_DECLARATION"},
        {State::IF_STATEMENT, "IF_STATEMENT"},
        {State::WHILE_STATEMENT, "WHILE_STATEMENT"},
        {State::FOR_STATEMENT, "FOR_STATEMENT"},
        {State::IMPORT_STATEMENT, "IMPORT_STATEMENT"},
        {State::ASSIGNMENT, "ASSIGNMENT"},
        {State::CALL_EXPRESSION, "CALL_EXPRESSION"},
        {State::ERROR, "ERROR"}
    };
    
    // Define transitions (simplified for visualization)
    transitions = {
        {{State::PROGRAM, "PROGRAM"}, {State::STATEMENT, "STATEMENT"}, "statement"},
        {{State::STATEMENT, "STATEMENT"}, {State::EXPRESSION, "EXPRESSION"}, "expression"},
        {{State::STATEMENT, "STATEMENT"}, {State::FUNCTION_DECLARATION, "FUNCTION_DECLARATION"}, "def keyword"},
        {{State::STATEMENT, "STATEMENT"}, {State::CLASS_DECLARATION, "CLASS_DECLARATION"}, "class keyword"},
        {{State::STATEMENT, "STATEMENT"}, {State::IF_STATEMENT, "IF_STATEMENT"}, "if keyword"},
        {{State::STATEMENT, "STATEMENT"}, {State::WHILE_STATEMENT, "WHILE_STATEMENT"}, "while keyword"},
        {{State::STATEMENT, "STATEMENT"}, {State::FOR_STATEMENT, "FOR_STATEMENT"}, "for keyword"},
        {{State::STATEMENT, "STATEMENT"}, {State::IMPORT_STATEMENT, "IMPORT_STATEMENT"}, "import/from keyword"},
        {{State::EXPRESSION, "EXPRESSION"}, {State::ASSIGNMENT, "ASSIGNMENT"}, "= operator"},
        {{State::EXPRESSION, "EXPRESSION"}, {State::CALL_EXPRESSION, "CALL_EXPRESSION"}, "( after identifier"}
    };
}

std::unique_ptr<ASTNode> Parser::parse() {
    try {
        auto program = parseProgram();
        
        // Check if we consumed all tokens except EOF
        if (current < tokens.size() - 1) {
            Token unconsumed = tokens[current];
            std::cerr << "Warning: Not all tokens were consumed. Remaining token: " 
                      << unconsumed.toString() << std::endl;
        }
        
        return program;
    } catch (const ParseError& error) {
        std::cerr << "Parse error: " << error.what() << std::endl;
        
        // Print context information - show a few tokens before and after the error
        size_t errorPos = current < tokens.size() ? current : tokens.size() - 1;
        size_t start = errorPos > 3 ? errorPos - 3 : 0;
        size_t end = std::min(errorPos + 3, tokens.size() - 1);
        
        std::cerr << "Token context:\n";
        for (size_t i = start; i <= end; i++) {
            std::cerr << (i == errorPos ? " --> " : "     ") 
                      << tokens[i].toString() << std::endl;
        }
        
        currentState = {State::ERROR, "ERROR"};
        return nullptr;
    }
}

std::unique_ptr<Program> Parser::parseProgram() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    
    while (!check(Token::END_OF_FILE)) {
        statements.push_back(parseStatement());
    }
    
    return std::make_unique<Program>(std::move(statements));
}

std::unique_ptr<Statement> Parser::parseStatement() {
    currentState = {State::STATEMENT, "STATEMENT"};
    
    if (match(Token::KEYWORD_DEF)) {
        return parseFunctionDeclaration();
    } else if (match(Token::KEYWORD_CLASS)) {
        return parseClassDeclaration();
    } else if (match(Token::KEYWORD_IF)) {
        return parseIfStatement();
    } else if (match(Token::KEYWORD_WHILE)) {
        return parseWhileStatement();
    } else if (match(Token::KEYWORD_FOR)) {
        return parseForStatement();
    } else if (match(Token::KEYWORD_RETURN)) {
        return parseReturnStatement();
    } else if (match(Token::KEYWORD_IMPORT) || match(Token::KEYWORD_FROM)) {
        // Go back one token since we consumed it
        current--;
        return parseImportStatement();
    } else if (match(Token::KEYWORD_BREAK)) {
        // Consume the newline
        consume(Token::NEWLINE, "Expected newline after 'break'");
        return std::make_unique<BreakStatement>();
    } else if (match(Token::KEYWORD_CONTINUE)) {
        // Consume the newline
        consume(Token::NEWLINE, "Expected newline after 'continue'");
        return std::make_unique<ContinueStatement>();
    } else if (match(Token::KEYWORD_PASS)) {
        // Consume the newline
        consume(Token::NEWLINE, "Expected newline after 'pass'");
        // Pass is a no-op, represented by an empty block
        return std::make_unique<Block>(std::vector<std::unique_ptr<Statement>>());
    } else {
        // Expression statement
        return parseExpressionStatement();
    }
}

std::unique_ptr<FunctionDeclaration> Parser::parseFunctionDeclaration() {
    currentState = {State::FUNCTION_DECLARATION, "FUNCTION_DECLARATION"};
    
    // Function name
    Token name = consume(Token::IDENTIFIER, "Expected function name");
    
    // Parameters
    consume(Token::SEP_LPAREN, "Expected '(' after function name");
    std::vector<FunctionDeclaration::Parameter> parameters = parseFunctionParameters();
    consume(Token::SEP_RPAREN, "Expected ')' after parameters");
    
    // Function body
    consume(Token::SEP_COLON, "Expected ':' after function declaration");
    consume(Token::NEWLINE, "Expected newline after function declaration");
    auto body = parseBlock();
    
    return std::make_unique<FunctionDeclaration>(name.value, std::move(parameters), std::move(body));
}

std::vector<FunctionDeclaration::Parameter> Parser::parseFunctionParameters() {
    std::vector<FunctionDeclaration::Parameter> parameters;
    
    if (!check(Token::SEP_RPAREN)) {
        do {
            Token paramName = consume(Token::IDENTIFIER, "Expected parameter name");
            
            // Check for default value
            std::unique_ptr<Expression> defaultValue = nullptr;
            if (match(Token::OP_ASSIGN)) {
                defaultValue = parseExpression();
            }
            
            parameters.push_back({paramName.value, std::move(defaultValue)});
        } while (match(Token::SEP_COMMA) && !check(Token::SEP_RPAREN));
    }
    
    return parameters;
}

std::unique_ptr<ClassDeclaration> Parser::parseClassDeclaration() {
    currentState = {State::CLASS_DECLARATION, "CLASS_DECLARATION"};
    
    // Class name
    Token name = consume(Token::IDENTIFIER, "Expected class name");
    
    // Base classes
    std::vector<std::string> baseClasses;
    if (match(Token::SEP_LPAREN)) {
        if (!check(Token::SEP_RPAREN)) {
            do {
                Token baseClass = consume(Token::IDENTIFIER, "Expected base class name");
                baseClasses.push_back(baseClass.value);
            } while (match(Token::SEP_COMMA) && !check(Token::SEP_RPAREN));
        }
        consume(Token::SEP_RPAREN, "Expected ')' after base classes");
    }
    
    // Class body
    consume(Token::SEP_COLON, "Expected ':' after class declaration");
    consume(Token::NEWLINE, "Expected newline after class declaration");
    auto body = parseBlock();
    
    return std::make_unique<ClassDeclaration>(name.value, std::move(baseClasses), std::move(body));
}

std::unique_ptr<ImportStatement> Parser::parseImportStatement() {
    currentState = {State::IMPORT_STATEMENT, "IMPORT_STATEMENT"};
    
    ImportStatement::ImportType type;
    std::string module;
    std::vector<std::string> items;
    std::string alias;
    
    if (match(Token::KEYWORD_FROM)) {
        // from module import items
        type = ImportStatement::IMPORT_FROM;
        
        // Module name
        Token moduleName = consume(Token::IDENTIFIER, "Expected module name after 'from'");
        module = moduleName.value;
        
        // Import keyword
        consume(Token::KEYWORD_IMPORT, "Expected 'import' after module name");
        
        // Import items
        do {
            Token item = consume(Token::IDENTIFIER, "Expected identifier after 'import'");
            items.push_back(item.value);
        } while (match(Token::SEP_COMMA));
    } else {
        // import module [as alias]
        consume(Token::KEYWORD_IMPORT, "Expected 'import'");
        
        // Module name
        Token moduleName = consume(Token::IDENTIFIER, "Expected module name after 'import'");
        module = moduleName.value;
        
        // Check for alias
        if (match(Token::IDENTIFIER) && tokens[current-1].value == "as") {
            type = ImportStatement::IMPORT_ALIAS;
            Token aliasName = consume(Token::IDENTIFIER, "Expected alias name after 'as'");
            alias = aliasName.value;
        } else {
            type = ImportStatement::IMPORT_MODULE;
        }
    }
    
    // Consume the newline
    consume(Token::NEWLINE, "Expected newline after import statement");
    
    return std::make_unique<ImportStatement>(type, module, items, alias);
}

std::unique_ptr<Statement> Parser::parseIfStatement() {
    currentState = {State::IF_STATEMENT, "IF_STATEMENT"};
    
    // If condition
    auto condition = parseExpression();
    consume(Token::SEP_COLON, "Expected ':' after if condition");
    consume(Token::NEWLINE, "Expected newline after if statement");
    auto body = parseBlock();
    
    IfStatement::Branch ifBranch = {std::move(condition), std::move(body)};
    
    // Elif branches
    std::vector<IfStatement::Branch> elifBranches;
    while (match(Token::KEYWORD_ELIF)) {
        auto elifCondition = parseExpression();
        consume(Token::SEP_COLON, "Expected ':' after elif condition");
        consume(Token::NEWLINE, "Expected newline after elif statement");
        auto elifBody = parseBlock();
        
        elifBranches.push_back({std::move(elifCondition), std::move(elifBody)});
    }
    
    // Else branch
    std::unique_ptr<Block> elseBranch = nullptr;
    if (match(Token::KEYWORD_ELSE)) {
        consume(Token::SEP_COLON, "Expected ':' after else");
        consume(Token::NEWLINE, "Expected newline after else statement");
        elseBranch = parseBlock();
    }
    
    return std::make_unique<IfStatement>(std::move(ifBranch), std::move(elifBranches), std::move(elseBranch));
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
    currentState = {State::WHILE_STATEMENT, "WHILE_STATEMENT"};
    
    // While condition
    auto condition = parseExpression();
    consume(Token::SEP_COLON, "Expected ':' after while condition");
    consume(Token::NEWLINE, "Expected newline after while statement");
    auto body = parseBlock();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Statement> Parser::parseForStatement() {
    currentState = {State::FOR_STATEMENT, "FOR_STATEMENT"};
    
    // For loop variable
    auto variable = parseExpression();
    
    // Check if variable is an identifier or a valid expression
    consume(Token::KEYWORD_IN, "Expected 'in' after for loop variable");
    
    // For loop iterable
    auto iterable = parseExpression();
    
    consume(Token::SEP_COLON, "Expected ':' after for loop iterable");
    consume(Token::NEWLINE, "Expected newline after for statement");
    auto body = parseBlock();
    
    return std::make_unique<ForStatement>(std::move(variable), std::move(iterable), std::move(body));
}

std::unique_ptr<Statement> Parser::parseReturnStatement() {
    std::unique_ptr<Expression> value = nullptr;
    
    // Check if there's a return value
    if (!check(Token::NEWLINE)) {
        value = parseExpression();
    }
    
    consume(Token::NEWLINE, "Expected newline after return statement");
    
    return std::make_unique<ReturnStatement>(std::move(value));
}

std::unique_ptr<Block> Parser::parseBlock() {
    // Expect an INDENT token at the start of a block
    consume(Token::INDENT, "Expected indented block");
    
    std::vector<std::unique_ptr<Statement>> statements;
    
    // Parse statements until we hit a DEDENT
    while (!check(Token::DEDENT) && !check(Token::END_OF_FILE)) {
        statements.push_back(parseStatement());
    }
    
    // Consume the DEDENT
    if (!check(Token::END_OF_FILE)) {
        consume(Token::DEDENT, "Expected dedent at end of block");
    }
    
    return std::make_unique<Block>(std::move(statements));
}

std::unique_ptr<Statement> Parser::parseExpressionStatement() {
    std::unique_ptr<Expression> expr = parseExpression();
    
    // Handle assignment
    if (match(Token::OP_ASSIGN)) {
        auto target = std::move(expr);
        auto value = parseExpression();
        consume(Token::NEWLINE, "Expected newline after assignment");
        return std::make_unique<AssignmentStatement>(std::move(target), std::move(value));
    }
    
    // Regular expression statement
    consume(Token::NEWLINE, "Expected newline after expression");
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<Expression> Parser::parseExpression() {
    currentState = {State::EXPRESSION, "EXPRESSION"};
    return parseLogicalOr();
}

std::unique_ptr<Expression> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();
    
    while (match(Token::KEYWORD_OR)) {
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpression>(
            BinaryExpression::OR,
            std::move(expr),
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseLogicalAnd() {
    auto expr = parseEquality();
    
    while (match(Token::KEYWORD_AND)) {
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpression>(
            BinaryExpression::AND,
            std::move(expr),
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseEquality() {
    auto expr = parseComparison();
    
    while (match({Token::OP_EQUALS, Token::OP_NOT_EQUALS})) {
        Token::Type op = tokens[current - 1].type;
        auto right = parseComparison();
        
        BinaryExpression::Operator bop = (op == Token::OP_EQUALS) 
            ? BinaryExpression::EQUAL 
            : BinaryExpression::NOT_EQUAL;
            
        expr = std::make_unique<BinaryExpression>(
            bop,
            std::move(expr),
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseComparison() {
    auto expr = parseTerm();
    
    while (match({Token::OP_LESS, Token::OP_GREATER, Token::OP_LESS_EQUAL, Token::OP_GREATER_EQUAL})) {
        Token::Type op = tokens[current - 1].type;
        auto right = parseTerm();
        
        BinaryExpression::Operator bop;
        switch (op) {
            case Token::OP_LESS: bop = BinaryExpression::LESS; break;
            case Token::OP_GREATER: bop = BinaryExpression::GREATER; break;
            case Token::OP_LESS_EQUAL: bop = BinaryExpression::LESS_EQUAL; break;
            case Token::OP_GREATER_EQUAL: bop = BinaryExpression::GREATER_EQUAL; break;
            default: bop = BinaryExpression::EQUAL; break; // Shouldn't happen
        }
        
        expr = std::make_unique<BinaryExpression>(
            bop,
            std::move(expr),
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseTerm() {
    auto expr = parseFactor();
    
    while (match({Token::OP_PLUS, Token::OP_MINUS})) {
        Token::Type op = tokens[current - 1].type;
        auto right = parseFactor();
        
        BinaryExpression::Operator bop = (op == Token::OP_PLUS) 
            ? BinaryExpression::ADD 
            : BinaryExpression::SUBTRACT;
            
        expr = std::make_unique<BinaryExpression>(
            bop,
            std::move(expr),
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseFactor() {
    auto expr = parseUnary();
    
    while (match({Token::OP_MULTIPLY, Token::OP_DIVIDE, Token::OP_MODULO})) {
        Token::Type op = tokens[current - 1].type;
        auto right = parseUnary();
        
        BinaryExpression::Operator bop;
        switch (op) {
            case Token::OP_MULTIPLY: bop = BinaryExpression::MULTIPLY; break;
            case Token::OP_DIVIDE: bop = BinaryExpression::DIVIDE; break;
            case Token::OP_MODULO: bop = BinaryExpression::MODULO; break;
            default: bop = BinaryExpression::MULTIPLY; break; // Shouldn't happen
        }
        
        expr = std::make_unique<BinaryExpression>(
            bop,
            std::move(expr),
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseUnary() {
    if (match({Token::OP_MINUS, Token::KEYWORD_NOT, Token::OP_BITNOT})) {
        Token::Type op = tokens[current - 1].type;
        auto right = parseUnary();
        
        UnaryExpression::Operator uop;
        switch (op) {
            case Token::OP_MINUS: uop = UnaryExpression::NEGATE; break;
            case Token::KEYWORD_NOT: uop = UnaryExpression::NOT; break;
            case Token::OP_BITNOT: uop = UnaryExpression::BITWISE_NOT; break;
            default: uop = UnaryExpression::NEGATE; break; // Shouldn't happen
        }
        
        return std::make_unique<UnaryExpression>(uop, std::move(right));
    }
    
    return parseCall();
}

std::unique_ptr<Expression> Parser::parseCall() {
    auto expr = parsePrimary();
    
    // Function call
    while (true) {
        if (match(Token::SEP_LPAREN)) {
            currentState = {State::CALL_EXPRESSION, "CALL_EXPRESSION"};
            
            // Parse arguments
            std::vector<std::unique_ptr<Expression>> arguments;
            if (!check(Token::SEP_RPAREN)) {
                do {
                    arguments.push_back(parseExpression());
                } while (match(Token::SEP_COMMA) && !check(Token::SEP_RPAREN));
            }
            
            consume(Token::SEP_RPAREN, "Expected ')' after arguments");
            
            expr = std::make_unique<CallExpression>(std::move(expr), std::move(arguments));
        } else if (match(Token::SEP_DOT)) {
            // Member access (obj.prop)
            Token name = consume(Token::IDENTIFIER, "Expected property name after '.'");
            expr = std::make_unique<MemberExpression>(std::move(expr), name.value);
        } else if (match(Token::SEP_LBRACKET)) {
            // Subscript access (array[index])
            auto index = parseExpression();
            consume(Token::SEP_RBRACKET, "Expected ']' after index");
            expr = std::make_unique<SubscriptExpression>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    // Handle literals
    if (match(Token::LITERAL_INT)) {
        return std::make_unique<Literal>(
            Literal::INTEGER, 
            tokens[current - 1].value
        );
    } else if (match(Token::LITERAL_FLOAT)) {
        return std::make_unique<Literal>(
            Literal::FLOAT, 
            tokens[current - 1].value
        );
    } else if (match(Token::LITERAL_STRING)) {
        return std::make_unique<Literal>(
            Literal::STRING, 
            tokens[current - 1].value
        );
    } else if (match(Token::KEYWORD_TRUE)) {
        return std::make_unique<Literal>(
            Literal::BOOLEAN, 
            "true"
        );
    } else if (match(Token::KEYWORD_FALSE)) {
        return std::make_unique<Literal>(
            Literal::BOOLEAN, 
            "false"
        );
    } else if (match(Token::KEYWORD_NONE)) {
        return std::make_unique<Literal>(
            Literal::NONE, 
            "null"
        );
    } else if (match(Token::IDENTIFIER)) {
        return std::make_unique<Identifier>(tokens[current - 1].value);
    } else if (match(Token::SEP_LPAREN)) {
        // Grouping
        auto expr = parseExpression();
        consume(Token::SEP_RPAREN, "Expected ')' after expression");
        return expr;
    } else if (match(Token::SEP_LBRACKET)) {
        // List literal
        return parseListLiteral();
    } else if (match(Token::SEP_LBRACE)) {
        // Dict literal
        return parseDictLiteral();
    }
    
    throw error(peek(), "Expected expression");
}

std::unique_ptr<ListExpression> Parser::parseListLiteral() {
    std::vector<std::unique_ptr<Expression>> elements;
    
    if (!check(Token::SEP_RBRACKET)) {
        do {
            elements.push_back(parseExpression());
        } while (match(Token::SEP_COMMA) && !check(Token::SEP_RBRACKET));
    }
    
    consume(Token::SEP_RBRACKET, "Expected ']' after list elements");
    
    return std::make_unique<ListExpression>(std::move(elements));
}

std::unique_ptr<DictExpression> Parser::parseDictLiteral() {
    std::vector<DictExpression::KeyValuePair> entries;
    
    if (!check(Token::SEP_RBRACE)) {
        do {
            auto key = parseExpression();
            consume(Token::SEP_COLON, "Expected ':' after dict key");
            auto value = parseExpression();
            
            DictExpression::KeyValuePair pair;
            pair.key = std::move(key);
            pair.value = std::move(value);
            
            entries.push_back(std::move(pair));
        } while (match(Token::SEP_COMMA) && !check(Token::SEP_RBRACE));
    }
    
    consume(Token::SEP_RBRACE, "Expected '}' after dict entries");
    
    return std::make_unique<DictExpression>(std::move(entries));
}

// Helper methods for token handling
Token Parser::peek() const {
    if (current >= tokens.size()) {
        return tokens.back(); // Return EOF token
    }
    return tokens[current];
}

Token Parser::advance() {
    // Skip any comment tokens
    do {
        current++;
    } while (current < tokens.size() && tokens[current].type == Token::COMMENT);
    
    if (current > 0) {
        return tokens[current - 1];
    }
    return tokens[current]; // Should never reach here in normal operation
}

bool Parser::check(Token::Type type) const {
    if (current >= tokens.size()) {
        return false;
    }
    return tokens[current].type == type;
}

bool Parser::match(Token::Type type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(const std::vector<Token::Type>& types) {
    for (Token::Type type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(Token::Type type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    
    throw error(peek(), message);
}

void Parser::synchronize() {
    advance();
    
    while (current < tokens.size()) {
        if (tokens[current - 1].type == Token::NEWLINE) {
            return;
        }
        
        switch (peek().type) {
            case Token::KEYWORD_DEF:
            case Token::KEYWORD_CLASS:
            case Token::KEYWORD_IF:
            case Token::KEYWORD_WHILE:
            case Token::KEYWORD_FOR:
            case Token::KEYWORD_RETURN:
            case Token::KEYWORD_IMPORT:
            case Token::KEYWORD_FROM:
                return;
            default:
                // Continue to the next token
                break;
        }
        
        advance();
    }
}

Parser::ParseError Parser::error(const Token& token, const std::string& message) {
    std::stringstream err;
    
    if (token.type == Token::END_OF_FILE) {
        err << "Error at end: " << message;
    } else {
        err << "Error at " << token.line << ":" << token.column 
            << " (" << token.value << "): " << message;
    }
    
    return ParseError(err.str());
}

std::vector<Parser::State> Parser::getStates() const {
    return states;
}

std::vector<Parser::Transition> Parser::getTransitions() const {
    return transitions;
}

Parser::State Parser::getCurrentState() const {
    return currentState;
}
