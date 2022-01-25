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

FieldDecNode* Parser::parseFieldDecl(std::vector<FieldDecNode *> &fields, std::string &thisClassName) {
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
                exit(0); // error field already exists
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

MethodDecNode* Parser::parseMethodDecl(std::vector<MethodDecNode*> &methods, std::string &thisClassName) {
    MethodDecNode *method = nullptr;
    return method;
}

bool Parser::parseTypeDecl() {
    std::vector<FieldDecNode*> fields;
    std::vector<MethodDecNode*> methods;
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
                auto decl = currentScope->lookup(new VariableExprNode(token.data));
                if (decl != nullptr && dynamic_cast<DeclarationNode*>(decl) != nullptr)
                {
                    parent = static_cast<TypeDecStatementNode *>(decl);
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
                                    fields.push_back(field);
                                    //if (DEBUG) llvm::outs() << "parsed field " << field->name->value << " " << field->isPrivate << "\n";
                                }
                                else
                                {
                                    // error
                                }
                            }
                            if (token.type == TokenType::Method)
                            {
                                //tokensIterator -= 2;
                                //token = *tokensIterator;
                                //parseFieldDecl(fields, value);
                                advance();
                                std::string methodName = token.data;
                                // !!!!!!!!!!!!!!!PLACEHOLDER!!!!!!!!!!!!!!!
                                while (token.type != TokenType::End)
                                {
                                    advance();
                                }
                                advance();
                                advance();
                                // !!!!!!!!!!!!!!!PLACEHOLDER!!!!!!!!!!!!!!!
                            }
                        }
                    }
                    advance();
                    if (token.type == TokenType::Identifier && token.data == name && advance().type == TokenType::Semicolon)
                    {
                        currentScope = scope->parent;
                        TypeDecStatementNode typeNode(new VariableExprNode(name), &fields, &methods, parent);
                        currentScope->insert(&typeNode);
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
