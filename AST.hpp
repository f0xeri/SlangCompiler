//
// Created by f0xeri on 23.01.2022.
//

#ifndef SLANGPARSER_AST_HPP
#define SLANGPARSER_AST_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"

#include <llvm/IR/Value.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Pass.h>

#include "TokenType.hpp"

class CodeGenContext;

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

enum ParameterType
{
    In,
    Out,
    Var
};

class Node
{
public:
    virtual llvm::Value *codegen(CodeGenContext &cgcontext) = 0;
};

class ExprNode : public Node
{
public:
    virtual ~ExprNode() = default;
    E_TYPE _type = E_UNKNOWN;
    virtual llvm::Value *codegen(CodeGenContext &cgcontext) = 0;
};

class StatementNode : public Node
{
public:
    virtual ~StatementNode() = default;
    virtual llvm::Value *codegen(CodeGenContext &cgcontext) = 0;
};

class IntExprNode : public ExprNode
{
public:
    int value;
    explicit IntExprNode(int value): ExprNode(), value(value) {
        _type = E_INT;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class RealExprNode : public ExprNode
{
public:
    double value;
    explicit RealExprNode(double value): ExprNode(), value(value) {
        _type = E_REAL;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class CharExprNode : public ExprNode
{
public:
    char value;
    explicit CharExprNode(char value): ExprNode(), value(value) {
        _type = E_CHAR;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class StringExprNode : public ExprNode
{
public:
    std::string value;
    explicit StringExprNode(std::string value): ExprNode(), value(std::move(value)) {
        _type = E_STRING;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ArrayExprNode : public ExprNode
{
public:
    std::vector<ArrayExprNode*> *values;
    explicit ArrayExprNode(std::vector<ArrayExprNode*> *values): values(values) {
        _type = E_ARRAY;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class BooleanExprNode : public ExprNode
{
public:
    bool value;
    explicit BooleanExprNode(bool value): ExprNode(), value(value) {
        _type = E_BOOLEAN;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class VariableExprNode: public ExprNode, public StatementNode {
public:
    std::string value;

    explicit VariableExprNode(std::string name, E_TYPE type = E_UNKNOWN): value(std::move(name)) {
        _type = type;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class UnaryOperatorExprNode: public ExprNode {
public:
    TokenType op;
    ExprNode *right;

    UnaryOperatorExprNode(TokenType op, ExprNode *right): right(right), ExprNode(), op(op) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class OperatorExprNode: public ExprNode {
public:
    TokenType op;
    ExprNode *left, *right;

    OperatorExprNode(ExprNode *left, TokenType op, ExprNode *right): left(left), right(right), op(op) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ConditionalExprNode: public ExprNode {
public:
    TokenType op;
    ExprNode *left, *right;

    ConditionalExprNode(ExprNode *left, TokenType op, ExprNode *right): left(left), right(right), op(op) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class CallExprNode : public ExprNode, public StatementNode {
public:
    VariableExprNode *name;
    std::vector<ExprNode*> *args;

    CallExprNode(VariableExprNode *name, std::vector<ExprNode*> *args) : name(name), args(args) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class BlockExprNode: public ExprNode {
public:
    std::vector<StatementNode*> *statements;

    BlockExprNode(): statements(new std::vector<StatementNode*>()) {}
    BlockExprNode(std::vector<StatementNode*> *statements): statements(statements) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class AssignExprNode: public StatementNode {
public:
    VariableExprNode *left;
    ExprNode *right;

    AssignExprNode(VariableExprNode *left, ExprNode *right): left(left), right(right) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FuncExprNode: public ExprNode {
public:
    VariableExprNode *functor;
    std::vector<ExprNode*> *args;

    explicit FuncExprNode(VariableExprNode *functor): functor(functor), args(new std::vector<ExprNode*>()) {}
    FuncExprNode(VariableExprNode *functor, std::vector<ExprNode*> *args): functor(functor), args(args) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class CastExprNode: public ExprNode {
public:
    VariableExprNode *type;
    ExprNode *expr;

    CastExprNode(VariableExprNode *type, ExprNode *expr): type(type), expr(expr) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ExprStatementNode : public StatementNode {
public:
    ExprNode *expr;

    explicit ExprStatementNode(ExprNode *expr): expr(expr) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ReturnStatementNode : public StatementNode {
public:
    ExprNode *expr;

    explicit ReturnStatementNode(ExprNode *expr): expr(expr) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class OutputStatementNode : public StatementNode {
public:
    ExprNode *expr;

    explicit OutputStatementNode(ExprNode *expr): expr(expr) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class DeclarationNode : public StatementNode
{
public:
    virtual ~DeclarationNode() = default;
    VariableExprNode *name;

    DeclarationNode(VariableExprNode *name) : name(name){};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class VarDecStatementNode : public DeclarationNode {
public:
    std::string type;
    ExprNode *expr;
    bool isGlobal = false;

    VarDecStatementNode(std::string type, VariableExprNode *name): type(std::move(type)), DeclarationNode(name), expr(nullptr) {}
    VarDecStatementNode(std::string type, VariableExprNode *name, ExprNode *expr, bool isGlobal = false): type(std::move(type)), DeclarationNode(name), expr(expr), isGlobal(isGlobal) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FuncParamDecStatementNode : public DeclarationNode {
public:
    ParameterType parameterType;
    std::string type;
    ExprNode *expr;

    FuncParamDecStatementNode(std::string type, VariableExprNode *name, ParameterType parameterType): type(std::move(type)), DeclarationNode(name), parameterType(parameterType), expr(nullptr) {}
    FuncParamDecStatementNode(std::string type, VariableExprNode *name, ParameterType parameterType, ExprNode *expr): type(std::move(type)), DeclarationNode(name), parameterType(parameterType), expr(expr) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
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
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class IndexExprNode : public ExprNode {
public:
    VariableExprNode *name;
    ExprNode *expr;
    ExprNode *assign;
public:
    IndexExprNode(VariableExprNode *name, ExprNode *expr): name(name), expr(expr), assign(nullptr) {}
    IndexExprNode(VariableExprNode *name, ExprNode *expr, ExprNode *assign): name(name), expr(expr), assign(assign) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FuncDecStatementNode : public DeclarationNode {
public:
    std::string type;
    std::vector<FuncParamDecStatementNode*> *args = nullptr;
    BlockExprNode *block = nullptr;
    bool isPrivate = false;
    bool isFunction = false;

    FuncDecStatementNode(std::string type, VariableExprNode *name, bool isPrivate, bool isFunction, std::vector<FuncParamDecStatementNode*> *args, BlockExprNode *block): type(std::move(type)), DeclarationNode(name), isPrivate(isPrivate), isFunction(isFunction), args(args), block(block) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FieldDecNode : public DeclarationNode
{
public:
    bool isPrivate;
    FieldDecNode(VariableExprNode *name, bool isPrivate) : DeclarationNode(name), isPrivate(isPrivate) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FieldVarDecNode : public FieldDecNode
{
public:
    std::string type;
    ExprNode *expr;

    FieldVarDecNode(VariableExprNode *name, bool isPrivate, std::string type): type(std::move(type)), FieldDecNode(name, isPrivate), expr(nullptr) {};
    FieldVarDecNode(VariableExprNode *name, bool isPrivate, std::string type, ExprNode *expr): type(std::move(type)), FieldDecNode(name, isPrivate), expr(expr) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FieldArrayVarDecNode : public FieldDecNode
{
public:
    ArrayDecStatementNode *var;
    FieldArrayVarDecNode(VariableExprNode *name, bool isPrivate, ArrayDecStatementNode *var) : FieldDecNode(name, isPrivate), var(var) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class MethodDecNode : public DeclarationNode
{
public:
    std::string type;
    VariableExprNode *thisName;
    std::vector<FuncParamDecStatementNode*> *args = nullptr;
    BlockExprNode *block = nullptr;
    bool isPrivate = false;

    MethodDecNode(std::string type, VariableExprNode *name, bool isPrivate, VariableExprNode *thisName, std::vector<FuncParamDecStatementNode*> *args, BlockExprNode *block):
            type(std::move(type)), DeclarationNode(name), isPrivate(isPrivate), thisName(thisName), args(args), block(block) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class TypeDecStatementNode : public DeclarationNode
{
public:
    std::vector<FieldVarDecNode*> *fields = nullptr;
    std::vector<MethodDecNode*> *methods = nullptr;
    TypeDecStatementNode *parentType = nullptr;
    bool isPrivate = false;
    TypeDecStatementNode(VariableExprNode *name, bool isPrivate, std::vector<FieldVarDecNode*> *fields, std::vector<MethodDecNode*> *methods, TypeDecStatementNode *parentType = nullptr):
                         DeclarationNode(name), isPrivate(isPrivate), fields(fields), methods(methods), parentType(parentType) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ExternFuncDecStatementNode : public DeclarationNode {
public:
    VariableExprNode *type;
    std::vector<VarDecStatementNode*> *args;

    ExternFuncDecStatementNode(VariableExprNode *type, VariableExprNode *name, std::vector<VarDecStatementNode*> *_args): type(type), DeclarationNode(name), args(_args) {
        std::vector<VarDecStatementNode*>::const_iterator it;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ElseIfStatementNode : public StatementNode {
public:
    ExprNode *condExpr;
    BlockExprNode *trueBlock;

    ElseIfStatementNode(ExprNode *condExpr, BlockExprNode *trueBlock): condExpr(condExpr), trueBlock(trueBlock) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class IfStatementNode : public StatementNode {
public:
    ExprNode *condExpr;
    BlockExprNode *trueBlock;
    BlockExprNode *falseBlock;
    std::vector<ElseIfStatementNode*> *elseifNodes = nullptr;

    IfStatementNode(ExprNode *condExpr, BlockExprNode *trueBlock): condExpr(condExpr), trueBlock(trueBlock), falseBlock(new BlockExprNode()) {}
    IfStatementNode(ExprNode *condExpr, BlockExprNode *trueBlock, BlockExprNode *falseBlock): condExpr(condExpr), trueBlock(trueBlock), falseBlock(falseBlock) {}
    IfStatementNode(ExprNode *condExpr, BlockExprNode *trueBlock, std::vector<ElseIfStatementNode*> *elseifNodes, BlockExprNode *falseBlock): condExpr(condExpr), trueBlock(trueBlock), elseifNodes(elseifNodes), falseBlock(falseBlock) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ForStatementNode : public StatementNode {
public:
    ExprNode *initExpr;
    ExprNode *condExpr;
    ExprNode *loopExpr;
    BlockExprNode *block;

    ForStatementNode(ExprNode *initExpr, ExprNode *condExpr, ExprNode *loopExpr, BlockExprNode *block): initExpr(initExpr), condExpr(condExpr), loopExpr(loopExpr), block(block) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class WhileStatementNode : public StatementNode {
public:
    ExprNode *whileExpr;
    BlockExprNode *block;

    WhileStatementNode(ExprNode *whileExpr, BlockExprNode *block): whileExpr(whileExpr), block(block) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ModuleStatementNode : public StatementNode
{
public:
    VariableExprNode *name;
    BlockExprNode *block;

    ModuleStatementNode(VariableExprNode *name, BlockExprNode *block): name(name), block(block) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ImportStatementNode : public StatementNode
{
public:
    VariableExprNode *name;
    BlockExprNode *block;

    ImportStatementNode(VariableExprNode *name, BlockExprNode *block): name(name), block(block) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

using namespace llvm;
using namespace std;


class CodeGenBlock {
public:
    llvm::BasicBlock* block;
    std::map <std::string, llvm::Value*> locals;
};

class CodeGenContext {

    std::map <std::string, llvm::Value*> globalVariables;
    llvm::Function* mMainFunction;
public:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    llvm::Module* mModule;
    std::vector<CodeGenBlock*> blocks;
    std::map<std::string, StructType *> allocatedClasses;
    FuncDecStatementNode *currentFunc = nullptr;
    CodeGenContext()
    {
        context = make_unique<llvm::LLVMContext>();
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
        mModule = new Module("main", *context);
        mModule->setTargetTriple("x86_64-w64-windows-gnu");
    }
    ~CodeGenContext() = default;


    Function* printfFunction() {
        vector<Type*> printfArgs;
        printfArgs.push_back(Type::getInt8PtrTy(*context));
        FunctionType* printfType = FunctionType::get(Type::getInt32Ty(*context), printfArgs, true);
        Function *printfFunc = Function::Create(printfType, Function::ExternalLinkage, Twine("printf"), mModule);
        printfFunc->setCallingConv(CallingConv::C);
        return printfFunc;
    }

    Function* llvmTrap() {
        vector<Type*> agrs;
        FunctionType* type = FunctionType::get(Type::getVoidTy(*context), agrs, false);
        Function *llvmTrap = Function::Create(type, Function::ExternalLinkage, Twine("llvm.trap"), mModule);
        llvmTrap->setCallingConv(CallingConv::C);
        return llvmTrap;
    }

    void generateCode(ModuleStatementNode *mainModule, const std::vector<std::pair<std::string, DeclarationNode*>>& symbols)
    {
        printfFunction();
        llvmTrap();
        for (auto g : symbols)
        {
            g.second->codegen(*this);
        }
        FunctionType *mainType = FunctionType::get(builder->getInt32Ty(), false);
        mMainFunction = Function::Create(mainType, Function::ExternalLinkage, "main",
                                          mModule);
        BasicBlock *entry = BasicBlock::Create(*context, "entry", mMainFunction);
        builder->SetInsertPoint(entry);
        pushBlock(entry);

        for (auto decl : *mainModule->block->statements)
        {
            decl->codegen(*this);
        }

        auto var = builder->CreateRet(ConstantInt::get(*context, APInt(32, 0)));
        popBlock();
    }

    std::map <std::string, llvm::Value*>& locals() {
        return blocks.back()->locals;
    }

    llvm::Value* localsLookup(const std::string &name) {
        Value *value = nullptr;
        for (auto &b : blocks)
        {
            if (b->locals.contains(name))
            {
                value = b->locals[name];
                break;
            }
        }
        return value;
    }

    std::map<string, Value*>& globals() {
        return globalVariables;
    }

    llvm::BasicBlock* currentBlock() {
        return blocks.back()->block;
    }

    void ret(BasicBlock* block) { blocks.back()->block = block; }

    void pushBlock(llvm::BasicBlock* block) {
        blocks.push_back(new CodeGenBlock());
        blocks.back()->block = block;
    }

    void popBlock() {
        CodeGenBlock* top = blocks.back();
        blocks.pop_back();
        delete top;
    }
};

#endif //SLANGPARSER_AST_HPP
