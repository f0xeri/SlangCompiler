//
// Created by Yaroslav on 22.01.2022.
//
#include "Parser.hpp"

void Parser::parse()
{
    parseImports();
    parseModuleDecl();
    mainModuleNode->block = parseBlock(mainModuleNode->name);
}

bool Parser::parseImports()
{
    while (token.type == TokenType::Import)
    {
        consume(TokenType::Import);
        if (!expect(TokenType::Identifier)) return false;
        // import module
        advance();
    }
    return true;
}

bool Parser::parseModuleDecl()
{
    bool ret = false;
    consume(TokenType::Module);
    if (!expect(TokenType::Identifier)) return false;
    mainModuleNode = new ModuleStatementNode(new VariableExprNode(token.data), new BlockExprNode());
    advance();
    while (token.type != TokenType::Start)
    {
        if (token.type == TokenType::VisibilityType) ret = parseVisibilityOperator();
        else if (token.type == TokenType::Variable)  ret = parseVariableDecl(true);
        else return false;
        advance();
    }

    return ret;
}

BlockExprNode* Parser::parseBlock(VariableExprNode *name) {
    bool blockEnd = false;
    auto *statements = new std::vector<StatementNode*>();
    auto *block = new BlockExprNode(statements);
    while (!blockEnd)
    {
        advance();
        if (token.type == TokenType::If) statements->push_back(parseIfStatement());
        else if (token.type == TokenType::Variable) statements->push_back(parseVariableDecl());
        else if (token.type == TokenType::Output) statements->push_back(parseOutputStatement());
        else if (token.type == TokenType::Return) statements->push_back(parseReturnStatement());
        else if (token.type == TokenType::Let) statements->push_back(parseAssignStatement());
        else if (token.type == TokenType::While) statements->push_back(parseWhileStatement());
        if (token.type == TokenType::End)
        {
            advance();
            std::string endName = token.data;
            advance();
            if (endName == name->value)
                blockEnd = true;
            else
            {
                llvm::errs() << "[ERROR] Expected end of block \"" << name->value << "\", not \"" << endName << "\".\n";
                return block;
            }
        }
    }
    return block;
}

bool Parser::parseVisibilityOperator() {
    advance();
    //bool ret = false;
    if (token.type == TokenType::Function) parseFunctionDecl();
    else if (token.type == TokenType::Procedure) parseFunctionDecl();
    else if (token.type == TokenType::Class) parseTypeDecl();
    return true;
}

bool Parser::parseStatement() {
    return false;
}

FieldVarDecNode* Parser::parseFieldDecl(std::vector<FieldVarDecNode *> *fields, std::string &thisClassName, bool &constructorRequired) {
    FieldVarDecNode *field = nullptr;
    std::string name;
    std::string type;
    bool isPrivate = false;
    Token tok = consume(TokenType::VisibilityType);
    if (token.data == "private") isPrivate = true;
    consume(TokenType::Field);
    consume(TokenType::Minus);
    tok = consume(TokenType::Identifier);
    type = tok.data;
    if (type == "array")
    {
        // parse array
    }
    else
    {
        if (type == thisClassName || currentScope->lookup(type) || oneOfDefaultTypes(type))
        {
            tok = consume(TokenType::Identifier);
            name = tok.data;
            if (!isFieldNameCorrect(fields, name))
            {
                llvm::errs() << "[ERROR] Field \"" << name << "\" already declared in class \"" << thisClassName << "\".\n";
                hasError = true; // error field already exists
            }

            if (token.type == TokenType::Assign || token.type == TokenType::Semicolon)
            {
                bool init = token.type == TokenType::Assign;
                ExprNode* expr;
                if (init) {
                    advance();
                    expr = parseExpression();
                    field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type, expr);
                    constructorRequired = true;
                    if (DEBUG) llvm::outs() << "parsed field " << type << " " << name << "\n";
                }
                else
                {
                    TokenType dataType = init ? token.type : TokenType::Nil;
                    std::string data = init ? token.data : "";
                    if (oneOfDefaultTypes(type))
                    {
                        field = initDefaultType(isPrivate, name, type, dataType, data);
                        if (DEBUG) llvm::outs() << "parsed field " << type << " " << name << " - " << data << "\n";
                    }
                    else
                    {
                        // object fields
                    }
                }
                if (init)
                {
                    //advance();
                    expect(TokenType::Semicolon);
                }
            }
            else error();
        }
    }
    return field;
}

std::vector<FuncParamDecStatementNode*> *Parser::parseFuncParameters()
{
    auto* params = new std::vector<FuncParamDecStatementNode*>();
    consume(TokenType::LParen);
    while (token.type != TokenType::RParen)
    {
        std::string type, name;
        ParameterType parameterType{};
        std::string ptype = consume(TokenType::Identifier).data;
        if (ptype == "in") parameterType = ParameterType::In;
        else if (ptype == "var") parameterType = ParameterType::Var;
        else if (ptype == "out") parameterType = ParameterType::Out;        // probably we dont need "out" type
        type = consume(TokenType::Identifier).data;
        name = consume(TokenType::Identifier).data;
        params->push_back(new FuncParamDecStatementNode(type, new VariableExprNode(name), parameterType));
        //advance();
        if (token.type != TokenType::Comma)
        {
            expect(TokenType::RParen);
        }
        else advance();
    }
    return params;
}

MethodDecNode* Parser::parseMethodDecl(std::vector<MethodDecNode*> *methods, std::string &thisClassName) {
    MethodDecNode *method = nullptr;
    std::string name;
    std::string type;
    std::string thisName;
    bool isPrivate = false;
    BlockExprNode *block = nullptr;
    Token tok = consume(TokenType::VisibilityType);
    if (token.data == "private") isPrivate = true;
    consume(TokenType::Method);
    tok = consume(TokenType::Identifier);
    name = tok.data;
    consume(TokenType::LParen);
    tok = consume(TokenType::Identifier);
    if (tok.data != thisClassName)
    {
        llvm::errs() << "[ERROR] \"this\" should be \"" << thisClassName + "\", not \"" << tok.data << "\".\n";
        hasError = true;
    }
    tok = consume(TokenType::Identifier);
    thisName = tok.data;
    consume(TokenType::RParen);
    auto params = parseFuncParameters();
    advance();
    if (token.type == TokenType::Colon)
    {
        advance();
        type = consume(TokenType::Identifier).data;
    }
    else
    {
        type = "";
    }

    // parse statements

    while (token.type != TokenType::End)
    {
        advance();
    }
    advance();
    tok = consume(TokenType::Identifier);
    if (tok.data != name)
    {
        llvm::errs() << "[ERROR] Expected end of block \"" << name + "\", not \"" << tok.data << "\".\n";
        hasError = true;
    }

    if (token.type != TokenType::Semicolon)
    {
        llvm::errs() << "[ERROR] Unexpected token \"" << token.data + "\", expected \"" + Lexer::getTokenName(TokenType::Semicolon) + "\".\n";
        hasError = true;
    }

    method = new MethodDecNode(type, new VariableExprNode(name), isPrivate, new VariableExprNode(thisName), params, block);
    if (DEBUG) llvm::outs() << "parsed method of class " << thisClassName << " " << type << " " << name << "\n";
    return method;
}

bool Parser::parseTypeDecl() {
    TypeDecStatementNode *typeNode;
    auto *fields = new std::vector<FieldVarDecNode*>();
    auto *methods = new std::vector<MethodDecNode*>();
    std::string name;
    TypeDecStatementNode *parent;
    bool isPrivate = token.data == "private";
    advance();
    bool constructorRequired = false;
    if (token.type == TokenType::Identifier)
    {
        name = token.data;
        advance();
        if (token.type == TokenType::Inherits)
        {
            advance();
            if (token.type == TokenType::Identifier)
            {
                DeclarationNode *decl = currentScope->lookup(token.data);
                if (decl != nullptr)
                {
                    parent = dynamic_cast<TypeDecStatementNode*>(decl);
                    if (parent != nullptr)
                    {
                        if (parent->fields != nullptr) fields->assign(parent->fields->begin(), parent->fields->end());
                        if (parent->methods != nullptr) methods->assign(parent->methods->begin(), parent->methods->end());
                    }

                    Scope *scope = new Scope(currentScope);
                    currentScope = scope;

                    while (token.type != TokenType::End)
                    {
                        advance();
                        if (token.type == TokenType::VisibilityType)
                        {
                            advance();
                            if (token.type == TokenType::Field)
                            {
                                tokensIterator--;
                                token = *tokensIterator;
                                auto field = parseFieldDecl(fields, name, constructorRequired);
                                if (field != nullptr)
                                {
                                    // TODO Check if field name already exists in fields
                                    fields->push_back(field);
                                    //if (DEBUG) llvm::outs() << "parsed field " << field->name->value << " " << field->isPrivate << "\n";
                                }
                                else
                                {
                                    llvm::errs() << "[ERROR] Failed to parse field of class \"" << name << "\".\n";
                                }
                            }
                            if (token.type == TokenType::Method)
                            {
                                tokensIterator--;
                                token = *tokensIterator;
                                auto method = parseMethodDecl(methods, name);
                                if (method != nullptr)
                                {
                                    // TODO Check if method name already exists in methods
                                    methods->push_back(method);
                                }
                            }
                        }
                    }

                    if (constructorRequired)
                    {
                        std::string constructorName = name + "DefaultConstructor";
                        std::string constructorType = "";
                        bool constructorIsPrivate = false;
                        auto *constructorStatements = new std::vector<StatementNode*>();
                        //constructorStatements->push_back(new AssignExprNode());
                        for (auto field : *fields)
                        {
                            constructorStatements->push_back(new AssignExprNode(field->name, field->expr));
                        }
                        auto constructor = new MethodDecNode(constructorType, new VariableExprNode(constructorName), constructorIsPrivate, new VariableExprNode(name),
                                                   nullptr, new BlockExprNode(constructorStatements));
                        methods->push_back(constructor);
                    }

                    advance();
                    if (token.type == TokenType::Identifier && token.data == name && advance().type == TokenType::Semicolon)
                    {
                        currentScope = scope->parent;
                        typeNode = new TypeDecStatementNode(new VariableExprNode(name), isPrivate, fields, methods, parent);
                        currentScope->insert(typeNode);
                    }
                }
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

ExprNode *Parser::parseExpression() {
    return parseStrInterpolation();
}

ExprNode *Parser::parseStrInterpolation() {
    auto result = parseOr();

    return result;
}

ExprNode *Parser::parseOr() {
    auto result = parseAnd();

    while (true)
    {
        if (match(TokenType::Or)) {
            result = new OperatorExprNode(result, TokenType::Or, parseAnd());
            continue;
        }
        break;
    }

    return result;
}

ExprNode *Parser::parseAnd() {
    auto result = parseEquality();

    while (true)
    {
        if (match(TokenType::And)) {
            result = new ConditionalExprNode(result, TokenType::And, parseEquality());
            continue;
        }
        break;
    }

    return result;
}

ExprNode *Parser::parseEquality() {
    auto result = parseConditional();

    while (true)
    {
        if (match(TokenType::Equal)) {
            result = new OperatorExprNode(result, TokenType::Equal, parseConditional());
            continue;
        }
        if (match(TokenType::NotEqual)) {
            result = new OperatorExprNode(result, TokenType::NotEqual, parseConditional());
            continue;
        }
        break;
    }

    return result;
}

ExprNode *Parser::parseConditional() {
    auto result = parseAddSub();

    while (true)
    {
        if (match(TokenType::Less)) {
            result = new OperatorExprNode(result, TokenType::Less, parseAddSub());
            continue;
        }
        if (match(TokenType::LessOrEqual)) {
            result = new OperatorExprNode(result, TokenType::LessOrEqual, parseAddSub());
            continue;
        }
        if (match(TokenType::Greater)) {
            result = new OperatorExprNode(result, TokenType::Greater, parseAddSub());
            continue;
        }
        if (match(TokenType::GreaterOrEqual)) {
            result = new OperatorExprNode(result, TokenType::GreaterOrEqual, parseAddSub());
            continue;
        }
        break;
    }

    return result;
}

ExprNode *Parser::parseAddSub() {
    auto result = parseMulDiv();

    while (true)
    {
        if (match(TokenType::Plus)) {
            result = new OperatorExprNode(result, TokenType::Plus, parseMulDiv());
            continue;
        }
        if (match(TokenType::Minus)) {
            result = new OperatorExprNode(result, TokenType::Minus, parseMulDiv());
        }
        break;
    }

    return result;
}

ExprNode *Parser::parseMulDiv() {
    auto result = parseUnary();

    while (true)
    {
        if (match(TokenType::Multiplication)) {
            result = new OperatorExprNode(result, TokenType::Multiplication, parseUnary());
            continue;
        }
        if (match(TokenType::Division)) {
            result = new OperatorExprNode(result, TokenType::Division, parseUnary());
        }
        break;
    }

    return result;
}

ExprNode *Parser::parseUnary() {
    return match(TokenType::Minus) ? new UnaryOperatorExprNode(TokenType::Minus, parsePrimary()) : parsePrimary();
}

ExprNode *Parser::parsePrimary() {

    Token tok = token;
    advance();
    if (tok.type == TokenType::Boolean) return new BooleanExprNode(tok.data == "true");
    else if (tok.type == TokenType::String)
    {
        if (tok.data.size() == 1) return new CharExprNode(tok.data[0]);
        else return new StringExprNode(tok.data);
    }
    else if (tok.type == TokenType::Identifier)
    {
        // vars, funcs, arrays, fields
        return reinterpret_cast<ExprNode *>(parseVarOrCall());
    }
    else if (tok.type == TokenType::Integer) return new IntExprNode(std::stoi(tok.data));
    else if (tok.type == TokenType::Real) return new RealExprNode(std::stod(tok.data));
    else if (tok.type == TokenType::LParen)
    {
        auto result = parseExpression();
        consume(TokenType::RParen);
        return result;
    }
    llvm::errs() << "[ERROR] Failed to parse expression.\n";
    return nullptr;
}

StatementNode* Parser::parseVarOrCall() {
    tokensIterator--;
    token = *tokensIterator;
    Token tok = token;
    std::string name = tok.data;
    advance();
    if (token.type != TokenType::LParen) return new VariableExprNode(name);
    consume(TokenType::LParen);
    auto *params = new std::vector<ExprNode*>();
    if (token.type != TokenType::RParen)
    {
        while (1)
        {
            if (auto arg = parseExpression())
                params->push_back(arg);
            else return nullptr;
            if (token.type == TokenType::RParen) break;
            consume(TokenType::Comma);
        }
    }
    advance();
    return new CallExprNode(new VariableExprNode(name), params);
}

VarDecStatementNode* Parser::parseVariableDecl(bool isGlobal) {
    std::string type;
    std::string name;
    advance();
    consume(TokenType::Minus);
    ExprNode *expr = nullptr;
    type = consume(TokenType::Identifier).data;
    if (oneOfDefaultTypes(type))
    {
        name = consume(TokenType::Identifier).data;
        if (token.type == TokenType::Assign)
        {
            advance();
            expr = parseExpression();
        }
        expect(TokenType::Semicolon);
    }
    else
    {
        auto typeStatement = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(type));
        if (typeStatement == nullptr)
        {
            llvm::errs() << "[ERROR] Unknown type \"" << type << "\".\n";
        }
        name = consume(TokenType::Identifier).data;
        // arrays and objects
    }
    if (currentScope->lookup(name) != nullptr || oneOfDefaultTypes(name))
    {
        llvm::errs() << "[ERROR] Name conflict - \"" << name << "\".\n";
    }
    auto result = new VarDecStatementNode(type, new VariableExprNode(name), expr, isGlobal);
    if (isGlobal) currentScope->insert(result);
    return result;
}

FuncDecStatementNode *Parser::parseFunctionDecl() {
    tokensIterator--;
    token = *tokensIterator;
    bool isPrivate = false;
    if (token.data == "private") isPrivate = true;
    advance();
    bool isFunction = false;
    if (match(TokenType::Function)) isFunction = true;
    else {
        consume(TokenType::Procedure);
    }
    std::string name = consume(TokenType::Identifier).data;
    std::string type;
    auto params = parseFuncParameters();
    if (isFunction)
    {
        advance();
        consume(TokenType::Colon);
        expect(TokenType::Identifier);
        type = token.data;
    }

    auto block = parseBlock(new VariableExprNode(name));

    if (token.type != TokenType::Semicolon)
    {
        llvm::errs() << "[ERROR] Unexpected token \"" << token.data + "\", expected \"" + Lexer::getTokenName(TokenType::Semicolon) + "\".\n";
        hasError = true;
    }
    auto function = new FuncDecStatementNode(type, new VariableExprNode(name), isPrivate, params, block);
    currentScope->insert(function);
    return function;
}

IfStatementNode *Parser::parseIfStatement() {
    consume(TokenType::If);
    auto expr = parseExpression();
    consume(TokenType::Then);
    BlockExprNode *trueBlock = nullptr;
    BlockExprNode *falseBlock = nullptr;
    auto *elseifNodes = new std::vector<ElseIfStatementNode*>();
    tokensIterator--;
    token = *tokensIterator;

    if (token.type != TokenType::Else && token.type != TokenType::Elseif)
    {
        bool blockEnd = false;
        auto *statements = new std::vector<StatementNode*>();
        auto *block = new BlockExprNode(statements);
        while (!blockEnd)
        {
            advance();
            if (token.type == TokenType::If) statements->push_back(parseIfStatement());
            else if (token.type == TokenType::Variable) statements->push_back(parseVariableDecl());
            else if (token.type == TokenType::Output) statements->push_back(parseOutputStatement());
            else if (token.type == TokenType::Return) statements->push_back(parseReturnStatement());
            else if (token.type == TokenType::Let) statements->push_back(parseAssignStatement());
            if (token.type == TokenType::End || token.type == TokenType::Else || token.type == TokenType::Elseif)
            {
                blockEnd = true;
            }
        }
        trueBlock = new BlockExprNode(statements);
    }
    if (token.type == TokenType::Elseif) elseifNodes->push_back(parseElseIfBlock());
    if (token.type == TokenType::Else) falseBlock = parseElseBlock();
    //advance();
    consume(TokenType::End);
    consume(TokenType::If);
    expect(TokenType::Semicolon);
    return new IfStatementNode(expr, trueBlock, elseifNodes, falseBlock);
}

ElseIfStatementNode *Parser::parseElseIfBlock() {
    bool blockEnd = false;
    auto *statements = new std::vector<StatementNode*>();

    consume(TokenType::Elseif);
    auto expr = parseExpression();
    consume(TokenType::Then);
    ElseIfStatementNode *block = nullptr;
    tokensIterator--;
    token = *tokensIterator;

    while (!blockEnd)
    {
        advance();
        if (token.type == TokenType::If) statements->push_back(parseIfStatement());
        else if (token.type == TokenType::Variable) statements->push_back(parseVariableDecl());
        else if (token.type == TokenType::Output) statements->push_back(parseOutputStatement());
        else if (token.type == TokenType::Return) statements->push_back(parseReturnStatement());
        else if (token.type == TokenType::Let) statements->push_back(parseAssignStatement());
        if (token.type == TokenType::Else)
        {
            blockEnd = true;
        }
    }
    block = new ElseIfStatementNode(expr, new BlockExprNode(statements));
    return block;
}

BlockExprNode *Parser::parseElseBlock() {
    BlockExprNode *falseBlock = nullptr;
    bool blockEnd = false;
    auto *statements = new std::vector<StatementNode*>();
    auto *block = new BlockExprNode(statements);
    while (!blockEnd)
    {
        advance();
        if (token.type == TokenType::If) statements->push_back(parseIfStatement());
        else if (token.type == TokenType::Variable) statements->push_back(parseVariableDecl());
        else if (token.type == TokenType::Output) statements->push_back(parseOutputStatement());
        else if (token.type == TokenType::Return) statements->push_back(parseReturnStatement());
        else if (token.type == TokenType::Let) statements->push_back(parseAssignStatement());
        else if (token.type == TokenType::While) statements->push_back(parseWhileStatement());
        if (token.type == TokenType::End)
        {
            blockEnd = true;
        }
    }
    falseBlock = new BlockExprNode(statements);
    return falseBlock;
}

OutputStatementNode *Parser::parseOutputStatement() {
    consume(TokenType::Output);
    auto expr = parseExpression();
    return new OutputStatementNode(expr);
}

AssignExprNode *Parser::parseAssignStatement() {
    consume(TokenType::Let);
    std::string varName = consume(TokenType::Identifier).data;
    consume(TokenType::Assign);
    auto expr = parseExpression();
    return new AssignExprNode(new VariableExprNode(varName), expr);
}

ReturnStatementNode *Parser::parseReturnStatement() {
    consume(TokenType::Return);
    auto expr = parseExpression();
    expect(TokenType::Semicolon);
    return new ReturnStatementNode(expr);
}

WhileStatementNode *Parser::parseWhileStatement() {
    consume(TokenType::While);
    auto expr = parseExpression();
    expect(TokenType::Repeat);
    BlockExprNode* block = parseBlock(new VariableExprNode("while"));
    expect(TokenType::Semicolon);
    return new WhileStatementNode(expr, block);
}
