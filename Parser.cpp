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
    bool ret = false;
    if (token.type == TokenType::Function) ret = parseFuncDecl();
    else if (token.type == TokenType::Procedure) ret = parseFuncDecl();
    else if (token.type == TokenType::Class) ret = parseTypeDecl();
    return ret;
}

bool Parser::parseStatement() {
    return false;
}

bool Parser::parseFuncDecl() {
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
                if (init) advance();
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
                if (init)
                {
                    advance();
                    expect(TokenType::Semicolon);
                }
            }
            else error();
        }
    }
    return field;
}

std::vector<VarDecStatementNode*> *Parser::parseFuncParameters()
{
    auto* params = new std::vector<VarDecStatementNode*>();
    consume(TokenType::LParen);
    while (token.type != TokenType::RParen)
    {
        std::string type, name;
        type = consume(TokenType::Identifier).data;
        name = consume(TokenType::Identifier).data;
        params->push_back(new VarDecStatementNode(type, new VariableExprNode(name)));
        advance();
        if (expect(TokenType::Comma)) advance();
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
