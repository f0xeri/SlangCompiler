//
// Created by f0xeri on 23.01.2022.
//

#ifndef SLANGPARSER_AST_HPP
#define SLANGPARSER_AST_HPP

#include <string>
#include <utility>
#include <vector>
#include "TokenType.hpp"

enum E_TYPE {
    E_UNKNOWN = -1,
    E_VOID = 0,
    E_CONST,
    E_CHAR,
    E_INT,
    E_REAL,
    E_BOOLEAN,
    E_ARRAY,
    E_STRING,
    E_PTR,
    E_FUNC,
};

enum Operations
{
    Plus,
    Minus,
    Multiply,
    Divide,
};

class Node
{
public:

};

class ExprNode : public Node
{
public:
    virtual ~ExprNode() = default;
    E_TYPE _type = E_UNKNOWN;
};

class StatementNode : public Node
{
public:
    virtual ~StatementNode() = default;
};

class IntExprNode : public ExprNode
{
public:
    int value;
    explicit IntExprNode(int value): ExprNode(), value(value) {
        _type = E_INT;
    }
};

class RealExprNode : public ExprNode
{
public:
    double value;
    explicit RealExprNode(double value): ExprNode(), value(value) {
        _type = E_REAL;
    }
};

class CharExprNode : public ExprNode
{
public:
    char value;
    explicit CharExprNode(char value): ExprNode(), value(value) {
        _type = E_CHAR;
    }
};

class StringExprNode : public ExprNode
{
public:
    std::string value;
    explicit StringExprNode(std::string value): ExprNode(), value(std::move(value)) {
        _type = E_STRING;
    }
};

class ArrayExprNode : public ExprNode
{
public:
    std::vector<ArrayExprNode*> *values;
    explicit ArrayExprNode(std::vector<ArrayExprNode*> *values): values(values) {
        _type = E_ARRAY;
    }
};

class BooleanExprNode : public ExprNode
{
public:
    bool value;
    explicit BooleanExprNode(bool value): ExprNode(), value(value) {
        _type = E_BOOLEAN;
    }
};

class VariableExprNode: public ExprNode {
public:
    std::string value;

    explicit VariableExprNode(std::string name, E_TYPE type = E_UNKNOWN): value(std::move(name)) {
        _type = type;
    }
};

class UnaryOperatorExprNode: public ExprNode {
public:
    Operations op;
    ExprNode *right;

    UnaryOperatorExprNode(Operations op, ExprNode *right): right(right), ExprNode(), op(op) {}
};

class OperatorExprNode: public ExprNode {
public:
    Operations op;
    ExprNode *left, *right;

    OperatorExprNode(ExprNode *left, Operations op, ExprNode *right): left(left), right(right), op(op) {}
};

class ConditionalExprNode: public ExprNode {
public:
    TokenType op;
    ExprNode *left, *right;

    ConditionalExprNode(ExprNode *left, TokenType op, ExprNode *right): left(left), right(right), op(op) {}
};

class CallExprNode : public ExprNode {
public:
    VariableExprNode *name;
    std::vector<ExprNode*> *args;

    CallExprNode(VariableExprNode *name, std::vector<ExprNode*> *args) : name(name), args(args) {}
};

class BlockExprNode: public ExprNode {
public:
    std::vector<StatementNode*> *statements;

    BlockExprNode(): statements(new std::vector<StatementNode*>()) {}
    BlockExprNode(std::vector<StatementNode*> *statements): statements(statements) {}
};

class AssignExprNode: public ExprNode {
public:
    VariableExprNode *left;
    ExprNode *right;

    AssignExprNode(VariableExprNode *left, ExprNode *right): left(left), right(right) {}
};

class FuncExprNode: public ExprNode {
public:
    VariableExprNode *functor;
    std::vector<ExprNode*> *args;

    explicit FuncExprNode(VariableExprNode *functor): functor(functor), args(new std::vector<ExprNode*>()) {}
    FuncExprNode(VariableExprNode *functor, std::vector<ExprNode*> *args): functor(functor), args(args) {}
};

class CastExprNode: public ExprNode {
public:
    VariableExprNode *type;
    ExprNode *expr;

    CastExprNode(VariableExprNode *type, ExprNode *expr): type(type), expr(expr) {}
};

class ExprStatementNode : public StatementNode {
public:
    ExprNode *expr;

    explicit ExprStatementNode(ExprNode *expr): expr(expr) {}
};

class ReturnStatementNode : public StatementNode {
public:
    ExprNode *expr;

    explicit ReturnStatementNode(ExprNode *expr): expr(expr) {}
};

class DeclarationNode : public StatementNode
{
public:
    virtual ~DeclarationNode() = default;
    VariableExprNode *name;

    DeclarationNode(VariableExprNode *name) : name(name){};
};

class VarDecStatementNode : public DeclarationNode {
public:
    std::string type;
    ExprNode *expr;

    VarDecStatementNode(std::string type, VariableExprNode *name): type(std::move(type)), DeclarationNode(name), expr(nullptr) {}
    VarDecStatementNode(std::string type, VariableExprNode *name, ExprNode *expr): type(std::move(type)), DeclarationNode(name), expr(expr) {}
};

class ArrayDecStatementNode : public DeclarationNode {
public:
    std::string type;
    std::vector<ExprNode*> *init;
    long long size;
    bool isString;

    ArrayDecStatementNode(std::string type, VariableExprNode *name, long long size): type(std::move(type)), DeclarationNode(name), init(new std::vector<ExprNode*>()), size(size), isString(false) {}
    ArrayDecStatementNode(std::string type, VariableExprNode *name, std::vector<ExprNode*> *init): type(std::move(type)), DeclarationNode(name), init(init), size(init->size()), isString(false) {}
    ArrayDecStatementNode(std::string type, VariableExprNode *name, const std::string &str): type(std::move(type)), DeclarationNode(name), init(new std::vector<ExprNode*>()), isString(true) {
        for(auto it = str.begin(); it != str.end(); it++)
            init->push_back((ExprNode*)(new CharExprNode(*it)));
        init->push_back((ExprNode*)(new CharExprNode(0)));
        size = init->size() + 1;
    }
};

class IndexExprNode : public ExprNode {
public:
    VariableExprNode *name;
    ExprNode *expr;
    ExprNode *assign;
public:
    IndexExprNode(VariableExprNode *name, ExprNode *expr): name(name), expr(expr), assign(nullptr) {}
    IndexExprNode(VariableExprNode *name, ExprNode *expr, ExprNode *assign): name(name), expr(expr), assign(assign) {}
};

class FuncDecStatementNode : public DeclarationNode {
public:
    VariableExprNode *type;
    std::vector<VarDecStatementNode*> *args;
    BlockExprNode *block;

    FuncDecStatementNode(VariableExprNode *type, VariableExprNode *name, std::vector<VarDecStatementNode*> *args, BlockExprNode *block): type(type), DeclarationNode(name), args(args), block(block) {}
};

class FieldDecNode : public DeclarationNode
{
public:
    bool isPrivate;
    FieldDecNode(VariableExprNode *name, bool isPrivate) : DeclarationNode(name), isPrivate(isPrivate) {};
};

class FieldVarDecNode : public FieldDecNode
{
public:
    std::string type;
    ExprNode *expr;

    FieldVarDecNode(VariableExprNode *name, bool isPrivate, std::string type): type(std::move(type)), FieldDecNode(name, isPrivate), expr(nullptr) {};
    FieldVarDecNode(VariableExprNode *name, bool isPrivate, std::string type, ExprNode *expr): type(std::move(type)), FieldDecNode(name, isPrivate), expr(expr) {};
};

class FieldArrayVarDecNode : public FieldDecNode
{
public:
    ArrayDecStatementNode *var;
    FieldArrayVarDecNode(VariableExprNode *name, bool isPrivate, ArrayDecStatementNode *var) : FieldDecNode(name, isPrivate), var(var) {};
};

class MethodDecNode : public DeclarationNode
{
public:
    std::string type;
    VariableExprNode *thisName;
    std::vector<VarDecStatementNode*> *args = nullptr;
    BlockExprNode *block = nullptr;
    bool isPrivate;

    MethodDecNode(std::string type, VariableExprNode *name, bool isPrivate, VariableExprNode *thisName, std::vector<VarDecStatementNode*> *args, BlockExprNode *block):
            type(std::move(type)), DeclarationNode(name), isPrivate(isPrivate), thisName(thisName), args(args), block(block) {};
};

class TypeDecStatementNode : public DeclarationNode
{
public:
    //std::vector<FieldDecNode*> *fields = nullptr;
    //std::vector<MethodDecNode*> *methods = nullptr;
    //TypeDecStatementNode *parentType = nullptr;
    std::vector<FieldDecNode*> *fields = nullptr;
    std::vector<MethodDecNode*> *methods = nullptr;
    TypeDecStatementNode *parentType = nullptr;
    TypeDecStatementNode(VariableExprNode *name, std::vector<FieldDecNode*> *fields, std::vector<MethodDecNode*> *methods, TypeDecStatementNode *parentType = nullptr):
                         DeclarationNode(name), fields(fields), methods(methods), parentType(parentType) {};
};

class ExternFuncDecStatementNode : public DeclarationNode {
public:
    VariableExprNode *type;
    std::vector<VarDecStatementNode*> *args;

    ExternFuncDecStatementNode(VariableExprNode *type, VariableExprNode *name, std::vector<VarDecStatementNode*> *_args): type(type), DeclarationNode(name), args(_args) {
        std::vector<VarDecStatementNode*>::const_iterator it;
    }
};

class IfStatementNode : public StatementNode {
public:
    ExprNode *condExpr;
    BlockExprNode *trueBlock;
    BlockExprNode *falseBlock;

    IfStatementNode(ExprNode *condExpr, BlockExprNode *trueBlock): condExpr(condExpr), trueBlock(trueBlock), falseBlock(new BlockExprNode()) {}
    IfStatementNode(ExprNode *condExpr, BlockExprNode *trueBlock, BlockExprNode *falseBlock): condExpr(condExpr), trueBlock(trueBlock), falseBlock(falseBlock) {}
};

class ForStatementNode : public StatementNode {
public:
    ExprNode *initExpr;
    ExprNode *condExpr;
    ExprNode *loopExpr;
    BlockExprNode *block;

    ForStatementNode(ExprNode *initExpr, ExprNode *condExpr, ExprNode *loopExpr, BlockExprNode *block): initExpr(initExpr), condExpr(condExpr), loopExpr(loopExpr), block(block) {}
};

class WhileStatementNode : public StatementNode {
public:
    ExprNode *whileExpr;
    BlockExprNode *block;

    WhileStatementNode(ExprNode *whileExpr, BlockExprNode *block): whileExpr(whileExpr), block(block) {}
};

class ModuleStatementNode : public StatementNode
{
public:
    VariableExprNode *name;
    BlockExprNode *block;

    ModuleStatementNode(VariableExprNode *name, BlockExprNode *block): name(name), block(block) {}
};

class ImportStatementNode : public StatementNode
{
public:
    VariableExprNode *name;
    BlockExprNode *block;

    ImportStatementNode(VariableExprNode *name, BlockExprNode *block): name(name), block(block) {}
};


class AST
{

};


#endif //SLANGPARSER_AST_HPP
