//
// Created by Yaroslav on 22.01.2022.
//
#include "Parser.hpp"

void Parser::parse()
{
    parseImports();
    for (auto &module : *importedModules)
    {
        for (auto &decl : module->currentScope->symbols)
        {
            if (dynamic_cast<FuncDecStatementNode*>(decl.second) != nullptr)
            {
                auto func = dynamic_cast<FuncDecStatementNode*>(decl.second);
                if (!func->isPrivate)
                {
                    auto declFunc = new FuncDecStatementNode(*func);
                    declFunc->block = nullptr;
                    currentScope->insert(declFunc);
                }
            }
            else if (dynamic_cast<TypeDecStatementNode*>(decl.second) != nullptr)
            {
                auto var = dynamic_cast<TypeDecStatementNode*>(decl.second);
                for (auto &method : *var->methods)
                {
                    method->block = nullptr;
                }
                currentScope->insert(var);
            }
            else if (dynamic_cast<ExternFuncDecStatementNode*>(decl.second) != nullptr)
            {
                auto func = dynamic_cast<ExternFuncDecStatementNode*>(decl.second);
                if (!func->isPrivate)
                {
                    auto declFunc = new ExternFuncDecStatementNode(*func);
                    declFunc->block = nullptr;
                    currentScope->insert(declFunc);
                }
            }
            else if (dynamic_cast<VarDecStatementNode*>(decl.second) != nullptr)
            {
                auto var = dynamic_cast<VarDecStatementNode*>(decl.second);
                if (!var->isPrivate)
                {
                    auto declVar = new VarDecStatementNode(*var);
                    //declVar->expr = nullptr;
                    currentScope->insert(declVar);
                }
            }
            else if (dynamic_cast<ArrayDecStatementNode*>(decl.second) != nullptr)
            {
                auto var = dynamic_cast<ArrayDecStatementNode*>(decl.second);
                if (!var->isPrivate)
                {
                    auto declVar = new ArrayDecStatementNode(*var);
                    //declVar->expr = nullptr;
                    currentScope->insert(declVar);
                }
            }
            else if (dynamic_cast<FuncPointerStatementNode*>(decl.second) != nullptr)
            {
                auto var = dynamic_cast<FuncPointerStatementNode*>(decl.second);
                if (!var->isPrivate)
                {
                    auto declVar = new FuncPointerStatementNode(*var);
                    currentScope->insert(declVar);
                }
            }
        }
    }
    parseModuleDecl();
    mainModuleNode->block = parseBlock(mainModuleNode->name);
}

bool Parser::parseImports()
{
    while (token.type == TokenType::Import)
    {
        consume(TokenType::Import);
        if (!expect(TokenType::Identifier)) return false;
        std::string moduleName = token.data;
        advance();

        Lexer lexer(moduleName + ".sl");
        lexer.tokenize();
        Parser* parser = new Parser(lexer.tokens);
        parser->parse();
        if (parser->hasError)
        {
            llvm::errs() << "[ERROR] Compilation failed.\n";
            exit(-1);
        }
        CodeGenContext codeGenContext(parser->mainModuleNode, false, parser->currentScope->symbols);
        codeGenContext.generateCode(parser->mainModuleNode);
        if (DEBUG) codeGenContext.mModule->print(llvm::dbgs(), nullptr);
        std::error_code EC;
        auto outFileStream = llvm::raw_fd_ostream(moduleName + ".ll", EC, sys::fs::OF_None);
        codeGenContext.mModule->print(outFileStream, nullptr);

        importedModules->push_back(parser);

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
        //else if (token.type == TokenType::Variable)  ret = parseVariableDecl(true);
        else return false;
        advance();
    }

    return ret;
}

BlockExprNode* Parser::parseBlock(VariableExprNode *name) {
    Scope *scope = new Scope(currentScope);
    currentScope = scope;
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
        else if (token.type == TokenType::Call) statements->push_back(parseCall());
        else if (token.type == TokenType::Delete) statements->push_back(parseDelete());
        if (token.type == TokenType::End)
        {
            advance();
            std::string endName = token.data;
            advance();
            if (endName == name->value)
                blockEnd = true;
            else
            {
                llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Expected end of block \"" << name->value << "\", not \"" << endName << "\".\n";
                hasError = true;
                return block;
            }
        }
    }
    currentScope = scope->parent;
    return block;
}

bool Parser::parseVisibilityOperator() {
    advance();
    //bool ret = false;
    if (token.type == TokenType::Function) parseFunctionDecl();
    else if (token.type == TokenType::Procedure) parseFunctionDecl();
    else if (token.type == TokenType::Extern && (tokensIterator + 1)->type == TokenType::Class) {tokensIterator = tokensIterator - 1; parseTypeDecl();}
    else if (token.type == TokenType::Extern && ((tokensIterator + 1)->type == TokenType::Procedure || (tokensIterator + 1)->type == TokenType::Function)) parseFunctionDecl();
    else if (token.type == TokenType::Extern && (tokensIterator + 1)->type == TokenType::Variable) parseVariableDecl(true);
    else if (token.type == TokenType::Variable) parseVariableDecl(true);
    else if (token.type == TokenType::Class) parseTypeDecl();
    return true;
}

bool Parser::parseStatement() {
    return false;
}

DeclarationNode* Parser::parseFieldDecl(std::vector<DeclarationNode *> *fields, std::string &thisClassName, bool &constructorRequired, int index) {
    DeclarationNode *field = nullptr;
    std::string name;
    std::string type;
    bool isPrivate = false;
    bool isArray = false;
    Token tok = consume(TokenType::VisibilityType);
    if (token.data == "private") isPrivate = true;
    consume(TokenType::Field);
    consume(TokenType::Minus);
    tok = consume(TokenType::Identifier);
    type = tok.data;
    if (type == "array") {
        ExprNode* expr;
        isArray = true;
        tokensIterator--;
        token = *tokensIterator;
        std::string typeOfArray;
        ExprNode *size;
        advance();
        consume(TokenType::LBracket);
        size = parseExpression();
        consume(TokenType::RBracket);
        std::string arrType = token.data;
        ArrayExprNode *arrExpr = new ArrayExprNode(arrType, size, new std::vector<ExprNode *>());
        int indicesCount = 1;
        while (token.data == "array")
        {
            indicesCount++;
            advance();
            consume(TokenType::LBracket);
            size = parseExpression();
            consume(TokenType::RBracket);
            if (token.type == TokenType::Identifier)
            {
                arrType = token.data;
                if (!oneOfDefaultTypes(arrType) && arrType != "array")
                {
                    advance();
                    type = parseTypeName(arrType);
                    auto typeStatement = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(type));
                    if (typeStatement == nullptr)
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Unknown type \"" << type << "\".\n";
                        hasError = true;
                    }
                    tokensIterator--;
                    token = *tokensIterator;
                }
            }
            arrExpr->values->push_back(new ArrayExprNode(arrType, size, nullptr));
        }
        expr = arrExpr;
        advance();
        name = consume(TokenType::Identifier).data;
        constructorRequired = true;
        field = new FieldArrayVarDecNode(thisClassName, new VariableExprNode(name), isPrivate, new ArrayDecStatementNode(nullptr, arrExpr, isPrivate, indicesCount), index);
    }
    else
    {
        if (type == thisClassName || currentScope->lookup(type) || oneOfDefaultTypes(type))
        {
            tok = consume(TokenType::Identifier);
            name = tok.data;
            if (!isFieldNameCorrect(fields, name))
            {
                llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber <<  ") Field \"" << name << "\" already declared in class \"" << thisClassName << "\".\n";
                hasError = true; // error field already exists
            }

            if (token.type == TokenType::Assign || token.type == TokenType::Semicolon)
            {
                bool init = token.type == TokenType::Assign;
                ExprNode* expr;
                if (init) {
                    advance();
                    expr = parseExpression();
                    field = new FieldVarDecNode(thisClassName, new VariableExprNode(name), isPrivate, type, expr, index);
                    constructorRequired = true;
                    if (DEBUG) llvm::outs() << "parsed field " << type << " " << name << "\n";
                }
                else
                {
                    TokenType dataType = init ? token.type : TokenType::Nil;
                    std::string data = init ? token.data : "";
                    if (oneOfDefaultTypes(type))
                    {
                        field = initDefaultType(thisClassName, isPrivate, name, type, dataType, data, index);
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

std::vector<FuncParamDecStatementNode*> *Parser::parseFuncParameters(bool named)
{
    auto* params = new std::vector<FuncParamDecStatementNode*>();
    consume(TokenType::LParen);
    while (token.type != TokenType::RParen)
    {
        std::string name;
        ExprNode* typeNode;
        ParameterType parameterType{};
        std::string ptype = consume(TokenType::Identifier).data;
        if (ptype == "in") parameterType = ParameterType::In;
        else if (ptype == "var") parameterType = ParameterType::Var;
        else if (ptype == "out") parameterType = ParameterType::Out;        // probably we dont need "out" type
        typeNode = lookupTypes(token.data);
        advance();
        if (named) {
            name = consume(TokenType::Identifier).data;
        }
        params->push_back(new FuncParamDecStatementNode(typeNode, new VariableExprNode(name), parameterType));
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
    auto scope = new Scope(currentScope);

    MethodDecNode *method = nullptr;
    bool isFunction = false;
    std::string name;
    ExprNode* type;
    std::string thisName;
    bool isPrivate = false;
    BlockExprNode *block = nullptr;
    Token tok = consume(TokenType::VisibilityType);
    if (token.data == "private") isPrivate = true;
    consume(TokenType::Method);
    tok = consume(TokenType::Identifier);
    auto shortName = tok.data;
    name = mainModuleNode->name->value + "." + thisClassName + "." + tok.data;
    consume(TokenType::LParen);
    tok = consume(TokenType::Identifier);
    if (tok.data != thisClassName)
    {
        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") \"this\" should be \"" << thisClassName + "\", not \"" << tok.data << "\".\n";
        hasError = true;
    }
    tok = consume(TokenType::Identifier);
    thisName = tok.data;
    scope->insert(new VarDecStatementNode(mainModuleNode->name->value + "." + thisClassName, new VariableExprNode(thisName)));
    currentScope = scope;
    consume(TokenType::RParen);
    auto params = new std::vector<FuncParamDecStatementNode *>();
    params = parseFuncParameters();
    params->insert(params->begin(), new FuncParamDecStatementNode(new VariableExprNode(mainModuleNode->name->value + "." + thisClassName), new VariableExprNode(thisName), ParameterType::Out));
    advance();
    if (token.type == TokenType::Colon)
    {
        advance();
        type = lookupTypes(token.data);
        isFunction = true;
    }
    else
    {
        type = new VariableExprNode("");
        isFunction = false;
    }

    // override
    for (auto it = methods->begin(); it != methods->end(); it++)
    {
        if ((*it)->name->value == name)
        {
            auto margs = (*it)->args;
            bool argsEqual = true;
            if (margs->size() == params->size())
            {
                int i = 0;
                for (auto argIt = margs->begin(); argIt != margs->end(); argIt++, i++)
                {
                    auto left = (*argIt)->type;
                    auto right = params->at(i)->type;
                    if (dynamic_cast<VariableExprNode*>(left) != nullptr && dynamic_cast<VariableExprNode*>(right) != nullptr)
                    {
                        if (dynamic_cast<VariableExprNode*>(left)->value != dynamic_cast<VariableExprNode*>(right)->value)
                        {
                            argsEqual = false;
                        }
                    }
                    else if (dynamic_cast<ArrayExprNode*>(left) != nullptr && dynamic_cast<ArrayExprNode*>(right) != nullptr)
                    {
                        auto larr = dynamic_cast<ArrayExprNode*>(left);
                        auto rarr = dynamic_cast<ArrayExprNode*>(right);
                        if (getArrayFinalType(larr) != getArrayFinalType(rarr))
                        {
                            argsEqual = false;
                        }
                    }
                }
            }
            else
            {
                argsEqual = false;
            }
            if (argsEqual)
            {
                delete *it;
                methods->erase(it);
                break;
            }
        }
    }

    // parse statements
    tokensIterator--;
    token = *tokensIterator;
    block = parseBlock(new VariableExprNode(shortName));

    if (token.type != TokenType::Semicolon)
    {
        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Unexpected token \"" << token.data + "\", expected \"" + Lexer::getTokenName(TokenType::Semicolon) + "\".\n";
        hasError = true;
    }
    currentScope = scope->parent;
    method = new MethodDecNode(type, new VariableExprNode(name), isPrivate, isFunction, new VariableExprNode(thisName), params, block);
    if (DEBUG) llvm::outs() << "parsed method of class " << thisClassName << " " << type << " " << name << "\n";
    return method;
}

bool Parser::parseTypeDecl() {
    TypeDecStatementNode *typeNode;
    auto *fields = new std::vector<DeclarationNode*>();
    auto *methods = new std::vector<MethodDecNode*>();
    std::string name;
    TypeDecStatementNode *parent;
    bool isPrivate = token.data == "private";
    bool isExtern = false;
    advance();
    if (token.type == TokenType::Extern)
    {
        advance();
        advance();
        isExtern = true;
    }
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
                if (decl == nullptr) decl = currentScope->lookup(mainModuleNode->name->value + "." + token.data);
                if (decl != nullptr)
                {
                    parent = dynamic_cast<TypeDecStatementNode*>(decl);
                    if (parent != nullptr)
                    {
                        if (parent->fields != nullptr)
                        {
                            fields->assign(parent->fields->begin(), parent->fields->end());
                        }
                        if (parent->methods != nullptr)
                        {
                            for (auto &method : *parent->methods)
                            {
                                auto newName = new VariableExprNode(mainModuleNode->name->value + "." + name + method->name->value.substr(method->name->value.rfind('.')));
                                auto args = new std::vector<FuncParamDecStatementNode*>();
                                if (method->args != nullptr)
                                {
                                    if (method->args != nullptr && !method->args->empty())
                                    {
                                        args->push_back(new FuncParamDecStatementNode(new VariableExprNode(mainModuleNode->name->value + "." + name),
                                                                                      new VariableExprNode(method->args->at(0)->name->value),
                                                                                      method->args->at(0)->parameterType,
                                                                                      method->args->at(0)->expr));
                                    }
                                }
                                auto newStatements = new std::vector<StatementNode*>();
                                if (method->name->value == method->name->value + "._DefaultConstructor_")
                                {
                                    for (auto &statement : *method->block->statements)
                                    {
                                        if (dynamic_cast<FieldVarDecNode*>(statement) != nullptr)
                                        {
                                            auto pFieldVar = dynamic_cast<FieldVarDecNode*>(statement);
                                            auto fieldVarDec = new FieldVarDecNode(name, new VariableExprNode(pFieldVar->name->value), pFieldVar->isPrivate, pFieldVar->type, pFieldVar->expr, pFieldVar->index);
                                            newStatements->push_back(fieldVarDec);
                                        }
                                        else if (dynamic_cast<FieldArrayVarDecNode*>(statement) != nullptr)
                                        {
                                            auto pFieldArrVar = dynamic_cast<FieldArrayVarDecNode*>(statement);
                                            auto fieldArrVarDec = new FieldArrayVarDecNode(name, new VariableExprNode(pFieldArrVar->name->value), pFieldArrVar->isPrivate, pFieldArrVar->var, pFieldArrVar->index);
                                            newStatements->push_back(fieldArrVarDec);
                                        }
                                    }
                                }
                                auto methodDecNode = new MethodDecNode(method->type, newName, method->isPrivate, method->isFunction, new VariableExprNode(method->thisName->value), args, new BlockExprNode(newStatements));
                                methods->push_back(methodDecNode);
                            }
                        }
                    }
                    // insert before methods parsing
                    typeNode = new TypeDecStatementNode(new VariableExprNode(mainModuleNode->name->value + "." + name), isPrivate, fields, methods, isExtern, parent);
                    currentScope->insert(typeNode);

                    Scope *scope = new Scope(currentScope);
                    currentScope = scope;
                    int fieldIndex = 0;
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
                                auto field = parseFieldDecl(fields, name, constructorRequired, fieldIndex);
                                if (field != nullptr)
                                {
                                    // TODO Check if field value already exists in fields
                                    fields->push_back(field);
                                    fieldIndex++;
                                    //if (DEBUG) llvm::outs() << "parsed field " << field->value->value << " " << field->isPrivate << "\n";
                                }
                                else
                                {
                                    llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Failed to parse field of class \"" << name << "\".\n";
                                    hasError = true;
                                }
                            }
                            if (token.type == TokenType::Method)
                            {
                                tokensIterator--;
                                token = *tokensIterator;
                                auto method = parseMethodDecl(methods, name);
                                if (method != nullptr)
                                {
                                    // TODO Check if method value already exists in methods
                                    methods->push_back(method);
                                }
                            }
                        }
                    }

                    if (constructorRequired)
                    {
                        std::string constructorName = name + "._DefaultConstructor_";
                        std::string constructorType;
                        bool constructorIsPrivate = false;
                        auto *constructorStatements = new std::vector<StatementNode*>();
                        //constructorStatements->push_back(new AssignExprNode());
                        for (auto field : *fields)
                        {
                            if (dynamic_cast<FieldVarDecNode*>(field) != nullptr)
                            {
                                constructorStatements->push_back(field);
                            }
                            else if (dynamic_cast<FieldArrayVarDecNode*>(field) != nullptr)
                            {
                                constructorStatements->push_back(field);
                            }
                        }
                        auto constructor = new MethodDecNode(new VariableExprNode(constructorType), new VariableExprNode(mainModuleNode->name->value + "." + constructorName), constructorIsPrivate, false, new VariableExprNode(name),
                                                   nullptr, new BlockExprNode(constructorStatements));
                        methods->push_back(constructor);
                    }

                    advance();
                    if (token.type == TokenType::Identifier && token.data == name && advance().type == TokenType::Semicolon)
                    {
                        currentScope = scope->parent;
                        typeNode = new TypeDecStatementNode(new VariableExprNode(mainModuleNode->name->value + "." + name), isPrivate, fields, methods, isExtern, parent);
                        // already inserted
                        //currentScope->insert(typeNode);
                    }
                }
                else
                {
                    llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Type " + token.data + " is not declared.\n";
                    hasError = true;
                }
            }
        }
    }
    else
    {
        return false;
    }
    if (isExtern) {
        typeNode->name->value = name;
        typeNode->methods->clear();
        typeNode->fields->clear();
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
            result = new OperatorExprNode(result, TokenType::And, parseEquality());
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
    if (tok.type == TokenType::Nil)
        return new NilExprNode();
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
    else if (tok.type == TokenType::Float) return new FloatExprNode(std::stof(tok.data));
    else if (tok.type == TokenType::LParen)
    {
        auto result = parseExpression();
        consume(TokenType::RParen);
        return result;
    }
    llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Failed to parse expression.\n";
    hasError = true;
    return nullptr;
}

// TODO: rewrite all this hell
StatementNode* Parser::parseVarOrCall() {
    tokensIterator--;
    token = *tokensIterator;
    Token tok = token;
    std::string name = tok.data;
    advance();

    TypeDecStatementNode* type;
    bool dotModule = false;
    bool dotClass = false;
    bool isIndex = false;

    bool fieldExists = false;
    bool methodExists = false;

    ExprNode* varExpr = nullptr;

    int fieldIndex = -1;
    std::string methodName;

    FieldArrayVarDecNode *fieldArray = nullptr;
    // if we have dot
    if (token.type == TokenType::Dot)
    {
        // is it module or class value
        Parser *moduleP = nullptr;
        for (auto &module : *importedModules)
        {
            if (module->mainModuleNode->name->value == name)
            {
                dotModule = true;
                moduleP = module;
                break;
            }
        }
        if (!dotModule)
        {
            type = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(name));
            if (type != nullptr) dotClass = true;
        }
        if (!dotModule && !dotClass)
        {
            auto var = currentScope->lookup(name);
            if (var == nullptr)
            {
                llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") " + name + " is not declared.\n";
                hasError = true;
            }
            type = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(dynamic_cast<VarDecStatementNode*>(var)->type));
            if (type != nullptr) dotClass = true;
        }
        advance();
        expect(TokenType::Identifier);
        if (dotModule)
        {
            if (moduleP->currentScope->lookup(name + "." + token.data) != nullptr)
                name += "." + token.data;
            else
            {
                llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") " + name + "." + token.data + " is not declared.\n";
                hasError = true;
            }
        }
        if (dotClass)
        {
            for (auto &field : *type->fields)
            {
                if (field->name->value == token.data)
                {
                    name += "." + token.data;
                    fieldExists = true;
                    if (dynamic_cast<FieldVarDecNode*>(field) != nullptr) fieldIndex = dynamic_cast<FieldVarDecNode*>(field)->index;
                    else if (dynamic_cast<FieldArrayVarDecNode*>(field)) {
                        fieldIndex = dynamic_cast<FieldArrayVarDecNode*>(field)->index;
                        fieldArray = dynamic_cast<FieldArrayVarDecNode*>(field);
                    }
                    break;
                }
            }

            for (auto &method : *type->methods)
            {
                if (method->name->value == type->name->value + "." + token.data)
                {
                    //name = type->name->value + "." + token.data;
                    methodName = type->name->value + "." + token.data;
                    methodExists = true;
                    break;
                }
            }
        }
        advance();
    }
    // if we have an array
    auto indexes = new std::vector<ExprNode*>();
    if (token.type == TokenType::LBracket)
    {
        if (dotClass && !fieldExists) {
            llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Field " + name + "." + token.data + " is not declared in " + type->name->value + " type.\n";
            hasError = true;
        }
        isIndex = true;
        ExprNode* index = nullptr;
        while (token.type == TokenType::LBracket)
        {
            advance();
            index = parseExpression();
            indexes->push_back(index);
            expect(TokenType::RBracket);
            advance();
        }
        if (token.type == TokenType::Dot)
        {
            advance();
            //TODO: call methods of array object
            //auto var = currentScope->lookup(name);
            //if (var == nullptr)
            //{
                //llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") " + name + " is not declared.\n";
                //hasError = true;
            //}

            if (fieldArray != nullptr) {
                type = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(getArrayFinalType(fieldArray->var->expr)));
            }
            else {
                auto var = currentScope->lookup(name);
                if (var == nullptr)
                {
                    llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") " + name + " is not declared.\n";
                    hasError = true;
                }
                type = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(getArrayFinalType(dynamic_cast<ArrayDecStatementNode *>(var)->expr)));
            }
            if (type != nullptr) dotClass = true;
            if (dotClass)
            {
                for (auto &field : *type->fields)
                {
                    if (field->name->value == token.data)
                    {
                        name += "." + token.data;
                        fieldExists = true;
                        if (dynamic_cast<FieldVarDecNode*>(field) != nullptr) fieldIndex = dynamic_cast<FieldVarDecNode*>(field)->index;
                        else if (dynamic_cast<FieldArrayVarDecNode*>(field)) fieldIndex = dynamic_cast<FieldArrayVarDecNode*>(field)->index;
                        break;
                    }
                }

                for (auto &method : *type->methods)
                {
                    if (method->name->value == type->name->value + "." + token.data)
                    {
                        methodName = type->name->value + "." + token.data;
                        methodExists = true;
                        break;
                    }
                }
            }
            advance();
            //varExpr = new IndexesExprNode(name, indexes, dotModule, dotClass, fieldIndex, true);
            if (indexes->size() == 1)
                varExpr = new IndexExprNode(name, indexes->at(0), dotModule, dotClass, fieldIndex, true);
            else
                varExpr = new IndexesExprNode(name, indexes, dotModule, dotClass, fieldIndex, true);
            //varExpr = new IndexExprNode(name, index, dotModule, dotClass, fieldIndex, true);
        }
        else
        {
            //varExpr = new IndexExprNode(name, index, dotModule, dotClass, fieldIndex, true);
            //return new IndexesExprNode(name, indexes, dotModule, dotClass, fieldIndex);
            if (indexes->size() == 1)
                return new IndexExprNode(name, indexes->at(0), dotModule, dotClass, fieldIndex);
            else
                return new IndexesExprNode(name, indexes, dotModule, dotClass, fieldIndex);

        }
    }
    if (token.type != TokenType::LParen) {
        return new VariableExprNode(name, E_UNKNOWN, dotModule, dotClass, fieldIndex);
    }
    if (!dotModule && currentScope->lookup(mainModuleNode->name->value + "." + name) != nullptr)
        name = mainModuleNode->name->value + "." + name;
    //else
    //llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") " + mainModuleNode->value->value + "." + value + " is not declared.\n";
    if (dotClass && !methodExists) {
        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Method " + name + "." + token.data + " is not declared in " + type->name->value + " type.\n";
        hasError = true;
    }
    consume(TokenType::LParen);
    auto params = new std::vector<ExprNode*>();
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
    auto funcDecl = currentScope->lookup(name);

    if (dynamic_cast<FuncDecStatementNode*>(funcDecl) != nullptr) {
        funcDecl = dynamic_cast<FuncDecStatementNode*>(funcDecl);
    }
    else if (dynamic_cast<ExternFuncDecStatementNode*>(funcDecl) != nullptr) {
        funcDecl = dynamic_cast<ExternFuncDecStatementNode*>(funcDecl);
    }
    else if (dynamic_cast<FuncPointerStatementNode*>(funcDecl) != nullptr) {
        funcDecl = dynamic_cast<FuncPointerStatementNode*>(funcDecl);
    }
    if (dynamic_cast<FuncDecStatementNode*>(funcDecl) != nullptr)
    {
        if (params->size() == dynamic_cast<FuncDecStatementNode*>(funcDecl)->args->size()) {
            int i = 0;
            while (i < params->size()) {
                if (dynamic_cast<NilExprNode*>(params->at(i)) != nullptr) {
                    dynamic_cast<NilExprNode*>(params->at(i))->type = dynamic_cast<ExternFuncDecStatementNode*>(funcDecl)->args->at(i)->type;
                }
                i++;
            }
        }
    }
    else if (dynamic_cast<ExternFuncDecStatementNode*>(funcDecl) != nullptr)
    {
        if (params->size() == dynamic_cast<ExternFuncDecStatementNode*>(funcDecl)->args->size()) {
            int i = 0;
            while (i < params->size()) {
                if (dynamic_cast<NilExprNode*>(params->at(i)) != nullptr) {
                    dynamic_cast<NilExprNode*>(params->at(i))->type = dynamic_cast<ExternFuncDecStatementNode*>(funcDecl)->args->at(i)->type;
                }
                i++;
            }
        }
    }
    else if (dynamic_cast<FuncPointerStatementNode*>(funcDecl) != nullptr)
    {
        if (params->size() == dynamic_cast<FuncPointerStatementNode*>(funcDecl)->args->size()) {
            int i = 0;
            while (i < params->size()) {
                if (dynamic_cast<NilExprNode*>(params->at(i)) != nullptr) {
                    dynamic_cast<NilExprNode*>(params->at(i))->type = dynamic_cast<FuncPointerStatementNode*>(funcDecl)->args->at(i)->type;
                }
                i++;
            }
        }
    }
    advance();
    if (dotClass)
    {
        if (isIndex)
        {
            params->insert(params->begin(), varExpr);
        }
        else
        {
            params->insert(params->begin(), new VariableExprNode(name, E_UNKNOWN, dotModule, dotClass));
        }
        return new CallExprNode(new VariableExprNode(methodName), params);
    }
    else
    {

        return new CallExprNode(new VariableExprNode(name, E_UNKNOWN, dotModule, dotClass), params);
    }
    return nullptr;
}

DeclarationNode* Parser::parseVariableDecl(bool isGlobal) {
    std::string type;
    std::string name;
    bool isPrivate = false;
    bool isExtern = false;
    if (isGlobal)
    {
        tokensIterator--;
        token = *tokensIterator;
        if (token.data == "private") isPrivate = true;
        advance();

        if (token.type == TokenType::Extern)
        {
            isExtern = true;
            advance();
        }
    }
    advance();
    consume(TokenType::Minus);
    ExprNode *expr = nullptr;
    type = token.data;
    advance();
    bool isArray = false;
    int indicesCount = 0;

    bool isFuncPointer = false;
    bool isFunction = type == "function";
    std::vector<FuncParamDecStatementNode *> *args = nullptr;
    ExprNode* funcType;

    DeclarationNode* result;
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
    else if (type == "array")
    {
        indicesCount++;
        isArray = true;
        tokensIterator--;
        token = *tokensIterator;
        std::string typeOfArray;
        ExprNode* size;
        advance();
        consume(TokenType::LBracket);
        size = parseExpression();
        consume(TokenType::RBracket);
        std::string arrType = token.data;
        if (!oneOfDefaultTypes(arrType) && arrType != "array")
        {
            advance();
            arrType = parseTypeName(arrType);
            tokensIterator--;
            token = *tokensIterator;
        }
        ArrayExprNode* arrExpr = new ArrayExprNode(arrType, size, new std::vector<ExprNode*>());
        while (token.data == "array")
        {
            indicesCount++;
            advance();
            consume(TokenType::LBracket);
            size = parseExpression();
            consume(TokenType::RBracket);
            if (token.type == TokenType::Identifier)
            {
                arrType = token.data;
                if (!oneOfDefaultTypes(arrType) && arrType != "array")
                {
                    advance();
                    type = parseTypeName(arrType);
                    auto typeStatement = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(type));
                    if (typeStatement == nullptr)
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Unknown type \"" << type << "\".\n";
                        hasError = true;
                    }
                    tokensIterator--;
                    token = *tokensIterator;
                }
            }
            arrExpr->values->push_back(new ArrayExprNode(arrType, size, nullptr));
        }
        expr = arrExpr;
        advance();
        name = consume(TokenType::Identifier).data;
    }
    // variable-function(in integer, in integer): integer funcPointer;
    else if (type == "function" || type == "procedure")
    {
        isFuncPointer = true;
        isFunction = type == "function";
        args = parseFuncParameters(false);

        if (isFunction)
        {
            advance();
            if (token.type == TokenType::Colon)
            {
                advance();
                funcType = lookupTypes(token.data);
            }
            else
            {
                funcType = new VariableExprNode("");
            }
        }
        else funcType = new VariableExprNode("");
        advance();
        name = consume(TokenType::Identifier).data;
    }
    else
    {
        type = parseTypeName(type);
        name = consume(TokenType::Identifier).data;
    }
    if (currentScope->lookup(name) != nullptr || oneOfDefaultTypes(name))
    {
        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Name conflict - \"" << name << "\".\n";
        hasError = true;
    }
    if (isGlobal && !isExtern) name = mainModuleNode->name->value + "." + name;
    if (isArray) {
        result = new ArrayDecStatementNode(new VariableExprNode(name), dynamic_cast<ArrayExprNode*>(expr), isGlobal, indicesCount, isPrivate, isExtern);
    }
    else if (isFuncPointer) {
        result = new FuncPointerStatementNode(funcType, new VariableExprNode(name), isFunction, isGlobal, args, isPrivate, isExtern);
    }
    else {
        result = new VarDecStatementNode(type, new VariableExprNode(name), expr, isGlobal, isPrivate, isExtern);
    }
    if (isGlobal) currentScope->insert(result);
    currentScope->insert(result);
    return result;
}

DeclarationNode *Parser::parseFunctionDecl() {
    tokensIterator--;
    token = *tokensIterator;
    bool isPrivate = false;
    if (token.data == "private") isPrivate = true;
    bool isExtern = false;
    advance();
    if (token.type == TokenType::Extern) {
        isExtern = true;
        advance();
    }
    bool isFunction = false;
    if (match(TokenType::Function)) isFunction = true;
    else {
        consume(TokenType::Procedure);
    }
    std::string name = consume(TokenType::Identifier).data;
    ExprNode* type;
    auto params = parseFuncParameters();
    if (isFunction)
    {
        advance();
        if (token.type == TokenType::Colon)
        {
            advance();
            type = lookupTypes(token.data);
        }
        else
        {
            type = new VariableExprNode("");
        }
    }
    else type = new VariableExprNode("");
    VariableExprNode *funcName = nullptr;
    if (isExtern) {
        funcName = new VariableExprNode(name);
    } else {
        funcName = new VariableExprNode(mainModuleNode->name->value + "." + name);
    }
    DeclarationNode* function = nullptr;
    BlockExprNode* block = nullptr;
    if (isExtern)
    {
        function = new ExternFuncDecStatementNode(type, funcName, isPrivate, isFunction, params, block);
    } else {
        function = new FuncDecStatementNode(type, funcName, isPrivate, isFunction, params, block);
    }

    currentScope->insert(function);
    block = parseBlock(new VariableExprNode(name));
    if (dynamic_cast<ExternFuncDecStatementNode*>(function)) dynamic_cast<ExternFuncDecStatementNode*>(function)->block = block;
    else if (dynamic_cast<FuncDecStatementNode*>(function)) dynamic_cast<FuncDecStatementNode*>(function)->block = block;

    if (token.type != TokenType::Semicolon)
    {
        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Unexpected token \"" << token.data + "\", expected \"" + Lexer::getTokenName(TokenType::Semicolon) + "\".\n";
        hasError = true;
    }

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
            else if (token.type == TokenType::While) statements->push_back(parseWhileStatement());
            else if (token.type == TokenType::Call) statements->push_back(parseCall());
            else if (token.type == TokenType::Delete) statements->push_back(parseDelete());
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
        else if (token.type == TokenType::While) statements->push_back(parseWhileStatement());
        else if (token.type == TokenType::Call) statements->push_back(parseCall());
        else if (token.type == TokenType::Delete) statements->push_back(parseDelete());
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
        else if (token.type == TokenType::Call) statements->push_back(parseCall());
        else if (token.type == TokenType::Delete) statements->push_back(parseDelete());
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
    /*if (dynamic_cast<VariableExprNode *>(expr) != nullptr)
    {
        return new OutputStatementNode(dynamic_cast<VariableExprNode *>(expr));
    }
    else if (dynamic_cast<IndexExprNode *>(expr) != nullptr)
    {
        return new OutputStatementNode(dynamic_cast<IndexExprNode *>(expr));
    }*/
    return new OutputStatementNode(expr);
}

AssignExprNode *Parser::parseAssignStatement() {
    consume(TokenType::Let);
    advance();
    auto varName = parseVarOrCall();
    // TODO: ???
    //tokensIterator--;
    //token = *tokensIterator;
    consume(TokenType::Assign);
    auto expr = parseExpression();
    if (dynamic_cast<VariableExprNode *>(varName) != nullptr)
    {
        return new AssignExprNode(dynamic_cast<VariableExprNode *>(varName), expr);
    }
    else if (dynamic_cast<IndexExprNode *>(varName) != nullptr)
    {
        return new AssignExprNode(dynamic_cast<IndexExprNode *>(varName), expr);
    }
    return nullptr;
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

CallExprNode *Parser::parseCall() {
    consume(TokenType::Call);
    advance();
    auto callExpr = parseVarOrCall();
    if (dynamic_cast<CallExprNode *>(callExpr) == nullptr)
    {
        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") " << dynamic_cast<VariableExprNode *>(callExpr)->value << " is not a function.\n";
        hasError = true;
    }
    return dynamic_cast<CallExprNode *>(callExpr);
}

DeleteExprNode *Parser::parseDelete() {
    consume(TokenType::Delete);
    auto varExpr = parseExpression();
    return new DeleteExprNode(varExpr);
}

ExternFuncDecStatementNode *Parser::parseExternFuncDecl() {
    return nullptr;
}
