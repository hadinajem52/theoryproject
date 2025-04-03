#include "parser.h"
#include <iostream>
#include <sstream>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0), currentState({PROGRAM, "PROGRAM"}), 
      hasError(false), errorMessage("") {
    initAutomaton();
}

void Parser::initAutomaton() {
    // Define states
    states = {
        {PROGRAM, "PROGRAM"},
        {STATEMENT, "STATEMENT"},
        {EXPRESSION, "EXPRESSION"},
        {FUNCTION_DECLARATION, "FUNCTION_DECLARATION"},
        {CLASS_DECLARATION, "CLASS_DECLARATION"},
        {IF_STATEMENT, "IF_STATEMENT"},
        {WHILE_STATEMENT, "WHILE_STATEMENT"},
        {FOR_STATEMENT, "FOR_STATEMENT"},
        {IMPORT_STATEMENT, "IMPORT_STATEMENT"},
        {ASSIGNMENT, "ASSIGNMENT"},
        {CALL_EXPRESSION, "CALL_EXPRESSION"},
        {ERROR, "ERROR"}
    };
    
    // Define transitions (simplified for visualization)
    transitions = {
        {{PROGRAM, "PROGRAM"}, {STATEMENT, "STATEMENT"}, "statement"},
        {{STATEMENT, "STATEMENT"}, {EXPRESSION, "EXPRESSION"}, "expression"},
        {{STATEMENT, "STATEMENT"}, {FUNCTION_DECLARATION, "FUNCTION_DECLARATION"}, "def keyword"},
        {{STATEMENT, "STATEMENT"}, {CLASS_DECLARATION, "CLASS_DECLARATION"}, "class keyword"},
        {{STATEMENT, "STATEMENT"}, {IF_STATEMENT, "IF_STATEMENT"}, "if keyword"},
        {{STATEMENT, "STATEMENT"}, {WHILE_STATEMENT, "WHILE_STATEMENT"}, "while keyword"},
        {{STATEMENT, "STATEMENT"}, {FOR_STATEMENT, "FOR_STATEMENT"}, "for keyword"},
        {{STATEMENT, "STATEMENT"}, {IMPORT_STATEMENT, "IMPORT_STATEMENT"}, "import/from keyword"},
        {{EXPRESSION, "EXPRESSION"}, {ASSIGNMENT, "ASSIGNMENT"}, "= operator"},
        {{EXPRESSION, "EXPRESSION"}, {CALL_EXPRESSION, "CALL_EXPRESSION"}, "( after identifier"}
    };
}

// Add this method to help with debugging
void debugASTStructure(ASTNode* node, int level = 0) {
    if (!node) return;
    
    std::string indent(level * 2, ' ');
    std::cerr << indent;
    
    if (auto program = dynamic_cast<Program*>(node)) {
        std::cerr << "Program (" << program->statements.size() << " statements)" << std::endl;
        for (auto& stmt : program->statements) {
            debugASTStructure(stmt.get(), level + 1);
        }
    } else if (auto block = dynamic_cast<Block*>(node)) {
        std::cerr << "Block (" << block->statements.size() << " statements)" << std::endl;
        for (auto& stmt : block->statements) {
            debugASTStructure(stmt.get(), level + 1);
        }
    } else if (auto ifStmt = dynamic_cast<IfStatement*>(node)) {
        std::cerr << "IfStatement" << std::endl;
        std::cerr << indent << "  Condition: ";
        debugASTStructure(ifStmt->ifBranch.condition.get(), 0);
        std::cerr << indent << "  Then: " << std::endl;
        debugASTStructure(ifStmt->ifBranch.body.get(), level + 2);
        
        for (auto& elif : ifStmt->elifBranches) {
            std::cerr << indent << "  Elif Condition: ";
            debugASTStructure(elif.condition.get(), 0);
            std::cerr << indent << "  Elif Body: " << std::endl;
            debugASTStructure(elif.body.get(), level + 2);
        }
        
        if (ifStmt->elseBranch) {
            std::cerr << indent << "  Else: " << std::endl;
            debugASTStructure(ifStmt->elseBranch.get(), level + 2);
        }
    } else {
        std::cerr << "Node of type: " << typeid(*node).name() << std::endl;
    }
}

std::unique_ptr<ASTNode> Parser::parse() {
    hasError = false;
    errorMessage = "";
    
    try {
        auto program = parseProgram();
        
        // Add debugging to visualize AST structure
        std::cerr << "AST Structure:" << std::endl;
        debugASTStructure(program.get());
        
        // Check if we consumed all tokens except EOF
        if (current < tokens.size() - 1) {
            Token unconsumed = tokens[current];
            std::cerr << "Warning: Not all tokens were consumed. Remaining token: " 
                      << unconsumed.toString() << std::endl;
        }
        
        return program;
    } catch (const ParseError& error) {
        hasError = true;
        errorMessage = error.what();
        
        std::cerr << "Parse error: " << errorMessage << std::endl;
        
        // Print context information - show a few tokens before and after the error
        size_t errorPos = current < tokens.size() ? current : tokens.size() - 1;
        size_t start = errorPos > 3 ? errorPos - 3 : 0;
        size_t end = std::min(errorPos + 3, tokens.size() - 1);
        
        std::cerr << "Token context:\n";
        for (size_t i = start; i <= end; i++) {
            std::cerr << (i == errorPos ? " --> " : "     ") 
                      << tokens[i].toString() << std::endl;
        }
        
        currentState = {ERROR, "ERROR"};
        return nullptr;
    }
}

std::unique_ptr<Program> Parser::parseProgram() {
    std::vector<std::unique_ptr<Statement>> statements;
    
    // Skip any leading newlines
    while (match(Token::NEWLINE)) {
        // Continue skipping consecutive newlines
    }
    
    // Keep track of the current indentation level
    int programIndentLevel = 0;
    
    while (!check(Token::END_OF_FILE)) {
        // Skip any DEDENT tokens between top-level statements
        while (match(Token::DEDENT)) {
            programIndentLevel--;
            // Skip these DEDENT tokens
        }
        
        // Skip any newlines between statements
        while (match(Token::NEWLINE)) {
            // Skip newlines
        }
        
        // Stop at EOF
        if (check(Token::END_OF_FILE)) {
            break;
        }
        
        auto stmt = parseStatement();
        if (stmt) {  // Check if statement is not null
            statements.push_back(std::move(stmt));
        }
    }
    
    return std::make_unique<Program>(std::move(statements));
}

std::unique_ptr<Statement> Parser::parseStatement() {
    currentState = {STATEMENT, "STATEMENT"};
    
    // Skip empty lines (consecutive newlines) and DEDENT tokens
    while (match(Token::NEWLINE) || match(Token::DEDENT)) {
        // Continue skipping newlines and dedents
    }
    
    if (current >= tokens.size()) {
        return nullptr; // End of file reached after newlines
    }
    
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
    } else if (check(Token::IDENTIFIER) && peek().value == "try") {
        // Try statement - check but don't consume the token
        match(Token::IDENTIFIER); // Now consume it since we've confirmed it's "try"
        return parseTryStatement();
    } else if (check(Token::END_OF_FILE)) {
        return nullptr; // End of file reached
    } else {
        // Expression statement
        return parseExpressionStatement();
    }
}

std::unique_ptr<FunctionDeclaration> Parser::parseFunctionDeclaration() {
    currentState = {FUNCTION_DECLARATION, "FUNCTION_DECLARATION"};
    
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
    currentState = {CLASS_DECLARATION, "CLASS_DECLARATION"};
    
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
    currentState = {IMPORT_STATEMENT, "IMPORT_STATEMENT"};
    
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
    currentState = {IF_STATEMENT, "IF_STATEMENT"};
    
    // If condition
    auto condition = parseExpression();
    consume(Token::SEP_COLON, "Expected ':' after if condition");
    consume(Token::NEWLINE, "Expected newline after if statement");
    auto body = parseBlock();
    
    IfStatement::Branch ifBranch = {std::move(condition), std::move(body)};
    
    // Elif branches
    std::vector<IfStatement::Branch> elifBranches;
    
    // Skip newlines before checking for elif/else
    while (match(Token::NEWLINE)) {
        // Skip these newlines
    }
    
    while (check(Token::KEYWORD_ELIF)) {
        match(Token::KEYWORD_ELIF); // Consume the elif token
        auto elifCondition = parseExpression();
        consume(Token::SEP_COLON, "Expected ':' after elif condition");
        consume(Token::NEWLINE, "Expected newline after elif statement");
        auto elifBody = parseBlock();
        
        elifBranches.push_back({std::move(elifCondition), std::move(elifBody)});
        
        // Skip newlines before checking for next elif/else
        while (match(Token::NEWLINE)) {
            // Skip these newlines
        }
    }
    
    // Else branch
    std::unique_ptr<Block> elseBranch = nullptr;
    if (check(Token::KEYWORD_ELSE)) {
        match(Token::KEYWORD_ELSE); // Consume the else token
        consume(Token::SEP_COLON, "Expected ':' after else");
        consume(Token::NEWLINE, "Expected newline after else statement");
        elseBranch = parseBlock();
    }
    
    return std::make_unique<IfStatement>(std::move(ifBranch), std::move(elifBranches), std::move(elseBranch));
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
    currentState = {WHILE_STATEMENT, "WHILE_STATEMENT"};
    
    // While condition
    auto condition = parseExpression();
    consume(Token::SEP_COLON, "Expected ':' after while condition");
    consume(Token::NEWLINE, "Expected newline after while statement");
    auto body = parseBlock();
    
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Statement> Parser::parseForStatement() {
    currentState = {FOR_STATEMENT, "FOR_STATEMENT"};
    
    // Start with an empty list of target expressions
    std::vector<std::unique_ptr<Expression>> targets;
    
    // Parse the first target expression
    targets.push_back(parseExpression());
    
    // If we find a comma, continue parsing target expressions
    while (match(Token::SEP_COMMA)) {
        targets.push_back(parseExpression());
    }
    
    // Expect 'in' keyword
    consume(Token::KEYWORD_IN, "Expected 'in' after for loop variable");
    
    // For loop iterable
    auto iterable = parseExpression();
    
    consume(Token::SEP_COLON, "Expected ':' after for loop iterable");
    consume(Token::NEWLINE, "Expected newline after for statement");
    auto body = parseBlock();
    
    // If we have multiple targets, create a list expression to represent the tuple
    std::unique_ptr<Expression> target;
    if (targets.size() == 1) {
        target = std::move(targets[0]);
    } else {
        target = std::make_unique<ListExpression>(std::move(targets));
    }
    
    return std::make_unique<ForStatement>(std::move(target), std::move(iterable), std::move(body));
}

std::unique_ptr<Statement> Parser::parseReturnStatement() {
    std::unique_ptr<Expression> value = nullptr;
    
    // Check if there's a return value
    if (!check(Token::NEWLINE)) {
        // Parse the first expression
        value = parseExpression();
        
        // If we find commas, we're returning multiple values (a tuple)
        if (match(Token::SEP_COMMA)) {
            // Start a vector with the first expression we already parsed
            std::vector<std::unique_ptr<Expression>> elements;
            elements.push_back(std::move(value));
            
            // Parse remaining expressions
            do {
                elements.push_back(parseExpression());
            } while (match(Token::SEP_COMMA));
            
            // Create a tuple expression from all the elements
            value = std::make_unique<ListExpression>(std::move(elements));
        }
    }
    
    consume(Token::NEWLINE, "Expected newline after return statement");
    
    return std::make_unique<ReturnStatement>(std::move(value));
}

std::unique_ptr<Block> Parser::parseBlock() {
    // Skip any comments or blank lines before the INDENT
    while (current < tokens.size() && 
           (tokens[current].type == Token::COMMENT || 
            tokens[current].type == Token::NEWLINE)) {
        advance();
    }
    
    // Expect an INDENT token at the start of a block
    consume(Token::INDENT, "Expected indented block");
    
    std::vector<std::unique_ptr<Statement>> statements;
    
    // Parse statements until we hit a DEDENT or EOF
    while (!check(Token::DEDENT) && !check(Token::END_OF_FILE)) {
        // Skip any newlines between statements in the block
        while (match(Token::NEWLINE)) {
            // Skip these newlines
        }
        
        // Check again after skipping newlines
        if (check(Token::DEDENT) || check(Token::END_OF_FILE)) {
            break;
        }
        
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    
    // Consume the DEDENT token if present
    if (check(Token::DEDENT)) {
        consume(Token::DEDENT, "Expected dedent at end of block");
    }
    
    return std::make_unique<Block>(std::move(statements));
}

std::unique_ptr<Statement> Parser::parseExpressionStatement() {
    std::unique_ptr<Expression> expr = parseExpression();
    
    // Handle assignments
    if (check(Token::OP_ASSIGN) || 
        check(Token::OP_PLUS_ASSIGN) || 
        check(Token::OP_MINUS_ASSIGN) ||
        check(Token::OP_MULTIPLY_ASSIGN) || 
        check(Token::OP_DIVIDE_ASSIGN) || 
        check(Token::OP_MODULO_ASSIGN)) {
        
        auto target = std::move(expr);
        Token::Type assignType = peek().type;
        advance(); // Consume the assignment operator
        
        auto value = parseExpression();
        
        // Check for EOF or DEDENT or consume newline
        if (!check(Token::END_OF_FILE) && !check(Token::DEDENT)) {
            consume(Token::NEWLINE, "Expected newline after assignment");
        }
        
        // For compound assignments (+=, -=, etc.), we need to create the equivalent
        // of: a = a + b, a = a - b, etc.
        if (assignType != Token::OP_ASSIGN) {
            BinaryExpression::Operator op;
            
            // Map each compound assignment to its binary operator
            switch (assignType) {
                case Token::OP_PLUS_ASSIGN: op = BinaryExpression::ADD; break;
                case Token::OP_MINUS_ASSIGN: op = BinaryExpression::SUBTRACT; break;
                case Token::OP_MULTIPLY_ASSIGN: op = BinaryExpression::MULTIPLY; break;
                case Token::OP_DIVIDE_ASSIGN: op = BinaryExpression::DIVIDE; break;
                case Token::OP_MODULO_ASSIGN: op = BinaryExpression::MODULO; break;
                default: op = BinaryExpression::ADD; break; // Shouldn't happen
            }
            
            // Create a target clone for the right side of the operation
            auto targetClone = target->clone();
            
            // Create compound expression (a + b, a - b, etc.)
            auto compoundExpr = std::make_unique<BinaryExpression>(
                op, 
                std::move(targetClone), 
                std::move(value)
            );
            
            // Make this the value of the assignment
            return std::make_unique<AssignmentStatement>(std::move(target), std::move(compoundExpr));
        }
        
        return std::make_unique<AssignmentStatement>(std::move(target), std::move(value));
    }
    
    // Handle docstrings (triple-quoted strings which appear as consecutive string literals)
    // Check if we just parsed a string literal and the next token is also a string literal
    if (auto literal = dynamic_cast<Literal*>(expr.get())) {
        if (literal->type == Literal::STRING && check(Token::LITERAL_STRING)) {
            // This is likely a triple-quoted string (docstring)
            std::string docstringContent = literal->value;
            
            // Consume the content of the triple quoted string
            advance();
            Token contentToken = tokens[current - 1];
            docstringContent += contentToken.value;
            
            // If there's a third string token (closing triple quotes), consume it
            if (check(Token::LITERAL_STRING)) {
                advance();
                Token closingToken = tokens[current - 1];
                docstringContent += closingToken.value;
            }
            
            // Update the literal with the full docstring content
            literal->value = docstringContent;
        }
    }
    
    // Regular expression statement
    // Check for EOF or DEDENT or consume newline
    if (!check(Token::END_OF_FILE) && !check(Token::DEDENT)) {
        consume(Token::NEWLINE, "Expected newline after expression");
    }
    
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<Expression> Parser::parseExpression() {
    currentState = {EXPRESSION, "EXPRESSION"};
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
    
    return parsePower();
}

std::unique_ptr<Expression> Parser::parsePower() {
    auto expr = parseCall();
    
    while (match(Token::OP_POWER)) {
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpression>(
            BinaryExpression::POWER,
            std::move(expr),
            std::move(right)
        );
    }
    
    return expr;
}

std::unique_ptr<Expression> Parser::parseCall() {
    auto expr = parsePrimary();
    
    // Function call
    while (true) {
        if (match(Token::SEP_LPAREN)) {
            currentState = {CALL_EXPRESSION, "CALL_EXPRESSION"};
            
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
            // Check if this is a slice notation or regular subscript
            
            // Try to parse the start expression (may be omitted in slices like [:end])
            std::unique_ptr<Expression> startExpr = nullptr;
            if (!check(Token::SEP_COLON)) {
                startExpr = parseExpression();
            }
            
            // If we find a colon, this is slice notation
            if (match(Token::SEP_COLON)) {
                // This is a slice notation [start:end:step]
                std::unique_ptr<Expression> endExpr = nullptr;
                std::unique_ptr<Expression> stepExpr = nullptr;
                
                // Parse the end expression (may be omitted in slices like [start:])
                if (!check(Token::SEP_COLON) && !check(Token::SEP_RBRACKET)) {
                    endExpr = parseExpression();
                }
                
                // Check for step part [start:end:step]
                if (match(Token::SEP_COLON)) {
                    // Parse the step expression (may be omitted in slices like [start:end:])
                    if (!check(Token::SEP_RBRACKET)) {
                        stepExpr = parseExpression();
                    }
                }
                
                consume(Token::SEP_RBRACKET, "Expected ']' after slice");
                expr = std::make_unique<SliceExpression>(
                    std::move(expr), 
                    std::move(startExpr), 
                    std::move(endExpr), 
                    std::move(stepExpr)
                );
            } else {
                // Regular subscript access (array[index])
                consume(Token::SEP_RBRACKET, "Expected ']' after index");
                expr = std::make_unique<SubscriptExpression>(std::move(expr), std::move(startExpr));
            }
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
    } else if (match(Token::LITERAL_FSTRING)) {
        // Handle f-string literals
        return std::make_unique<FStringLiteral>(
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
    } else if (match(Token::KEYWORD_LAMBDA)) {
        // Parse lambda expression
        std::vector<LambdaExpression::Parameter> parameters;
        
        // Parse parameters (if any)
        if (!check(Token::SEP_COLON)) {
            do {
                Token paramName = consume(Token::IDENTIFIER, "Expected parameter name");
                
                // Check for default value
                std::unique_ptr<Expression> defaultValue = nullptr;
                if (match(Token::OP_ASSIGN)) {
                    defaultValue = parseExpression();
                }
                
                parameters.push_back({paramName.value, std::move(defaultValue)});
            } while (match(Token::SEP_COMMA) && !check(Token::SEP_COLON));
        }
        
        // Consume the colon and parse the body expression
        consume(Token::SEP_COLON, "Expected ':' after lambda parameters");
        auto body = parseExpression();
        
        return std::make_unique<LambdaExpression>(std::move(parameters), std::move(body));
    } else if (match(Token::IDENTIFIER)) {
        return std::make_unique<Identifier>(tokens[current - 1].value);
    } else if (match(Token::SEP_LPAREN)) {
        // Check for empty tuple
        if (match(Token::SEP_RPAREN)) {
            // Empty tuple
            return std::make_unique<ListExpression>(std::vector<std::unique_ptr<Expression>>());
        }
        
        // Parse the first expression
        auto expr = parseExpression();
        
        // If we find a comma, this is a tuple, not just grouping
        if (match(Token::SEP_COMMA)) {
            // Start a vector with the first expression we already parsed
            std::vector<std::unique_ptr<Expression>> elements;
            elements.push_back(std::move(expr));
            
            // Parse remaining expressions
            if (!check(Token::SEP_RPAREN)) {  // Allow trailing comma
                do {
                    elements.push_back(parseExpression());
                } while (match(Token::SEP_COMMA) && !check(Token::SEP_RPAREN));
            }
            
            consume(Token::SEP_RPAREN, "Expected ')' after tuple elements");
            return std::make_unique<ListExpression>(std::move(elements));
        }
        
        // Regular grouping
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

std::unique_ptr<Expression> Parser::parseListLiteral() {
    std::vector<std::unique_ptr<Expression>> elements;
    
    // Skip any newlines or indentation after the opening bracket
    while (match(Token::NEWLINE) || match(Token::INDENT)) {
        // Skip these tokens
    }
    
    // Check for empty list
    if (check(Token::SEP_RBRACKET)) {
        consume(Token::SEP_RBRACKET, "Expected ']' after list elements");
        return std::make_unique<ListExpression>(std::move(elements));
    }
    
    // Parse the first expression
    auto firstExpr = parseExpression();
    
    // Check if this is a list comprehension
    if (match(Token::KEYWORD_FOR)) {
        // This is a list comprehension
        // Parse the target variable
        auto variable = parseExpression();
        
        // Consume "in"
        consume(Token::KEYWORD_IN, "Expected 'in' after for variable in list comprehension");
        
        // Parse the iterable
        auto iterable = parseExpression();
        
        // Check for optional "if" condition
        std::unique_ptr<Expression> condition = nullptr;
        if (match(Token::KEYWORD_IF)) {
            condition = parseExpression();
        }
        
        // Skip any newlines or dedents before the closing bracket
        while (match(Token::NEWLINE) || match(Token::INDENT) || match(Token::DEDENT)) {
            // Skip these tokens
        }
        
        // Consume the closing bracket
        consume(Token::SEP_RBRACKET, "Expected ']' after list comprehension");
        
        // Create a ListComprehension node
        return std::make_unique<ListComprehension>(
            std::move(firstExpr),  // The expression to evaluate for each item
            std::move(variable),   // The iteration variable
            std::move(iterable),   // The iterable collection
            std::move(condition)   // Optional condition (may be null)
        );
    }
    
    // Regular list - add the first expression we already parsed
    elements.push_back(std::move(firstExpr));
    
    // Parse remaining elements if any
    while (match(Token::SEP_COMMA)) {
        // Skip any newlines or indentation after comma
        while (match(Token::NEWLINE) || match(Token::INDENT) || match(Token::DEDENT)) {
            // Skip these tokens
        }
        
        if (check(Token::SEP_RBRACKET)) {
            break; // Allow trailing comma
        }
        elements.push_back(parseExpression());
    }
    
    // Skip any newlines or dedents before the closing bracket
    while (match(Token::NEWLINE) || match(Token::INDENT) || match(Token::DEDENT)) {
        // Skip these tokens
    }
    
    consume(Token::SEP_RBRACKET, "Expected ']' after list elements");
    
    return std::make_unique<ListExpression>(std::move(elements));
}

std::unique_ptr<DictExpression> Parser::parseDictLiteral() {
    std::vector<DictExpression::KeyValuePair> entries;
    
    // Skip any newlines or indentation after the opening brace
    while (match(Token::NEWLINE) || match(Token::INDENT)) {
        // Skip these tokens
    }
    
    if (!check(Token::SEP_RBRACE)) {
        do {
            // Skip any newlines or indentation before the key
            while (match(Token::NEWLINE) || match(Token::INDENT)) {
                // Skip these tokens
            }
            
            auto key = parseExpression();
            consume(Token::SEP_COLON, "Expected ':' after dict key");
            auto value = parseExpression();
            
            DictExpression::KeyValuePair pair;
            pair.key = std::move(key);
            pair.value = std::move(value);
            
            entries.push_back(std::move(pair));
            
            // Skip any newlines or indentation after the value
            while (match(Token::NEWLINE) || match(Token::INDENT) || match(Token::DEDENT)) {
                // Skip these tokens
            }
            
        } while (match(Token::SEP_COMMA) && !check(Token::SEP_RBRACE));
        
        // Skip any trailing newlines, indentation, or dedents before the closing brace
        while (match(Token::NEWLINE) || match(Token::INDENT) || match(Token::DEDENT)) {
            // Skip these tokens
        }
    }
    
    consume(Token::SEP_RBRACE, "Expected '}' after dict entries");
    
    return std::make_unique<DictExpression>(std::move(entries));
}

std::unique_ptr<Statement> Parser::parseTryStatement() {
    // No need to consume the 'try' keyword here anymore since we did it in parseStatement
    
    // Consume the colon and newline
    consume(Token::SEP_COLON, "Expected ':' after 'try'");
    consume(Token::NEWLINE, "Expected newline after 'try' statement");
    
    // Parse try block
    auto tryBlock = parseBlock();
    
    // Parse except blocks
    std::vector<TryExceptStatement::CatchBlock> catchBlocks;
    
    // Skip any newlines or DEDENTs before checking for except
    while (match(Token::NEWLINE) || match(Token::DEDENT)) {
        // Skip these tokens
    }
    
    while (check(Token::IDENTIFIER) && peek().value == "except") {
        match(Token::IDENTIFIER); // Consume the "except" token
        
        TryExceptStatement::CatchBlock catchBlock;
        
        // Check if there's an exception type
        if (!check(Token::SEP_COLON)) {
            if (match(Token::IDENTIFIER)) {
                catchBlock.exceptionType = tokens[current-1].value;
                
                // Check for 'as variable'
                if (match(Token::IDENTIFIER) && tokens[current-1].value == "as") {
                    Token var = consume(Token::IDENTIFIER, "Expected variable name after 'as'");
                    catchBlock.variable = var.value;
                }
            }
        }
        
        // Parse except block
        consume(Token::SEP_COLON, "Expected ':' after 'except'");
        consume(Token::NEWLINE, "Expected newline after 'except' statement");
        catchBlock.body = parseBlock();
        
        catchBlocks.push_back(std::move(catchBlock));
        
        // Skip any newlines or DEDENTs after the except block
        while (match(Token::NEWLINE) || match(Token::DEDENT)) {
            // Skip these tokens
        }
    }
    
    // Parse optional finally block
    std::unique_ptr<Block> finallyBlock = nullptr;
    
    if (check(Token::IDENTIFIER) && peek().value == "finally") {
        match(Token::IDENTIFIER); // Consume the "finally" token
        
        consume(Token::SEP_COLON, "Expected ':' after 'finally'");
        consume(Token::NEWLINE, "Expected newline after 'finally' statement");
        finallyBlock = parseBlock();
    }
    
    return std::make_unique<TryExceptStatement>(
        std::move(tryBlock),
        std::move(catchBlocks),
        std::move(finallyBlock)
    );
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

bool Parser::hasParseError() const {
    return hasError;
}

std::string Parser::getErrorMessage() const {
    return errorMessage;
}
