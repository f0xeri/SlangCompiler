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

using Parameter = std::pair<std::string, std::string>;

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
        currentScope->insert(new TypeDecStatementNode(new VariableExprNode("Object"), {}, {}, nullptr));
        this->tokens = tokens;
    }
    ~Parser()
    {
        delete currentScope;
        delete mainModuleNode;
    };

    Token advance()
    {
        if (token.type == TokenType::EndOfFile)
        {
            llvm::errs() << "[ERROR] Unexpected EOF token.\n";
            return token;
        }
        tokensIterator++;
        token = *tokensIterator;
        return token;
    }
    Token token;
    bool hasError = false;

    void error()
    {
        llvm::errs() << "[ERROR] Unexpected token \"" << Lexer::getTokenName(token.type) << "\".\n";
        hasError = true;
    }

    bool expect(TokenType tokenType)
    {
        if (token.type != tokenType)
        {
            llvm::errs() << "[ERROR] Unexpected token \"" << Lexer::getTokenName(token.type) + "\", expected \"" + Lexer::getTokenName(tokenType) + "\".\n";
            hasError = true;
            return false;
        }
        return true;
    }

    Token consume(TokenType tokenType)
    {
        if (!expect(tokenType)) {

            //exit(0);
        }
        Token tok = token;
        advance();
        return tok;
    }

    bool match(TokenType tokenType)
    {
        if (token.type != tokenType) {

            return false;
        }
        advance();
        return true;
    }

    bool oneOfDefaultTypes(const std::string &name)
    {
        if (name == "integer" || name == "real" || name == "boolean" || name == "string" || name == "character") return true;
        return false;
    }

    bool isFieldNameCorrect(std::vector<FieldVarDecNode *> *fields, const std::string &name)
    {
        if (std::find_if(fields->begin(), fields->end(), [&name](FieldVarDecNode *field){ return field->name->value == name; }) != fields->end())
        {
            return false;
        }
        return true;
    }

    std::vector<FuncParamDecStatementNode *>* parseFuncParameters();

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
        return field;
    }

    void parse();
    ModuleStatementNode *mainModuleNode;
    bool parseImports();
    bool parseModuleDecl();
    BlockExprNode* parseBlock(VariableExprNode *name);
    bool parseVisibilityOperator();
    VarDecStatementNode* parseVariableDecl(bool isGlobal = false);
    bool parseStatement();
    FieldVarDecNode* parseFieldDecl(std::vector<FieldVarDecNode *> *fields, std::string &thisClassName, bool &constructorRequired);
    MethodDecNode* parseMethodDecl(std::vector<MethodDecNode*> *methods, std::string &thisClassName);
    FuncDecStatementNode* parseFunctionDecl();
    IfStatementNode* parseIfStatement();
    ElseIfStatementNode* parseElseIfBlock();
    BlockExprNode* parseElseBlock();
    AssignExprNode* parseAssignStatement();
    OutputStatementNode* parseOutputStatement();
    ReturnStatementNode* parseReturnStatement();
    WhileStatementNode* parseWhileStatement();
    bool parseTypeDecl();
    StatementNode * parseVarOrCall();
    ExprNode* parseExpression();
    ExprNode* parseStrInterpolation();
    ExprNode* parseOr();
    ExprNode* parseAnd();
    ExprNode* parseEquality();
    ExprNode* parseConditional();
    ExprNode* parseAddSub();
    ExprNode* parseMulDiv();
    ExprNode* parseUnary();
    ExprNode* parsePrimary();
};


#endif //SLANGPARSER_PARSER_HPP