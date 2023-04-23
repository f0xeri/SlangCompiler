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
            if (decl.second->isA<FuncDecStatementNode>())
            {
                auto func = decl.second->as<FuncDecStatementNode>();
                if (!func->isPrivate)
                {
                    auto declFunc = new FuncDecStatementNode(*func);
                    declFunc->block = nullptr;
                    currentScope->insert(declFunc);
                }
            }
            else if (decl.second->isA<TypeDecStatementNode>())
            {
                auto var = decl.second->as<TypeDecStatementNode>();
                for (auto &method : *var->methods)
                {
                    method->block = nullptr;
                }
                currentScope->insert(var);
            }
            else if (decl.second->isA<ExternFuncDecStatementNode>())
            {
                auto func = decl.second->as<ExternFuncDecStatementNode>();
                if (!func->isPrivate)
                {
                    auto declFunc = new ExternFuncDecStatementNode(*func);
                    declFunc->block = nullptr;
                    currentScope->insert(declFunc);
                }
            }
            else if (decl.second->isA<VarDecStatementNode>())
            {
                auto var = decl.second->as<VarDecStatementNode>();
                if (!var->isPrivate)
                {
                    auto declVar = new VarDecStatementNode(*var);
                    //declVar->expr = nullptr;
                    currentScope->insert(declVar);
                }
            }
            else if (decl.second->isA<ArrayDecStatementNode>())
            {
                auto var = decl.second->as<ArrayDecStatementNode>();
                if (!var->isPrivate)
                {
                    auto declVar = new ArrayDecStatementNode(*var);
                    //declVar->expr = nullptr;
                    currentScope->insert(declVar);
                }
            }
            else if (decl.second->isA<FuncPointerStatementNode>())
            {
                auto var = decl.second->as<FuncPointerStatementNode>();
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
        //std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(),[](unsigned char c){ return std::tolower(c); });
        bool needToCompile = true;
        if (std::find(globalImportedModuleNames.begin(), globalImportedModuleNames.end(), moduleName) != globalImportedModuleNames.end())
        {
            // we already compiled this module, so we don't need to do it again
            needToCompile = false;
        }
        advance();

        if (needToCompile)
        {
            //llvm::errs() << "[INFO] Compiling module \"" << moduleName << "\"...\n";
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
            globalImportedModules.push_back(parser);
            globalImportedModuleNames.push_back(moduleName);
        }
        else
        {
            for (auto &module : globalImportedModules)
            {
                if (module->mainModuleNode->name->value == moduleName)
                {
                    importedModules->push_back(module);
                    break;
                }
            }
        }

        advance();
    }
    return true;
}

bool Parser::parseModuleDecl()
{
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    bool ret = false;
    consume(TokenType::Module);
    if (!expect(TokenType::Identifier)) return false;
    mainModuleNode = new ModuleStatementNode(loc, new VariableExprNode(loc, token.data), new BlockExprNode(loc));
    advance();
    while (token.type != TokenType::Start)
    {
        if (token.type == TokenType::VisibilityType) ret = parseVisibilityOperator();
        //else if (token.type == TokenType::Variable)  ret = parseVariableDecl(true);
        else return false;
        advance();
    }
    llvm::outs() << "[INFO] Compiling module \"" << mainModuleNode->name->value << "\"...\n";
    return ret;
}

BlockExprNode* Parser::parseBlock(VariableExprNode *name, std::vector<FuncParamDecStatementNode*> *argsToInsert) {
    Scope *scope = new Scope(currentScope);
    currentScope = scope;
    if (argsToInsert != nullptr) {
        for (auto &arg : *argsToInsert) {
            currentScope->insert(arg);
        }
    }
    bool blockEnd = false;
    auto *statements = new std::vector<StatementNode*>();
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    auto *block = new BlockExprNode(loc, statements);

    while (!blockEnd)
    {
        advance();
        if (token.type == TokenType::If) statements->push_back(parseIfStatement());
        else if (token.type == TokenType::Variable) statements->push_back(parseVariableDecl());
        else if (token.type == TokenType::Output) statements->push_back(parseOutputStatement());
        else if (token.type == TokenType::Input) statements->push_back(parseInputStatement());
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
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    Token tok = consume(TokenType::VisibilityType);
    if (tok.data == "private") isPrivate = true;
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
        ArrayExprNode *arrExpr = new ArrayExprNode(loc, arrType, size, new std::vector<ExprNode *>());
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
                    auto typeStatement = (currentScope->lookup(type))->as<TypeDecStatementNode>();
                    if (typeStatement == nullptr)
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Unknown type \"" << type << "\".\n";
                        hasError = true;
                    }
                    tokensIterator--;
                    token = *tokensIterator;
                }
            }
            arrExpr->values->push_back(new ArrayExprNode(loc, arrType, size, nullptr));
        }
        expr = arrExpr;
        advance();
        name = consume(TokenType::Identifier).data;
        constructorRequired = true;
        field = new FieldArrayVarDecNode(loc, thisClassName, new VariableExprNode(loc, name), isPrivate, new ArrayDecStatementNode(loc, nullptr, arrExpr, isPrivate, indicesCount), index);
    }
    else
    {
        if (type == thisClassName || currentScope->lookup(type) || currentScope->lookup(mainModuleNode->name->value + "." + type) || oneOfDefaultTypes(type))
        {
            tok = consume(TokenType::Identifier);
            name = tok.data;
            if (!isFieldNameCorrect(fields, name))
            {
                llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber <<  ") Field \"" << name << "\" already declared in class \"" << thisClassName << "\".\n";
                hasError = true; // error field already exists
            }

            if (!oneOfDefaultTypes(type)) constructorRequired = true;

            if (token.type == TokenType::Assign || token.type == TokenType::Semicolon)
            {
                bool init = token.type == TokenType::Assign;
                ExprNode* expr;
                if (init) {
                    advance();
                    expr = parseExpression();
                    field = new FieldVarDecNode(loc, thisClassName, new VariableExprNode(loc, name), isPrivate, type, expr, index);
                    constructorRequired = true;
                    if (DEBUG) llvm::outs() << "parsed field " << type << " " << name << "\n";
                }
                else
                {
                    TokenType dataType = init ? token.type : TokenType::Nil;
                    std::string data = init ? token.data : "";
                    if (oneOfDefaultTypes(type))
                    {
                        field = initDefaultType(loc, thisClassName, isPrivate, name, type, dataType, data, index);
                        if (DEBUG) llvm::outs() << "parsed field " << type << " " << name << " - " << data << "\n";
                    }
                    else
                    {
                        // TODO: object fields
                        if (!currentScope->lookup(type) && currentScope->lookup(mainModuleNode->name->value + "." + type)) {
                            type = mainModuleNode->name->value + "." + type;
                        }
                        field = new FieldVarDecNode(loc, thisClassName, new VariableExprNode(loc, name), isPrivate, type, nullptr, index);
                        if (DEBUG) llvm::outs() << "parsed field " << type << " " << name << "\n";
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
        SourceLoc loc{token.stringNumber, token.symbolNumber};
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
        auto funcParam = new FuncParamDecStatementNode(loc, typeNode, new VariableExprNode(loc, name), parameterType);
        params->push_back(funcParam);
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
    SourceLoc methodLoc {token.stringNumber, token.symbolNumber};
    MethodDecNode *method = nullptr;
    bool isFunction = false;
    std::string name;
    ExprNode* type;
    std::string thisName;
    bool isPrivate = false;
    BlockExprNode *block = nullptr;
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    Token tok = consume(TokenType::VisibilityType);
    if (tok.data == "private") isPrivate = true;
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
    loc = {token.stringNumber, token.symbolNumber};
    tok = consume(TokenType::Identifier);
    thisName = tok.data;
    scope->insert(new VarDecStatementNode(loc, mainModuleNode->name->value + "." + thisClassName, new VariableExprNode(loc, thisName)));
    currentScope = scope;
    consume(TokenType::RParen);
    auto params = new std::vector<FuncParamDecStatementNode *>();
    params = parseFuncParameters();
    params->insert(params->begin(), new FuncParamDecStatementNode(loc, new VariableExprNode(loc, mainModuleNode->name->value + "." + thisClassName), new VariableExprNode(loc, thisName), ParameterType::Out));
    advance();
    if (token.type == TokenType::Colon)
    {
        advance();
        auto typeStr = parseTypeName(token.data);
        type = lookupTypes(typeStr);
        isFunction = true;
    }
    else
    {
        type = new VariableExprNode({}, "");
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
                    if (left->isA<VariableExprNode>() && right->isA<VariableExprNode>())
                    {
                        if (left->as<VariableExprNode>()->value != right->as<VariableExprNode>()->value)
                        {
                            argsEqual = false;
                        }
                    }
                    else if (left->isA<ArrayExprNode>() && right->isA<ArrayExprNode>())
                    {
                        auto larr = left->as<ArrayExprNode>();
                        auto rarr = right->as<ArrayExprNode>();
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
    loc = {token.stringNumber, token.symbolNumber};
    block = parseBlock(new VariableExprNode(loc, shortName));

    if (token.type != TokenType::Semicolon)
    {
        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Unexpected token \"" << token.data + "\", expected \"" + Lexer::getTokenName(TokenType::Semicolon) + "\".\n";
        hasError = true;
    }
    currentScope = scope->parent;
    method = new MethodDecNode(methodLoc, type, new VariableExprNode(methodLoc, name), isPrivate, isFunction, new VariableExprNode(methodLoc, thisName), params, block);
    if (DEBUG) llvm::outs() << "parsed method of class " << thisClassName << " " << type << " " << name << "\n";
    return method;
}

bool Parser::parseTypeDecl() {
    tokensIterator--;
    token = *tokensIterator;
    TypeDecStatementNode *typeNode;
    SourceLoc typeLoc{token.stringNumber, token.symbolNumber};
    auto *fields = new std::vector<DeclarationNode*>();
    auto *methods = new std::vector<MethodDecNode*>();
    std::string name;
    TypeDecStatementNode *parent;
    bool isPrivate = token.data == "private";
    advance();
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
        currentParsingTypeName = mainModuleNode->name->value + "." + name;
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
                    parent = decl->as<TypeDecStatementNode>();
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
                                auto newName = new VariableExprNode(method->loc, mainModuleNode->name->value + "." + name + method->name->value.substr(method->name->value.rfind('.')));
                                auto args = new std::vector<FuncParamDecStatementNode*>();
                                if (method->args != nullptr)
                                {
                                    // TODO: why we are pushing only 0 arg
                                    if (method->args != nullptr && !method->args->empty())
                                    {
                                        // first arg (this) should have type of current class, not parent
                                        args->push_back(new FuncParamDecStatementNode(method->args->at(0)->loc, new VariableExprNode(method->args->at(0)->loc, mainModuleNode->name->value + "." + name),
                                                                                      new VariableExprNode(method->args->at(0)->loc, method->args->at(0)->name->value),
                                                                                      method->args->at(0)->parameterType,
                                                                                      method->args->at(0)->expr));

                                        // just copy other args (if exists)
                                        if (method->args->size() > 1) {
                                            for (int i = 1; i < method->args->size(); i++) {
                                                auto arg = method->args->at(i);
                                                args->push_back(new FuncParamDecStatementNode(arg->loc, arg->type,
                                                                                              new VariableExprNode(arg->loc, arg->name->value),
                                                                                              arg->parameterType,
                                                                                              arg->expr));
                                            }
                                        }
                                    }
                                }
                                auto newStatements = new std::vector<StatementNode*>();
                                if (method->name->value == parent->name->value + "._DefaultConstructor_")
                                {
                                    for (auto &statement : *method->block->statements)
                                    {
                                        if (statement->isA<FieldVarDecNode>())
                                        {
                                            auto pFieldVar = statement->as<FieldVarDecNode>();
                                            auto fieldVarDec = new FieldVarDecNode(statement->loc, name, new VariableExprNode(statement->loc, pFieldVar->name->value), pFieldVar->isPrivate, pFieldVar->type, pFieldVar->expr, pFieldVar->index);
                                            newStatements->push_back(fieldVarDec);
                                        }
                                        else if (statement->isA<FieldArrayVarDecNode>())
                                        {
                                            auto pFieldArrVar = statement->as<FieldArrayVarDecNode>();
                                            auto fieldArrVarDec = new FieldArrayVarDecNode(statement->loc, name, new VariableExprNode(statement->loc, pFieldArrVar->name->value), pFieldArrVar->isPrivate, pFieldArrVar->var, pFieldArrVar->index);
                                            newStatements->push_back(fieldArrVarDec);
                                        }
                                    }
                                }
                                else if (method->name->value == parent->name->value + ".toString") {
                                    newStatements->push_back(new ReturnStatementNode(method->loc, new StringExprNode(method->loc, name)));
                                }
                                else {
                                    if (method->block != nullptr) {
                                        for (auto &statement : *method->block->statements) {
                                            newStatements->push_back(statement);
                                        }
                                    }
                                }

                                auto methodDecNode = new MethodDecNode(method->loc, method->type, newName, method->isPrivate, method->isFunction, new VariableExprNode(method->loc, method->thisName->value), args, new BlockExprNode(method->loc, newStatements));
                                methods->push_back(methodDecNode);
                            }
                        }
                    }
                    // insert before methods parsing
                    typeNode = new TypeDecStatementNode(typeLoc, new VariableExprNode(typeLoc, mainModuleNode->name->value + "." + name), isPrivate, fields, methods, isExtern, parent);
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
                        auto constructorExists = std::find_if(methods->begin(), methods->end(), [&name, this](MethodDecNode *method) {
                            return method->name->value == mainModuleNode->name->value + "." + name + "._DefaultConstructor_";
                        });
                        if (constructorExists == methods->end())
                        {
                            std::string constructorName = name + "._DefaultConstructor_";
                            std::string constructorType;
                            bool constructorIsPrivate = false;
                            auto *constructorStatements = new std::vector<StatementNode*>();
                            //constructorStatements->push_back(new AssignExprNode());
                            for (auto field : *fields)
                            {
                                if (field->isA<FieldVarDecNode>() || field->isA<FieldArrayVarDecNode>())
                                {
                                    constructorStatements->push_back(field);
                                }
                            }
                            auto constructor = new MethodDecNode(typeLoc, new VariableExprNode(typeLoc, constructorType), new VariableExprNode(typeLoc, mainModuleNode->name->value + "." + constructorName), constructorIsPrivate, false, new VariableExprNode(typeLoc, name),
                                                                 nullptr, new BlockExprNode(typeLoc, constructorStatements));
                            methods->push_back(constructor);
                        }
                        else {
                            // check what fields are already added from parent and add only new ones
                            auto constructor = *constructorExists;
                            for (auto field : *fields)
                            {
                                if (field->isA<FieldVarDecNode>() || field->isA<FieldArrayVarDecNode>())
                                {
                                    auto fieldExists = std::find_if(constructor->block->statements->begin(), constructor->block->statements->end(), [&field](StatementNode *statement) {
                                        if (statement->isA<FieldVarDecNode>())
                                        {
                                            return field->name->value == statement->as<FieldVarDecNode>()->name->value;
                                        }
                                        else if (statement->isA<FieldArrayVarDecNode>())
                                        {
                                            return field->name->value == statement->as<FieldArrayVarDecNode>()->name->value;
                                        }
                                        return false;
                                    });
                                    if (fieldExists == constructor->block->statements->end())
                                    {
                                        if (field->isA<FieldVarDecNode>())
                                        {
                                            field->as<FieldVarDecNode>()->index += parent->fields->size();
                                        }
                                        else if (field->isA<FieldArrayVarDecNode>())
                                        {
                                            field->as<FieldArrayVarDecNode>()->index += parent->fields->size();
                                        }
                                        constructor->block->statements->push_back(field);
                                    }
                                }
                            }
                        }
                    }

                    advance();
                    if (token.type == TokenType::Identifier && token.data == name && advance().type == TokenType::Semicolon)
                    {
                        currentScope = scope->parent;
                        typeNode = new TypeDecStatementNode(typeLoc, new VariableExprNode(typeLoc, mainModuleNode->name->value + "." + name), isPrivate, fields, methods, isExtern, parent);
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
    currentParsingTypeName = "";
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
        SourceLoc loc{token.stringNumber, token.symbolNumber};
        if (match(TokenType::Or)) {
            result = new OperatorExprNode(loc, result, TokenType::Or, parseAnd());
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
        SourceLoc loc{token.stringNumber, token.symbolNumber};
        if (match(TokenType::And)) {
            result = new OperatorExprNode(loc, result, TokenType::And, parseEquality());
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
        SourceLoc loc{token.stringNumber, token.symbolNumber};
        if (match(TokenType::Equal)) {
            result = new OperatorExprNode(loc, result, TokenType::Equal, parseConditional());
            continue;
        }
        if (match(TokenType::NotEqual)) {
            result = new OperatorExprNode(loc, result, TokenType::NotEqual, parseConditional());
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
        SourceLoc loc{token.stringNumber, token.symbolNumber};
        if (match(TokenType::Less)) {
            result = new OperatorExprNode(loc, result, TokenType::Less, parseAddSub());
            continue;
        }
        if (match(TokenType::LessOrEqual)) {
            result = new OperatorExprNode(loc, result, TokenType::LessOrEqual, parseAddSub());
            continue;
        }
        if (match(TokenType::Greater)) {
            result = new OperatorExprNode(loc, result, TokenType::Greater, parseAddSub());
            continue;
        }
        if (match(TokenType::GreaterOrEqual)) {
            result = new OperatorExprNode(loc, result, TokenType::GreaterOrEqual, parseAddSub());
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
        SourceLoc loc{token.stringNumber, token.symbolNumber};
        if (match(TokenType::Plus)) {
            result = new OperatorExprNode(loc, result, TokenType::Plus, parseMulDiv());
            continue;
        }
        if (match(TokenType::Minus)) {
            result = new OperatorExprNode(loc, result, TokenType::Minus, parseMulDiv());
        }
        break;
    }

    return result;
}

ExprNode *Parser::parseMulDiv() {
    auto result = parseUnary();

    while (true)
    {
        SourceLoc loc{token.stringNumber, token.symbolNumber};
        if (match(TokenType::Multiplication)) {
            result = new OperatorExprNode(loc, result, TokenType::Multiplication, parseUnary());
            continue;
        }
        if (match(TokenType::Division)) {
            result = new OperatorExprNode(loc, result, TokenType::Division, parseUnary());
        }
        if (match(TokenType::Remainder)) {
            result = new OperatorExprNode(loc, result, TokenType::Remainder, parseUnary());
        }
        break;
    }

    return result;
}

ExprNode *Parser::parseUnary() {
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    return match(TokenType::Minus) ? new UnaryOperatorExprNode(loc, TokenType::Minus, parsePrimary()) : parsePrimary();
}

ExprNode *Parser::parsePrimary() {
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    Token tok = token;
    advance();
    if (tok.type == TokenType::Boolean) return new BooleanExprNode(loc, tok.data == "true");
    if (tok.type == TokenType::Nil)
        return new NilExprNode(loc);
    else if (tok.type == TokenType::String)
    {
        if (tok.data.size() == 1) return new CharExprNode(loc, tok.data[0]);
        else return new StringExprNode(loc, tok.data);
    }
    else if (tok.type == TokenType::Identifier)
    {
        // vars, funcs, arrays, fields
        return reinterpret_cast<ExprNode *>(parseVarOrCall());
    }
    else if (tok.type == TokenType::Integer) return new IntExprNode(loc, std::stoi(tok.data));
    else if (tok.type == TokenType::Real) return new RealExprNode(loc, std::stod(tok.data));
    else if (tok.type == TokenType::Float) return new FloatExprNode(loc, std::stof(tok.data));
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
    SourceLoc loc{token.stringNumber, token.symbolNumber};
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
            type = (currentScope->lookup(name))->as<TypeDecStatementNode>();
            if (type != nullptr) dotClass = true;
        }
        if (!dotModule && !dotClass)
        {
            auto var = currentScope->lookup(name);
            if (var == nullptr)
            {
                var = currentScope->lookup(mainModuleNode->name->value + "." + name);
                if (var == nullptr)
                {
                    llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Variable " << name << " does not exist.\n";
                    hasError = true;
                    return nullptr;
                }
            }

            if (var->isA<VarDecStatementNode>())
                type = (currentScope->lookup(var->as<VarDecStatementNode>()->type))->as<TypeDecStatementNode>();
            else if (var->isA<FuncParamDecStatementNode>()) {
                auto funcParamVar = var->as<FuncParamDecStatementNode>();
                auto typeString = (funcParamVar->type)->as<VariableExprNode>()->value;
                type = (currentScope->lookup(typeString))->as<TypeDecStatementNode>();
            }

            if (type != nullptr)
            {
                if (type->isPrivate && !type->name->value.starts_with(mainModuleNode->name->value + "."))
                {
                    llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Cannot access private type " << type->name->value << ".\n";
                    hasError = true;
                }
                dotClass = true;
            }
        }
        advance();
        expect(TokenType::Identifier);
        if (dotModule)
        {
            if (moduleP->currentScope->lookup(name + "." + token.data) != nullptr)
            {
                name += "." + token.data;

                auto var = moduleP->currentScope->lookup(name);
                // check is private
                if (var->isA<VarDecStatementNode>())
                {
                    if (var->as<VarDecStatementNode>()->isPrivate && moduleP->mainModuleNode->name->value != mainModuleNode->name->value)
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Cannot access private variable " << name << ".\n";
                        hasError = true;
                    }
                }
                else if (var->isA<FuncDecStatementNode>()) {
                    if ((var)->as<FuncDecStatementNode>()->isPrivate && moduleP->mainModuleNode->name->value != mainModuleNode->name->value)
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Cannot access private function " << name << ".\n";
                        hasError = true;
                    }
                }
                else if (var->isA<ExternFuncDecStatementNode>())
                {
                    if ((var)->as<ExternFuncDecStatementNode>()->isPrivate && moduleP->mainModuleNode->name->value != mainModuleNode->name->value)
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Cannot access private extern function " << name << ".\n";
                        hasError = true;
                    }
                }
            }
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
                    if (field->isA<FieldVarDecNode>())
                    {
                        auto f = field->as<FieldVarDecNode>();
                        fieldIndex = f->index;
                        if (f->isPrivate && type->name->value != currentParsingTypeName)
                        {
                            llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Field " + name + " is private member of class " + type->name->value + ".\n";
                            hasError = true;
                        }
                    }
                    else if (field->isA<FieldArrayVarDecNode>()) {
                        fieldIndex = field->as<FieldArrayVarDecNode>()->index;
                        fieldArray = field->as<FieldArrayVarDecNode>();
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
                    if (method->isPrivate && type->name->value != currentParsingTypeName)
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Method " + methodName + " is private method of class " + type->name->value + ".\n";
                        hasError = true;
                    }
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
                type = (currentScope->lookup(getArrayFinalType(fieldArray->var->expr)))->as<TypeDecStatementNode>();
            }
            else {
                auto var = currentScope->lookup(name);
                if (var == nullptr)
                {
                    var = currentScope->lookup(mainModuleNode->name->value + "." + name);
                    if (var == nullptr)
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Variable " << name << " does not exist.\n";
                        hasError = true;
                        return nullptr;
                    }
                }
                type = (currentScope->lookup(getArrayFinalType((var)->as<ArrayDecStatementNode>()->expr)))->as<TypeDecStatementNode>();
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
                        if (field->isA<FieldVarDecNode>()) fieldIndex = (field)->as<FieldVarDecNode>()->index;
                        else if (field->isA<FieldArrayVarDecNode>()) fieldIndex = (field)->as<FieldArrayVarDecNode>()->index;
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
                varExpr = new IndexExprNode(loc, name, indexes->at(0), dotModule, dotClass, fieldIndex, true);
            else
                varExpr = new IndexesExprNode(loc, name, indexes, dotModule, dotClass, fieldIndex, true);
            //varExpr = new IndexExprNode(name, index, dotModule, dotClass, fieldIndex, true);
        }
        else
        {
            //varExpr = new IndexExprNode(name, index, dotModule, dotClass, fieldIndex, true);
            //return new IndexesExprNode(name, indexes, dotModule, dotClass, fieldIndex);
            if (indexes->size() == 1)
                return new IndexExprNode(loc, name, indexes->at(0), dotModule, dotClass, fieldIndex);
            else
                return new IndexesExprNode(loc, name, indexes, dotModule, dotClass, fieldIndex);

        }
    }
    if (token.type != TokenType::LParen) {
        return new VariableExprNode(loc, name, E_UNKNOWN, dotModule, dotClass, fieldIndex);
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

    if (funcDecl->isA<FuncDecStatementNode>()) {
        funcDecl = funcDecl->as<FuncDecStatementNode>();
    }
    else if (funcDecl->isA<ExternFuncDecStatementNode>()) {
        funcDecl = funcDecl->as<ExternFuncDecStatementNode>();
    }
    else if (funcDecl->isA<FuncPointerStatementNode>()) {
        funcDecl = funcDecl->as<FuncPointerStatementNode>();
    }
    if (funcDecl->isA<FuncDecStatementNode>())
    {
        if (params->size() == (funcDecl)->as<FuncDecStatementNode>()->args->size()) {
            int i = 0;
            while (i < params->size()) {
                if ((params->at(i))->isA<NilExprNode>()) {
                    (params->at(i))->as<NilExprNode>()->type = (funcDecl)->as<FuncDecStatementNode>()->args->at(i)->type;
                }
                i++;
            }
        }
    }
    else if (funcDecl->isA<ExternFuncDecStatementNode>())
    {
        if (params->size() == (funcDecl)->as<ExternFuncDecStatementNode>()->args->size()) {
            int i = 0;
            while (i < params->size()) {
                if ((params->at(i))->isA<NilExprNode>()) {
                    (params->at(i))->as<NilExprNode>()->type = (funcDecl)->as<ExternFuncDecStatementNode>()->args->at(i)->type;
                }
                i++;
            }
        }
    }
    else if (funcDecl->isA<FuncPointerStatementNode>())
    {
        if (params->size() == (funcDecl)->as<FuncPointerStatementNode>()->args->size()) {
            int i = 0;
            while (i < params->size()) {
                if ((params->at(i))->isA<NilExprNode>()) {
                    (params->at(i))->as<NilExprNode>()->type = (funcDecl)->as<FuncPointerStatementNode>()->args->at(i)->type;
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
            params->insert(params->begin(), new VariableExprNode(loc, name, E_UNKNOWN, dotModule, dotClass));
        }
        return new CallExprNode(loc, new VariableExprNode(loc, methodName), params);
    }
    else
    {

        return new CallExprNode(loc, new VariableExprNode(loc, name, E_UNKNOWN, dotModule, dotClass), params);
    }
    return nullptr;
}

DeclarationNode* Parser::parseVariableDecl(bool isGlobal) {
    std::string type;
    std::string name;
    bool isPrivate = false;
    bool isExtern = false;
    SourceLoc loc{token.stringNumber, token.symbolNumber};
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
        ArrayExprNode* arrExpr = new ArrayExprNode(loc, arrType, size, new std::vector<ExprNode*>());
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
                    auto typeStatement = (currentScope->lookup(type))->as<TypeDecStatementNode>();
                    if (typeStatement == nullptr)
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Unknown type \"" << type << "\".\n";
                        hasError = true;
                    }
                    if (typeStatement->isPrivate && !typeStatement->name->value.starts_with(mainModuleNode->name->value + "."))
                    {
                        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Cannot access private type " << typeStatement->name->value << ".\n";
                        hasError = true;
                    }
                    tokensIterator--;
                    token = *tokensIterator;
                }
            }
            arrExpr->values->push_back(new ArrayExprNode(loc, arrType, size, nullptr));
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
                funcType = new VariableExprNode(loc, "");
            }
        }
        else funcType = new VariableExprNode(loc, "");
        advance();
        name = consume(TokenType::Identifier).data;
    }
    else
    {
        type = parseTypeName(type);
        auto typeStatement = (currentScope->lookup(type))->as<TypeDecStatementNode>();
        if (typeStatement == nullptr)
        {
            llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Unknown type \"" << type << "\".\n";
            hasError = true;
        }
        if (typeStatement->isPrivate && !typeStatement->name->value.starts_with(mainModuleNode->name->value + "."))
        {
            llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Cannot access private type " << typeStatement->name->value << ".\n";
            hasError = true;
        }
        name = consume(TokenType::Identifier).data;
    }
    if (token.type == TokenType::Assign)
    {
        advance();
        expr = parseExpression();
    }
    expect(TokenType::Semicolon);
    if (currentScope->lookup(name) != nullptr || oneOfDefaultTypes(name))
    {
        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") Name conflict - \"" << name << "\".\n";
        hasError = true;
    }
    if (isGlobal && !isExtern) name = mainModuleNode->name->value + "." + name;
    if (isArray) {
        result = new ArrayDecStatementNode(loc, new VariableExprNode(loc, name), expr->as<ArrayExprNode>(), isGlobal, indicesCount, isPrivate, isExtern);
    }
    else if (isFuncPointer) {
        result = new FuncPointerStatementNode(loc, funcType, new VariableExprNode(loc, name), isFunction, isGlobal, args, expr, isPrivate, isExtern);
    }
    else {
        result = new VarDecStatementNode(loc, type, new VariableExprNode(loc, name), expr, isGlobal, isPrivate, isExtern);
    }
    //if (isGlobal)
        //currentScope->insert(result);
    currentScope->insert(result);
    return result;
}

DeclarationNode *Parser::parseFunctionDecl() {
    tokensIterator--;
    token = *tokensIterator;
    SourceLoc loc{token.stringNumber, token.symbolNumber};
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
            auto typeStr = parseTypeName(token.data);
            type = lookupTypes(typeStr);
        }
        else
        {
            type = new VariableExprNode({}, "");
        }
    }
    else type = new VariableExprNode({}, "");
    VariableExprNode *funcName = nullptr;
    if (isExtern) {
        funcName = new VariableExprNode(loc, name);
    } else {
        funcName = new VariableExprNode(loc, mainModuleNode->name->value + "." + name);
    }
    DeclarationNode* function = nullptr;
    BlockExprNode* block = nullptr;
    if (isExtern)
    {
        function = new ExternFuncDecStatementNode(loc, type, funcName, isPrivate, isFunction, params, block);
    } else {
        function = new FuncDecStatementNode(loc, type, funcName, isPrivate, isFunction, params, block);
    }

    currentScope->insert(function);
    block = parseBlock(new VariableExprNode(loc, name), params);
    if (function->isA<ExternFuncDecStatementNode>()) (function)->as<ExternFuncDecStatementNode>()->block = block;
    else if (function->isA<FuncDecStatementNode>()) (function)->as<FuncDecStatementNode>()->block = block;

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

    SourceLoc loc{token.stringNumber, token.symbolNumber};
    if (token.type != TokenType::Else && token.type != TokenType::Elseif)
    {

        bool blockEnd = false;
        auto *statements = new std::vector<StatementNode*>();
        auto *block = new BlockExprNode(loc, statements);
        while (!blockEnd)
        {
            advance();
            if (token.type == TokenType::If) statements->push_back(parseIfStatement());
            else if (token.type == TokenType::Variable) statements->push_back(parseVariableDecl());
            else if (token.type == TokenType::Output) statements->push_back(parseOutputStatement());
            else if (token.type == TokenType::Input) statements->push_back(parseInputStatement());
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
        trueBlock = new BlockExprNode(loc, statements);
    }
    while (token.type == TokenType::Elseif) elseifNodes->push_back(parseElseIfBlock());
    if (token.type == TokenType::Else) falseBlock = parseElseBlock();
    //advance();
    consume(TokenType::End);
    consume(TokenType::If);
    expect(TokenType::Semicolon);
    return new IfStatementNode(loc, expr, trueBlock, elseifNodes, falseBlock);
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
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    while (!blockEnd)
    {
        advance();
        if (token.type == TokenType::If) statements->push_back(parseIfStatement());
        else if (token.type == TokenType::Variable) statements->push_back(parseVariableDecl());
        else if (token.type == TokenType::Output) statements->push_back(parseOutputStatement());
        else if (token.type == TokenType::Input) statements->push_back(parseInputStatement());
        else if (token.type == TokenType::Return) statements->push_back(parseReturnStatement());
        else if (token.type == TokenType::Let) statements->push_back(parseAssignStatement());
        else if (token.type == TokenType::While) statements->push_back(parseWhileStatement());
        else if (token.type == TokenType::Call) statements->push_back(parseCall());
        else if (token.type == TokenType::Delete) statements->push_back(parseDelete());
        if (token.type == TokenType::Else || token.type == TokenType::Elseif)
        {
            blockEnd = true;
        }
    }
    block = new ElseIfStatementNode(loc, expr, new BlockExprNode(loc, statements));
    return block;
}

BlockExprNode *Parser::parseElseBlock() {
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    BlockExprNode *falseBlock = nullptr;
    bool blockEnd = false;
    auto *statements = new std::vector<StatementNode*>();
    auto *block = new BlockExprNode(loc, statements);
    while (!blockEnd)
    {
        advance();
        if (token.type == TokenType::If) statements->push_back(parseIfStatement());
        else if (token.type == TokenType::Variable) statements->push_back(parseVariableDecl());
        else if (token.type == TokenType::Output) statements->push_back(parseOutputStatement());
        else if (token.type == TokenType::Input) statements->push_back(parseInputStatement());
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
    falseBlock = new BlockExprNode(loc, statements);
    return falseBlock;
}

OutputStatementNode *Parser::parseOutputStatement() {
    SourceLoc loc{token.stringNumber, token.symbolNumber};
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
    expect(TokenType::Semicolon);
    return new OutputStatementNode(loc, expr);
}

InputStatementNode *Parser::parseInputStatement() {
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    consume(TokenType::Input);
    auto expr = parseExpression();
    expect(TokenType::Semicolon);
    return new InputStatementNode(loc, expr);
}

AssignExprNode *Parser::parseAssignStatement() {
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    consume(TokenType::Let);
    advance();
    auto varName = parseVarOrCall();
    // TODO: ???
    //tokensIterator--;
    //token = *tokensIterator;
    consume(TokenType::Assign);
    auto expr = parseExpression();
    if (varName->isA<VariableExprNode>())
    {
        return new AssignExprNode(loc, varName->as<VariableExprNode>(), expr);
    }
    else if (varName->isA<IndexExprNode>())
    {
        return new AssignExprNode(loc, varName->as<IndexExprNode>(), expr);
    }
    expect(TokenType::Semicolon);
    return nullptr;
}

ReturnStatementNode *Parser::parseReturnStatement() {
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    consume(TokenType::Return);
    auto expr = parseExpression();
    expect(TokenType::Semicolon);
    return new ReturnStatementNode(loc, expr);
}

WhileStatementNode *Parser::parseWhileStatement() {
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    consume(TokenType::While);
    auto expr = parseExpression();
    expect(TokenType::Repeat);
    BlockExprNode* block = parseBlock(new VariableExprNode(loc, "while"));
    expect(TokenType::Semicolon);
    return new WhileStatementNode(loc, expr, block);
}

CallExprNode *Parser::parseCall() {
    consume(TokenType::Call);
    advance();
    auto callExpr = parseVarOrCall();
    if (callExpr == nullptr)
    {
        hasError = true;
        return nullptr;
    }
    else if (!callExpr->isA<CallExprNode>())
    {
        llvm::errs() << "[ERROR] (" << token.stringNumber << ", " << token.symbolNumber << ") " << callExpr->as<VariableExprNode>()->value << " is not a function.\n";
        hasError = true;
    }
    expect(TokenType::Semicolon);
    return callExpr->as<CallExprNode>();
}

DeleteExprNode *Parser::parseDelete() {
    SourceLoc loc{token.stringNumber, token.symbolNumber};
    consume(TokenType::Delete);
    auto varExpr = parseExpression();
    expect(TokenType::Semicolon);
    return new DeleteExprNode(loc, varExpr);
}

ExternFuncDecStatementNode *Parser::parseExternFuncDecl() {
    return nullptr;
}
