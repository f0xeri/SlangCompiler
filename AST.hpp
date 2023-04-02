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
#include "llvm/Transforms/Utils/ModuleUtils.h"

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
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/IR/DIBuilder.h>

#include "TokenType.hpp"

struct SourceLoc {
    uint64_t line;
    uint64_t col;
};

class CodeGenContext;

enum E_TYPE {
    E_UNKNOWN = -1,
    E_VOID = 0,
    E_CONST,
    E_CHAR,
    E_INT,
    E_REAL,
    E_FLOAT,
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
    SourceLoc loc{};
    Node(SourceLoc loc) : loc(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext) = 0;
};

class ExprNode : public Node
{
public:
    bool isConst = false;
    ExprNode(bool isConst, SourceLoc loc): isConst(isConst), Node(loc) {}
    virtual ~ExprNode() = default;
    E_TYPE _type = E_UNKNOWN;
    virtual llvm::Value *codegen(CodeGenContext &cgcontext) = 0;
};

class StatementNode : public Node
{
public:
    StatementNode(SourceLoc loc) : Node(loc) {}
    virtual ~StatementNode() = default;
    virtual llvm::Value *codegen(CodeGenContext &cgcontext) = 0;
};

class IntExprNode : public ExprNode
{
public:
    int value;
    explicit IntExprNode(SourceLoc loc, int value): ExprNode(true, loc), value(value) {
        _type = E_INT;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class RealExprNode : public ExprNode
{
public:
    double value;
    explicit RealExprNode(SourceLoc loc, double value): ExprNode(true, loc), value(value) {
        _type = E_REAL;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FloatExprNode : public ExprNode
{
public:
    double value;
    explicit FloatExprNode(SourceLoc loc, double value): ExprNode(true, loc), value(value) {
        _type = E_FLOAT;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class CharExprNode : public ExprNode
{
public:
    char value;
    explicit CharExprNode(SourceLoc loc, char value): ExprNode(true, loc), value(value) {
        _type = E_CHAR;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class StringExprNode : public ExprNode
{
public:
    std::string value;
    explicit StringExprNode(SourceLoc loc, std::string value): ExprNode(true, loc), value(std::move(value)) {
        _type = E_STRING;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class NilExprNode : public ExprNode
{
public:
    ExprNode* type;
    explicit NilExprNode(SourceLoc loc, ExprNode* type = nullptr): ExprNode(true, loc), type(type) {
        _type = E_UNKNOWN;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ArrayExprNode : public ExprNode
{
public:
    std::vector<ExprNode*> *values;
    std::string type;
    ExprNode* size;
    explicit ArrayExprNode(SourceLoc loc, const std::string &type, ExprNode* size, std::vector<ExprNode*> *values): type(type), size(size), values(values), ExprNode(false, loc) {
        _type = E_ARRAY;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class BooleanExprNode : public ExprNode
{
public:
    bool value;
    explicit BooleanExprNode(SourceLoc loc, bool value): ExprNode(true, loc), value(value) {
        _type = E_BOOLEAN;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class VariableExprNode: public ExprNode, public StatementNode {
public:
    std::string value;
    bool dotClass = false;
    bool dotModule = false;
    bool isPointer = false;
    int index = -1;
    VariableExprNode(SourceLoc loc, std::string name, E_TYPE type = E_UNKNOWN, bool dotModule = false, bool dotClass = false, int index = -1, bool isPointer = false): value(std::move(name)), ExprNode(false, loc), StatementNode(loc), dotModule(dotModule), dotClass(dotClass), index(index), isPointer(isPointer) {
        _type = type;
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class IndexExprNode : public VariableExprNode  {
public:
    ExprNode *indexExpr;
    ExprNode *assign;
public:
    IndexExprNode(SourceLoc loc, const std::string &name, ExprNode *expr, bool dotModule = false, bool dotClass = false, int index = -1, bool isPointer = false): VariableExprNode(loc, name, E_UNKNOWN, dotModule, dotClass, index, isPointer), indexExpr(expr), assign(nullptr) {}
    IndexExprNode(SourceLoc loc, const std::string &name, ExprNode *expr, ExprNode *assign, bool dotModule = false, bool dotClass = false, int index = -1, bool isPointer = false): VariableExprNode(loc, name, E_UNKNOWN, dotModule, dotClass, index, isPointer), indexExpr(expr), assign(assign) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class IndexesExprNode : public VariableExprNode  {
public:
    std::vector<ExprNode*> *indexes;
    ExprNode *assign;
public:
    IndexesExprNode(SourceLoc loc, const std::string &name, std::vector<ExprNode*> *indexes, bool dotModule = false, bool dotClass = false, int index = -1, bool isPointer = false): VariableExprNode(loc, name, E_UNKNOWN, dotModule, dotClass, index, isPointer), indexes(indexes), assign(nullptr) {}
    IndexesExprNode(SourceLoc loc, const std::string &name, std::vector<ExprNode*> *indexes, ExprNode *assign, bool dotModule = false, bool dotClass = false, int index = -1, bool isPointer = false): VariableExprNode(loc, name, E_UNKNOWN, dotModule, dotClass, index, isPointer), indexes(indexes), assign(assign) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class UnaryOperatorExprNode: public ExprNode {
public:
    TokenType op;
    ExprNode *right;

    UnaryOperatorExprNode(SourceLoc loc, TokenType op, ExprNode *right): right(right), ExprNode(false, loc), op(op) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class OperatorExprNode: public ExprNode {
public:
    TokenType op;
    ExprNode *left, *right;

    OperatorExprNode(SourceLoc loc, ExprNode *left, TokenType op, ExprNode *right): ExprNode(false, loc), left(left), right(right), op(op) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ConditionalExprNode: public ExprNode {
public:
    TokenType op;
    ExprNode *left, *right;

    ConditionalExprNode(SourceLoc loc, ExprNode *left, TokenType op, ExprNode *right): ExprNode(false, loc), left(left), right(right), op(op) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class CallExprNode : public ExprNode, public StatementNode {
public:
    VariableExprNode *name;
    std::vector<ExprNode*> *args;

    CallExprNode(SourceLoc loc, VariableExprNode *name, std::vector<ExprNode*> *args) : ExprNode(false, loc), name(name), args(args), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class DeleteExprNode : public StatementNode {
public:
    ExprNode* expr;
    DeleteExprNode(SourceLoc loc, ExprNode *expr) : expr(expr), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class BlockExprNode: public ExprNode {
public:
    std::vector<StatementNode*> *statements;

    BlockExprNode(SourceLoc loc): ExprNode(false, loc), statements(new std::vector<StatementNode*>()) {}
    BlockExprNode(SourceLoc loc, std::vector<StatementNode*> *statements): ExprNode(false, loc), statements(statements) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class AssignExprNode: public StatementNode {
public:
    VariableExprNode *left;
    ExprNode *right;

    AssignExprNode(SourceLoc loc, VariableExprNode *left, ExprNode *right): left(left), right(right), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class DeclarationNode : public StatementNode
{
public:
    virtual ~DeclarationNode() = default;
    VariableExprNode *name;

    DeclarationNode(SourceLoc loc, VariableExprNode *name) : name(name), StatementNode(loc) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FuncParamDecStatementNode : public DeclarationNode {
public:
    ParameterType parameterType;
    ExprNode *type;
    ExprNode *expr;

    FuncParamDecStatementNode(SourceLoc loc, ExprNode* type, VariableExprNode *name, ParameterType parameterType): type(type), DeclarationNode(loc, name), parameterType(parameterType), expr(nullptr) {}
    FuncParamDecStatementNode(SourceLoc loc, ExprNode* type, VariableExprNode *name, ParameterType parameterType, ExprNode *expr): type(type), DeclarationNode(loc, name), parameterType(parameterType), expr(expr) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FuncExprNode: public ExprNode {
public:
    ExprNode* type;
    bool isFunction = true;
    std::vector<FuncParamDecStatementNode*> *args;

    FuncExprNode(SourceLoc loc, ExprNode* type, std::vector<FuncParamDecStatementNode*> *args, bool isFunction): ExprNode(false, loc), type(type), args(args), isFunction(isFunction) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class CastExprNode: public ExprNode {
public:
    VariableExprNode *type;
    ExprNode *expr;

    CastExprNode(SourceLoc loc, VariableExprNode *type, ExprNode *expr): ExprNode(false, loc), type(type), expr(expr) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ExprStatementNode : public StatementNode {
public:
    ExprNode *expr;

    explicit ExprStatementNode(SourceLoc loc, ExprNode *expr): expr(expr), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ReturnStatementNode : public StatementNode {
public:
    ExprNode *expr;

    explicit ReturnStatementNode(SourceLoc loc, ExprNode *expr): expr(expr), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class OutputStatementNode : public StatementNode {
public:
    ExprNode *expr;

    explicit OutputStatementNode(SourceLoc loc, ExprNode *expr): expr(expr), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class InputStatementNode : public StatementNode {
public:
    ExprNode *expr;

    explicit InputStatementNode(SourceLoc loc, ExprNode *expr): expr(expr), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class VarDecStatementNode : public DeclarationNode {
public:
    std::string type;
    ExprNode *expr;
    bool isGlobal = false;
    bool isPrivate = false;
    bool isExtern = false;

    VarDecStatementNode(SourceLoc loc, std::string type, VariableExprNode *name): type(std::move(type)), DeclarationNode(loc, name), expr(nullptr) {}
    VarDecStatementNode(SourceLoc loc, std::string type, VariableExprNode *name, ExprNode *expr, bool isGlobal = false, bool isPrivate = false, bool isExtern = false): type(std::move(type)), DeclarationNode(loc, name), expr(expr), isGlobal(isGlobal), isPrivate(isPrivate), isExtern(isExtern) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FuncPointerStatementNode : public DeclarationNode {
public:
    ExprNode* type;
    std::vector<FuncParamDecStatementNode*> *args = nullptr;
    bool isFunction = false;
    bool isGlobal = false;
    bool isPrivate = false;
    bool isExtern = false;
    ExprNode* expr;

    FuncPointerStatementNode(SourceLoc loc, ExprNode*  type, VariableExprNode *name, bool isFunction, bool isGlobal, std::vector<FuncParamDecStatementNode*> *args, ExprNode* expr, bool isPrivate = false, bool isExtern = false): type(type), DeclarationNode(loc, name), isFunction(isFunction), isGlobal(isGlobal), args(args), expr(expr), isPrivate(isPrivate), isExtern(isExtern) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ArrayDecStatementNode : public DeclarationNode {
public:
    /*std::string type;
    std::vector<ExprNode*> *init;
    ExprNode *size;
    bool isString;*/

    bool isGlobal = false;
    bool isPrivate = false;
    ArrayExprNode* expr;
    int indicesCount = 1;
    bool isExtern = false;
    ArrayDecStatementNode(SourceLoc loc, VariableExprNode *name, ArrayExprNode *expr, bool isGlobal, int indicesCount = 1, bool isPrivate = false, bool isExtern = false) : DeclarationNode(loc, name), expr(expr), isGlobal(isGlobal), indicesCount(indicesCount), isPrivate(isPrivate), isExtern(isExtern) {}

    /*ArrayDecStatementNode(std::string type, VariableExprNode *value, ExprNode *size): type(std::move(type)), DeclarationNode(value), init(new std::vector<ExprNode*>()), size(size), isString(false) {}
    ArrayDecStatementNode(std::string type, VariableExprNode *value, std::vector<ExprNode*> *init): type(std::move(type)), DeclarationNode(value), init(init), size(new IntExprNode(init->size())), isString(false) {}
    ArrayDecStatementNode(std::string type, VariableExprNode *value, const std::string &str): type(std::move(type)), DeclarationNode(value), init(new std::vector<ExprNode*>()), isString(true) {
        for(auto it = str.begin(); it != str.end(); it++)
            init->push_back((ExprNode*)(new CharExprNode(*it)));
        init->push_back((ExprNode*)(new CharExprNode(0)));
        size = init->size() + 1;
    }*/
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FuncDecStatementNode : public DeclarationNode {
public:
    ExprNode* type;
    std::vector<FuncParamDecStatementNode*> *args = nullptr;
    BlockExprNode *block = nullptr;
    bool isPrivate = false;
    bool isFunction = false;

    FuncDecStatementNode(SourceLoc loc, ExprNode*  type, VariableExprNode *name, bool isPrivate, bool isFunction, std::vector<FuncParamDecStatementNode*> *args, BlockExprNode *block): type(type), DeclarationNode(loc, name), isPrivate(isPrivate), isFunction(isFunction), args(args), block(block) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FieldVarDecNode : public DeclarationNode
{
public:
    std::string typeName;
    bool isPrivate;
    int index;

    std::string type;
    ExprNode *expr;

    FieldVarDecNode(SourceLoc loc, const std::string &typeName, VariableExprNode *name, bool isPrivate, std::string type, int index): typeName(typeName), DeclarationNode(loc, name), type(std::move(type)), expr(nullptr), isPrivate(isPrivate), index(index) {};
    FieldVarDecNode(SourceLoc loc, const std::string &typeName, VariableExprNode *name, bool isPrivate, std::string type, ExprNode *expr, int index): typeName(typeName),  DeclarationNode(loc, name), type(std::move(type)), isPrivate(isPrivate), expr(expr), index(index) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class FieldArrayVarDecNode : public DeclarationNode
{
public:
    std::string typeName;
    bool isPrivate;
    int index;

    ArrayDecStatementNode *var;

    FieldArrayVarDecNode(SourceLoc loc, const std::string &typeName, VariableExprNode *name, bool isPrivate, ArrayDecStatementNode *var, int index): typeName(typeName),  DeclarationNode(loc, name), isPrivate(isPrivate), var(var), index(index) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class MethodDecNode : public DeclarationNode
{
public:
    ExprNode* type;
    VariableExprNode *thisName;
    std::vector<FuncParamDecStatementNode*> *args = nullptr;
    BlockExprNode *block = nullptr;
    bool isPrivate = false;
    bool isFunction = false;
    MethodDecNode(SourceLoc loc, ExprNode* type, VariableExprNode *name, bool isPrivate, bool isFunction, VariableExprNode *thisName, std::vector<FuncParamDecStatementNode*> *args, BlockExprNode *block):
            type(type), DeclarationNode(loc, name), isPrivate(isPrivate), isFunction(isFunction), thisName(thisName), args(args), block(block) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class TypeDecStatementNode : public DeclarationNode
{
public:
    std::vector<DeclarationNode*> *fields = nullptr;
    std::vector<MethodDecNode*> *methods = nullptr;
    TypeDecStatementNode *parentType = nullptr;
    bool isExtern;
    bool isPrivate = false;
    TypeDecStatementNode(SourceLoc loc, VariableExprNode *name, bool isPrivate, std::vector<DeclarationNode*> *fields, std::vector<MethodDecNode*> *methods, bool isExtern, TypeDecStatementNode *parentType = nullptr):
                         DeclarationNode(loc, name), isPrivate(isPrivate), fields(fields), methods(methods), isExtern(isExtern), parentType(parentType) {};
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ExternFuncDecStatementNode : public DeclarationNode {
public:
    ExprNode* type;
    std::vector<FuncParamDecStatementNode*> *args = nullptr;
    BlockExprNode *block = nullptr;
    bool isPrivate = false;
    bool isFunction = false;
    ExternFuncDecStatementNode(SourceLoc loc, ExprNode*  type, VariableExprNode *name, bool isPrivate, bool isFunction, std::vector<FuncParamDecStatementNode*> *args, BlockExprNode *block): type(type), DeclarationNode(loc, name), isPrivate(isPrivate), isFunction(isFunction), args(args), block(block) {
    }
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ElseIfStatementNode : public StatementNode {
public:
    ExprNode *condExpr;
    BlockExprNode *trueBlock;

    ElseIfStatementNode(SourceLoc loc, ExprNode *condExpr, BlockExprNode *trueBlock): condExpr(condExpr), trueBlock(trueBlock),
                                                                                      StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class IfStatementNode : public StatementNode {
public:
    ExprNode *condExpr;
    BlockExprNode *trueBlock;
    BlockExprNode *falseBlock;
    std::vector<ElseIfStatementNode*> *elseifNodes = nullptr;

    IfStatementNode(SourceLoc loc, ExprNode *condExpr, BlockExprNode *trueBlock): condExpr(condExpr), trueBlock(trueBlock), falseBlock(new BlockExprNode(loc)),
                                                                                  StatementNode(loc) {}
    IfStatementNode(SourceLoc loc, ExprNode *condExpr, BlockExprNode *trueBlock, BlockExprNode *falseBlock): condExpr(condExpr), trueBlock(trueBlock), falseBlock(falseBlock),
                                                                                                             StatementNode(loc) {}
    IfStatementNode(SourceLoc loc, ExprNode *condExpr, BlockExprNode *trueBlock, std::vector<ElseIfStatementNode*> *elseifNodes, BlockExprNode *falseBlock): condExpr(condExpr), trueBlock(trueBlock), elseifNodes(elseifNodes), falseBlock(falseBlock),
                                                                                                                                                             StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ForStatementNode : public StatementNode {
public:
    ExprNode *initExpr;
    ExprNode *condExpr;
    ExprNode *loopExpr;
    BlockExprNode *block;

    ForStatementNode(SourceLoc loc, ExprNode *initExpr, ExprNode *condExpr, ExprNode *loopExpr, BlockExprNode *block): initExpr(initExpr), condExpr(condExpr), loopExpr(loopExpr), block(block), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class WhileStatementNode : public StatementNode {
public:
    ExprNode *whileExpr;
    BlockExprNode *block;

    WhileStatementNode(SourceLoc loc, ExprNode *whileExpr, BlockExprNode *block): whileExpr(whileExpr), block(block), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ModuleStatementNode : public StatementNode
{
public:
    VariableExprNode *name;
    BlockExprNode *block;

    ModuleStatementNode(SourceLoc loc, VariableExprNode *name, BlockExprNode *block): name(name), block(block), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

class ImportStatementNode : public StatementNode
{
public:
    VariableExprNode *name;
    BlockExprNode *block;

    ImportStatementNode(SourceLoc loc, VariableExprNode *name, BlockExprNode *block): name(name), block(block), StatementNode(loc) {}
    virtual llvm::Value *codegen(CodeGenContext &cgcontext);
};

using namespace llvm;
using namespace std;

struct DebugInfo {
    DICompileUnit* compileUnit;
    DIType* integerType = nullptr;
    DIType* realType = nullptr;
    DIType* floatType = nullptr;
    DIType* characterType = nullptr;
    DIType* booleanType = nullptr;
    std::vector<DIScope*> lexicalBlocks;
    std::map<std::string, DICompositeType*> dbgClasses;
    DIBuilder *debugBuilder;

    DIType *getIntegerTy() {
        if (integerType) return integerType;
        integerType = debugBuilder->createBasicType("integer", 32, dwarf::DW_ATE_signed);
        return integerType;
    };
    DIType *getRealTy() {
        if (realType) return realType;
        realType = debugBuilder->createBasicType("real", 64, dwarf::DW_ATE_float);
        return realType;
    };
    DIType *getFloatType() {
        if (floatType) return floatType;
        floatType = debugBuilder->createBasicType("float", 32, dwarf::DW_ATE_float);
        return floatType;
    };
    DIType *getCharacterType() {
        if (characterType) return characterType;
        characterType = debugBuilder->createBasicType("character", 8, dwarf::DW_ATE_signed_char);
        return characterType;
    };
    DIType *getBooleanType() {
        if (booleanType) return booleanType;
        booleanType = debugBuilder->createBasicType("boolean", 1, dwarf::DW_ATE_boolean);
        return booleanType;
    };

    DIType *getVoidType() {
        if (booleanType) return booleanType;

        return booleanType;
    };

    DISubroutineType* CreateFunctionType(const std::vector<Metadata*>& retAndArgTypes) {
        DITypeRefArray params = debugBuilder->getOrCreateTypeArray(makeArrayRef(retAndArgTypes));
        DISubroutineType* dbgFuncType = debugBuilder->createSubroutineType(params);
        return dbgFuncType;
    };

    DIDerivedType* createPointerType(DIType* type, const std::string& typeName) {
        return debugBuilder->createPointerType(type, type->getSizeInBits(), 0, None, typeName);
    }

    void emitLocation(IRBuilder<> *irBuilder, Node *AST) {
        if (!AST)
            return irBuilder->SetCurrentDebugLocation(DebugLoc());
        DIScope *Scope;
        if (lexicalBlocks.empty())
            Scope = compileUnit;
        else
            Scope = lexicalBlocks.back();
        irBuilder->SetCurrentDebugLocation(
                DILocation::get(Scope->getContext(), AST->loc.line, AST->loc.col, Scope));
    };

    void emitLocation(IRBuilder<> *irBuilder) {
        irBuilder->SetCurrentDebugLocation(DebugLoc());
    };
};

class CodeGenBlock {
public:
    llvm::BasicBlock* block;
    std::map <std::string, llvm::Value*> locals;
    std::map <std::string, DeclarationNode*> localsExprs;
};

class CodeGenContext {
    std::map <std::string, llvm::Value*> globalVariables;
    llvm::Function* mMainFunction;
public:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::DataLayout> dataLayout;

    std::unique_ptr<llvm::DIBuilder> debugBuilder;

    std::unique_ptr<std::vector<std::pair<std::string, DeclarationNode*>>> symbols;
    DebugInfo dbgInfo;
    llvm::Module* mModule;
    std::vector<CodeGenBlock*> blocks;
    std::map<std::string, StructType *> allocatedClasses;
    FuncDecStatementNode *currentFunc = nullptr;
    bool isFuncPointerAssignment = false;
    std::string currentNameAddition;
    bool isMainModule = false;
    std::string moduleName;
    bool callingExpr = false;

    LoadInst* currentTypeLoad;

    CodeGenContext(ModuleStatementNode *moduleStatement, bool isMainModule, const std::vector<std::pair<std::string, DeclarationNode*>>& symbols)
    {
        context = std::make_unique<llvm::LLVMContext>();
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
        mModule = new Module(moduleStatement->name->value, *context);
        mModule->setTargetTriple("x86_64-w64-windows-gnu");
        mModule->addModuleFlag(Module::Warning, "Debug Info Version", DEBUG_METADATA_VERSION);
        dataLayout = std::make_unique<llvm::DataLayout>(mModule);
        this->isMainModule = isMainModule;
        moduleName = moduleStatement->name->value;
        this->symbols = std::make_unique<std::vector<std::pair<std::string, DeclarationNode*>>>(symbols);
        debugBuilder = std::make_unique<DIBuilder>(*mModule);
        dbgInfo.compileUnit = debugBuilder->createCompileUnit(dwarf::DW_LANG_C, debugBuilder->createFile(moduleName + ".sl", ""), "Slangc", false, "", 0);
        dbgInfo.debugBuilder = debugBuilder.get();
        dbgInfo.getIntegerTy();
        dbgInfo.getRealTy();
        dbgInfo.getFloatType();
        dbgInfo.getCharacterType();
        dbgInfo.getBooleanType();
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

    Function* scanfFunction() {
        vector<Type*> scanfArgs;
        scanfArgs.push_back(Type::getInt8PtrTy(*context));
        FunctionType* scanfType = FunctionType::get(Type::getInt32Ty(*context), scanfArgs, true);
        Function *scanfFunc = Function::Create(scanfType, Function::ExternalLinkage, Twine("scanf"), mModule);
        scanfFunc->setCallingConv(CallingConv::C);
        return scanfFunc;
    }

    Function* GC_InitFunction() {
        vector<Type*> args;
        FunctionType* type = FunctionType::get(Type::getVoidTy(*context), args, false);
        Function *GC_InitFunc = Function::Create(type, Function::ExternalLinkage, Twine("GC_init"), mModule);
        GC_InitFunc->setCallingConv(CallingConv::C);
        return GC_InitFunc;
    }

    Function* GC_MallocFunction() {
        vector<Type*> args;
        args.push_back(Type::getInt32Ty(*context));
        FunctionType* type = FunctionType::get(Type::getInt8PtrTy(*context), args, false);
        Function *GC_MallocFunc = Function::Create(type, Function::ExternalLinkage, Twine("GC_malloc"), mModule);
        GC_MallocFunc->setCallingConv(CallingConv::C);
        return GC_MallocFunc;
    }

    Function* GC_FreeFunction() {
        vector<Type*> args;
        args.push_back(Type::getInt8PtrTy(*context));
        FunctionType* type = FunctionType::get(Type::getVoidTy(*context), args, false);
        Function *GC_FreeFunc = Function::Create(type, Function::ExternalLinkage, Twine("GC_free"), mModule);
        GC_FreeFunc->setCallingConv(CallingConv::C);
        return GC_FreeFunc;
    }

    Function* llvmTrap() {
        vector<Type*> agrs;
        FunctionType* type = FunctionType::get(Type::getVoidTy(*context), agrs, false);
        Function *llvmTrap = Function::Create(type, Function::ExternalLinkage, Twine("llvm.trap"), mModule);
        llvmTrap->setCallingConv(CallingConv::C);
        return llvmTrap;
    }

    void generateCode(ModuleStatementNode *mainModule)
    {
        printfFunction();
        scanfFunction();
        llvmTrap();
        GC_InitFunction();
        GC_MallocFunction();
        GC_FreeFunction();

        for (auto g : *symbols)
        {
            g.second->codegen(*this);
        }

        if (isMainModule)
        {
            FunctionType *mainType = FunctionType::get(builder->getInt32Ty(), false);
            mMainFunction = Function::Create(mainType, Function::ExternalLinkage, "main",
                                             mModule);
            BasicBlock *entry = BasicBlock::Create(*context, "entry", mMainFunction);
            builder->SetInsertPoint(entry);

            unsigned LineNo = mainModule->block->loc.line;
            unsigned ScopeLine = LineNo;
            DIFile* unit = debugBuilder->createFile(dbgInfo.compileUnit->getFilename(), dbgInfo.compileUnit->getDirectory());
            llvm::DISubprogram *DbgFunc = debugBuilder->createFunction(
                    dbgInfo.compileUnit, "main", "main", unit, LineNo,
                    dbgInfo.CreateFunctionType({dbgInfo.getIntegerTy()}), ScopeLine,
                    llvm::DISubprogram::FlagPrivate,
                    llvm::DISubprogram::SPFlagDefinition);
            mMainFunction->setSubprogram(DbgFunc);

            pushBlock(entry);
            dbgInfo.lexicalBlocks.push_back(DbgFunc);
            dbgInfo.emitLocation(builder.get(), mainModule->block);
            builder->CreateCall(mModule->getFunction("GC_init"), {});
            for (auto decl : *mainModule->block->statements)
            {
                decl->codegen(*this);
            }
            auto var = builder->CreateRet(ConstantInt::get(*context, APInt(32, 0)));
            popBlock();
            dbgInfo.lexicalBlocks.pop_back();
        }
        debugBuilder->finalize();
    }

    std::map <std::string, llvm::Value*>& locals() {
        return blocks.back()->locals;
    }

    std::map <std::string, DeclarationNode*>& localsExprs() {
        return blocks.back()->localsExprs;
    }

    DeclarationNode* localsExprsLookup(const std::string &name) {
        DeclarationNode *value = nullptr;
        for (auto &b : blocks)
        {
            if (b->localsExprs.contains(name))
            {
                value = b->localsExprs[name];
                break;
            }
        }
        return value;
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
    bool contains(const std::string &name)
    {
        return std::ranges::find(*symbols, name, &std::pair<std::string, DeclarationNode*>::first) != symbols->end();
    }

    DeclarationNode* get(const std::string &name)
    {
        return (*(std::ranges::find(*symbols, name, &std::pair<std::string, DeclarationNode*>::first))).second;
    }

    DeclarationNode* lookupFuncs(const std::string &name)
    {
        if (contains(name)) {
            auto decl = get(name);
            if (dynamic_cast<FuncDecStatementNode*>(decl) != nullptr
                || dynamic_cast<ExternFuncDecStatementNode*>(decl) != nullptr
                ) {
                return decl;
            }
        }
        for (auto &g : *symbols)
        {
            if (dynamic_cast<TypeDecStatementNode*>(g.second) != nullptr)
            {
                auto typeDec = dynamic_cast<TypeDecStatementNode*>(g.second);
                for (auto &method : *typeDec->methods)
                {
                    if (method->name->value == name)
                    {
                        return method;
                    }
                }
            }
        }
        return nullptr;
    }

    DeclarationNode* lookupFuncPointers(const std::string &name)
    {
        if (contains(name)) {
            auto decl = get(name);
            if (dynamic_cast<FuncPointerStatementNode*>(decl) != nullptr) {
                return decl;
            }
        }
        return nullptr;
    }
};

#endif //SLANGPARSER_AST_HPP
