//
// Created by Yaroslav on 22.01.2022.
//
#ifndef SLANGPARSER_PARSER_HPP
#define SLANGPARSER_PARSER_HPP

#include <llvm/Support/raw_ostream.h>
#include "Lexer.hpp"
#include "AST.hpp"
#include "Scope.hpp"

using Parameter = std::pair<std::string, std::string>;

class Parser {
public:
    std::vector<Token> tokens;
    std::vector<Token>::iterator tokensIterator;
    Scope* currentScope;

    std::vector<Parser*>* importedModules;

    Parser(std::vector<Token> &tokens)
    {
        tokensIterator = tokens.begin();
        token = *tokensIterator;
        currentScope = new Scope();
        auto objectType = new TypeDecStatementNode(new VariableExprNode("Object"), false, nullptr, nullptr);
        objectType->methods = new std::vector<MethodDecNode*>();
        auto toStringType = new ArrayExprNode("character", nullptr, new std::vector<ExprNode*>());
        auto args = new std::vector<FuncParamDecStatementNode *>();
        args->push_back(new FuncParamDecStatementNode(new VariableExprNode("Object"), new VariableExprNode("this"), ParameterType::Var));
        objectType->methods->push_back(new MethodDecNode(toStringType, new VariableExprNode("Object.toString"), false, true, new VariableExprNode("this"), args, nullptr));
        currentScope->insert(objectType);
        importedModules = new std::vector<Parser*>();
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

    bool isFieldNameCorrect(std::vector<DeclarationNode *> *fields, const std::string &name)
    {
        if (std::find_if(fields->begin(), fields->end(), [&name](DeclarationNode *field){ return field->name->value == name; }) != fields->end())
        {
            return false;
        }
        return true;
    }

    std::vector<FuncParamDecStatementNode *>* parseFuncParameters();

    FieldVarDecNode* initDefaultType(const std::string &typeName, bool isPrivate, const std::string &name, const std::string &type, TokenType dataType, const std::string &data, int index)
    {
        FieldVarDecNode *field = nullptr;
        if (type == "integer")
        {
            if (dataType == TokenType::Integer)
            {
                field = new FieldVarDecNode(typeName, new VariableExprNode(name), isPrivate, type,new IntExprNode(std::stoi(token.data)), index);
            }
            else
            {
                field = new FieldVarDecNode(typeName, new VariableExprNode(name), isPrivate, type,new IntExprNode(0), index);
            }
        }
        else if (type == "real")
        {
            if (dataType == TokenType::Real)
            {
                field = new FieldVarDecNode(typeName, new VariableExprNode(name), isPrivate, type, new RealExprNode(std::stod(data)), index);
            }
            else
            {
                field = new FieldVarDecNode(typeName, new VariableExprNode(name), isPrivate, type, new RealExprNode(0.0), index);
            }
        }
        else if (type == "boolean")
        {
            if (dataType == TokenType::Boolean)
            {
                bool bdata = data == "true";
                field = new FieldVarDecNode(typeName, new VariableExprNode(name), isPrivate, type, new BooleanExprNode(bdata), index);
            }
            else
            {
                field = new FieldVarDecNode(typeName, new VariableExprNode(name), isPrivate, type, new BooleanExprNode(false), index);
            }
        }
        else if (type == "character")
        {
            if (data.length() == 1)
            {
                char chdata = data[0];
                field = new FieldVarDecNode(typeName, new VariableExprNode(name), isPrivate, type, new CharExprNode(chdata), index);
            }
            else if (data.length() == 0)
            {
                field = new FieldVarDecNode(typeName, new VariableExprNode(name), isPrivate, type, new CharExprNode(NULL), index);
            }

        }
        else if (type == "string")
        {
            field = new FieldVarDecNode(typeName, new VariableExprNode(name), isPrivate, type, new StringExprNode(data), index);
        }
        return field;
    }

    ExprNode* lookupTypes(const std::string &type)
    {
        ExprNode *expr = nullptr;
        bool isArray = false;
        if (oneOfDefaultTypes(type))
        {
            return new VariableExprNode(type);
        }
        else if (type == "array")
        {
            isArray = true;
            std::string typeOfArray;
            ExprNode* size;
            advance();
            consume(TokenType::LBracket);
            if (token.type != TokenType::RBracket)
                size = parseExpression();
            else
                size = nullptr;
            consume(TokenType::RBracket);
            std::string arrType = token.data;
            ArrayExprNode* arrExpr = new ArrayExprNode(arrType, size, new std::vector<ExprNode*>());
            while (token.data == "array")
            {
                advance();
                consume(TokenType::LBracket);
                if (token.type != TokenType::RBracket)
                    size = parseExpression();
                else
                    size = nullptr;
                consume(TokenType::RBracket);
                if (token.type == TokenType::Identifier)
                {
                    arrType = token.data;
                    if (!oneOfDefaultTypes(arrType) && arrType != "array")
                    {
                        auto typeStatement = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(type));
                        if (typeStatement == nullptr)
                        {
                            llvm::errs() << "[ERROR] Unknown type \"" << type << "\".\n";
                        }
                    }
                }
                arrExpr->values->push_back(new ArrayExprNode(arrType, size, nullptr));
            }
            expr = arrExpr;
        }
        else
        {
            auto typeStatement = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(type));
            if (typeStatement == nullptr)
            {
                typeStatement = dynamic_cast<TypeDecStatementNode*>(currentScope->lookup(mainModuleNode->name->value + "." + type));
                if (typeStatement == nullptr)
                {
                    llvm::errs() << "[ERROR] Unknown type \"" << type << "\".\n";
                }
            }
            expr = new VariableExprNode(typeStatement->name->value);
            // arrays and objects
        }
        return expr;
    }

    std::string getArrayFinalType(ArrayExprNode* exprNode)
    {
        std::string type;
        if (exprNode->type == "array")
        {
            for (auto &slice : *exprNode->values)
            {
                auto castedSlice = dynamic_cast<ArrayExprNode*>(slice);
                //auto sliceSize = castedSlice->size->codegen(cgcontext);
                //auto newArraySize = BinaryOperator::Create(Instruction::Mul, sliceSize, arraySize, "", cgcontext.currentBlock());
                //arraySize = newArraySize;
                exprNode = castedSlice;
            }
        }
        type = exprNode->type;
        return type;
    }

    void parse();
    ModuleStatementNode *mainModuleNode;
    bool parseImports();
    bool parseModuleDecl();
    BlockExprNode* parseBlock(VariableExprNode *name);
    bool parseVisibilityOperator();
    DeclarationNode* parseVariableDecl(bool isGlobal = false);
    bool parseStatement();
    DeclarationNode* parseFieldDecl(std::vector<DeclarationNode *> *fields, std::string &thisClassName, bool &constructorRequired, int index);
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
    StatementNode* parseVarOrCall();
    CallExprNode* parseCall();
    DeleteExprNode* parseDelete();
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