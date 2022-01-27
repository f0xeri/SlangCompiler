//
// Created by Yaroslav on 22.01.2022.
//
#include "Parser.hpp"

#define DEBUG 1

void Parser::parse()
{
    parseImports();
    parseModuleDecl();
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
        else if (token.type == TokenType::Variable)       ret = parseVariableDecl();
        else return false;
        advance();
    }

    return ret;
}

bool Parser::parseBlock(VariableExprNode &name) {
    bool blockEnd = false;
    while (!blockEnd)
    {
        // parse statements
        advance();
        if (token.type != TokenType::End)
        {
            consume(TokenType::Identifier);
        }
        if (token.data == name.value)
            blockEnd = true;
        else return false;
    }
    return true;
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

FieldDecNode* Parser::parseFieldDecl(std::vector<FieldDecNode *> *fields, std::string &thisClassName) {
    FieldDecNode *field = nullptr;
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
        if (type == thisClassName || currentScope->lookup(new VariableExprNode(type)) || oneOfDefaultTypes(type))
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
    auto *fields = new std::vector<FieldDecNode*>();
    auto *methods = new std::vector<MethodDecNode*>();
    std::string name;
    TypeDecStatementNode *parent;
    advance();
    if (token.type == TokenType::Identifier)
    {
        name = token.data;
        advance();
        if (token.type == TokenType::Inherits)
        {
            advance();
            if (token.type == TokenType::Identifier)
            {
                DeclarationNode *decl = currentScope->lookup(new VariableExprNode(token.data));
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
                                auto field = parseFieldDecl(fields, name);
                                if (field != nullptr)
                                {
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
                                    methods->push_back(method);
                                }
                            }
                        }
                    }
                    advance();
                    if (token.type == TokenType::Identifier && token.data == name && advance().type == TokenType::Semicolon)
                    {
                        currentScope = scope->parent;
                        typeNode = new TypeDecStatementNode(new VariableExprNode(name), fields, methods, parent);
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
            result = new ConditionalExprNode(result, TokenType::Or, parseAnd());
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
            result = new ConditionalExprNode(result, TokenType::Equal, parseConditional());
            continue;
        }
        if (match(TokenType::NotEqual)) {
            result = new ConditionalExprNode(result, TokenType::NotEqual, parseConditional());
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
            result = new ConditionalExprNode(result, TokenType::Less, parseAddSub());
            continue;
        }
        if (match(TokenType::LessOrEqual)) {
            result = new ConditionalExprNode(result, TokenType::LessOrEqual, parseAddSub());
            continue;
        }
        if (match(TokenType::Greater)) {
            result = new ConditionalExprNode(result, TokenType::Greater, parseAddSub());
            continue;
        }
        if (match(TokenType::GreaterOrEqual)) {
            result = new ConditionalExprNode(result, TokenType::GreaterOrEqual, parseAddSub());
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
            result = new OperatorExprNode(result, Operations::Plus, parseMulDiv());
            continue;
        }
        if (match(TokenType::Minus)) {
            result = new OperatorExprNode(result, Operations::Minus, parseMulDiv());
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
            result = new OperatorExprNode(result, Operations::Multiply, parseUnary());
            continue;
        }
        if (match(TokenType::Division)) {
            result = new OperatorExprNode(result, Operations::Divide, parseUnary());
        }
        break;
    }

    return result;
}

ExprNode *Parser::parseUnary() {
    return match(TokenType::Minus) ? new UnaryOperatorExprNode(Operations::Minus, parsePrimary()) : parsePrimary();
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
        return nullptr;
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

ExprNode *Parser::parseVarOrCall() {
    Token tok = consume(TokenType::Identifier);
    std::string name = tok.data;
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
            advance();
            if (token.type == TokenType::RParen) break;
            consume(TokenType::Comma);
        }
    }
    advance();
    return new CallExprNode(new VariableExprNode(name), params);
}

VarDecStatementNode* Parser::parseVariableDecl() {
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
        // arrays and objects
    }
    auto result = new VarDecStatementNode(type, new VariableExprNode(name), expr);
    currentScope->insert(result);
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
        type = consume(TokenType::Identifier).data;
    }
    // parse statements

    while (token.type != TokenType::End)
    {
        advance();
    }
    advance();
    Token tok = consume(TokenType::Identifier);
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
    auto function = new FuncDecStatementNode(type, new VariableExprNode(name), isPrivate, params, nullptr);
    currentScope->insert(function);
    return function;
}
