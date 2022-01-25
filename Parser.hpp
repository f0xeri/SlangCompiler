//
// Created by Yaroslav on 22.01.2022.
//
#ifndef SLANGPARSER_PARSER_HPP
#define SLANGPARSER_PARSER_HPP

#define DEBUG 1

#include <llvm/Support/raw_ostream.h>
#include "Lexer.hpp"
#include "AST.hpp"
#include "Scope.hpp"

class Parser {
public:
    std::vector<Token> tokens;
    std::vector<Token>::iterator tokensIterator;
    Scope *currentScope;

    Parser(std::vector<Token> &tokens)
    {
        tokensIterator = tokens.begin();
        token = *tokensIterator;
        currentScope = new Scope();
        currentScope->insert(new TypeDecStatementNode(new VariableExprNode("Object"), nullptr, nullptr, nullptr));
        this->tokens = tokens;
    }

    Token advance()
    {
        tokensIterator++;
        token = *tokensIterator;
        return token;
    }
    Token token;
    bool hasError = false;

    void error()
    {
        llvm::errs() << "Unexpected: " << Lexer::getTokenName(token.type) << "\n";
        hasError = true;
    }

    bool expect(TokenType tokenType)
    {
        if (token.type != tokenType)
        {
            error();
            return false;
        }
        return true;
    }

    bool consume(TokenType tokenType)
    {
        if (expect(tokenType)) {
            advance();
            return true;
        }
        advance();
        return false;
    }

    bool oneOfDefaultTypes(const std::string &name)
    {
        if (name == "integer" || name == "real" || name == "boolean" || name == "string" || name == "character") return true;
        return false;
    }

    bool isFieldNameCorrect(std::vector<FieldDecNode *> &fields, const std::string &name)
    {
        if (std::find_if(fields.begin(), fields.end(), [&name](FieldDecNode *field){ return field->name->value == name; }) != fields.end())
        {
            return false;
        }
        return true;
    }

    FieldVarDecNode* initDefaultType(bool isPrivate, const std::string &name, const std::string &type, TokenType dataType, const std::string &data)
    {
        FieldVarDecNode *field = nullptr;
        if (type == "integer")
        {
            if (dataType == TokenType::Integer)
            {
                field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type,new IntExprNode(std::stoi(token.data)));
            }
            else
            {
                field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type,new IntExprNode(0));
            }
        }
        else if (type == "real")
        {
            if (dataType == TokenType::Real)
            {
                field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type, new RealExprNode(std::stod(data)));
            }
            else
            {
                field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type, new RealExprNode(0.0));
            }
        }
        else if (type == "boolean")
        {
            if (dataType == TokenType::Boolean)
            {
                bool bdata = data == "true";
                field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type,new BooleanExprNode(bdata));
            }
            else
            {
                field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type,new BooleanExprNode(false));
            }
        }
        else if (type == "character")
        {
            if (data.length() == 1)
            {
                char chdata = data[0];
                field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type, new CharExprNode(chdata));
            }
            else if (data.length() == 0)
            {
                field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type, new CharExprNode(NULL));
            }

        }
        else if (type == "string")
        {
            field = new FieldVarDecNode(new VariableExprNode(name), isPrivate, type, new StringExprNode(data));
        }

        if (DEBUG) llvm::outs() << "parsed field " << type << " " << name << " - " << data << "\n";
        return field;
    }

    void parse();
    ModuleStatementNode *mainModuleNode;
    bool parseImports();
    bool parseModuleDecl();
    bool parseBlock(VariableExprNode &name);
    bool parseVisibilityOperator();
    bool parseStatement();
    bool parseFuncDecl();
    FieldDecNode* parseFieldDecl(std::vector<FieldDecNode *> &fields, std::string &thisClassName);
    MethodDecNode* parseMethodDecl(std::vector<MethodDecNode*> &methods, std::string &thisClassName);
    bool parseTypeDecl();
};


#endif //SLANGPARSER_PARSER_HPP