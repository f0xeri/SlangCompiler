//
// Created by f0xeri on 05.05.2023.
//

#ifndef SLANGCREFACTORED_AST_HPP
#define SLANGCREFACTORED_AST_HPP

#include <utility>
#include <variant>
#include <memory>
#include <vector>
#include "llvm/IR/Value.h"
#include "lexer/TokenType.hpp"
#include "codegen/CodeGenContext.hpp"

namespace Slangc::AST {
    enum ParameterType {
        In,
        Out,
        Var
    };

#pragma region Variant types for basic types of nodes
    using ExprNode = std::variant<
            std::unique_ptr<struct ArrayExprNode>,
            std::unique_ptr<struct BlockExprNode>,
            std::unique_ptr<struct BooleanExprNode>,
            std::unique_ptr<struct CharExprNode>,
            std::unique_ptr<struct FloatExprNode>,
            std::unique_ptr<struct FuncExprNode>,
            std::unique_ptr<struct IntExprNode>,
            std::unique_ptr<struct NilExprNode>,
            std::unique_ptr<struct OperatorExprNode>,
            std::unique_ptr<struct RealExprNode>,
            std::unique_ptr<struct StringExprNode>,
            std::unique_ptr<struct UnaryOperatorExprNode>,
            std::unique_ptr<struct VarExprNode>,
            std::unique_ptr<struct IndexesExprNode>,
            std::unique_ptr<struct IndexExprNode>,
            std::unique_ptr<struct CallExprNode>
    >;

    using VariableExprNode = std::variant<
            std::unique_ptr<struct IndexExprNode>,
            std::unique_ptr<struct IndexesExprNode>,
            std::unique_ptr<struct VarExprNode>
    >;

    using DeclarationNode = std::variant<
            std::unique_ptr<struct ArrayDecStatementNode>,
            std::unique_ptr<struct ExternFuncDecStatementNode>,
            std::unique_ptr<struct FieldArrayVarDecNode>,
            std::unique_ptr<struct FieldVarDecNode>,
            std::unique_ptr<struct FuncDecStatementNode>,
            std::unique_ptr<struct FuncParamDecStatementNode>,
            std::unique_ptr<struct FuncPointerStatementNode>,
            std::unique_ptr<struct MethodDecNode>,
            std::unique_ptr<struct TypeDecStatementNode>,
            std::unique_ptr<struct VarDecStatementNode>
    >;

    using StatementNode = std::variant<
            std::unique_ptr<struct AssignExprNode>,
            std::unique_ptr<struct CallExprNode>,
            std::unique_ptr<struct ArrayDecStatementNode>,
            std::unique_ptr<struct ExternFuncDecStatementNode>,
            std::unique_ptr<struct FieldArrayVarDecNode>,
            std::unique_ptr<struct FieldVarDecNode>,
            std::unique_ptr<struct FuncDecStatementNode>,
            std::unique_ptr<struct FuncParamDecStatementNode>,
            std::unique_ptr<struct FuncPointerStatementNode>,
            std::unique_ptr<struct MethodDecNode>,
            std::unique_ptr<struct TypeDecStatementNode>,
            std::unique_ptr<struct VarDecStatementNode>,
            std::unique_ptr<struct DeleteExprNode>,
            std::unique_ptr<struct ElseIfStatementNode>,
            std::unique_ptr<struct IfStatementNode>,
            std::unique_ptr<struct InputStatementNode>,
            std::unique_ptr<struct ModuleStatementNode>,
            std::unique_ptr<struct OutputStatementNode>,
            std::unique_ptr<struct ReturnStatementNode>,
            std::unique_ptr<struct VarExprNode>,
            std::unique_ptr<struct IndexesExprNode>,
            std::unique_ptr<struct IndexExprNode>,
            std::unique_ptr<struct WhileStatementNode>
    >;
#pragma endregion

#pragma region Nodes declarations
    // codegen methods are defined in codegen/CodeGen.cpp

    struct IntExprNode {
        SourceLocation location;
        int value{};
        bool isConst = true;

        IntExprNode(SourceLocation location, int value) : location(std::move(location)), value(value) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct FloatExprNode {
        SourceLocation location;
        float value{};
        bool isConst = true;

        FloatExprNode(SourceLocation location, float value) : location(location), value(value) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct RealExprNode {
        SourceLocation location;
        double value{};
        bool isConst = true;

        RealExprNode(SourceLocation location, double value) : location(location), value(value) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct CharExprNode {
        SourceLocation location;
        char value{};
        bool isConst = true;

        CharExprNode(SourceLocation location, char value) : location(location), value(value) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct StringExprNode {
        SourceLocation location;
        std::string value;
        bool isConst = true;

        StringExprNode(SourceLocation location, std::string value) : location(location), value(std::move(value)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct NilExprNode {
        SourceLocation location;
        ExprNode expr;
        bool isConst = true;

        NilExprNode(SourceLocation location, ExprNode expr) : location(location), expr(std::move(expr)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct ArrayExprNode {
        SourceLocation location;
        std::vector<ExprNode> values;
        std::string type;
        ExprNode size;
        bool isConst = false;

        ArrayExprNode(SourceLocation location, std::vector<ExprNode> values, std::string type, ExprNode size)
                : location(location), values(std::move(values)), type(std::move(type)), size(std::move(size)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct BooleanExprNode {
        SourceLocation location;
        bool value{};
        bool isConst = true;

        BooleanExprNode(SourceLocation location, bool value) : location(location), value(value) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct VarExprNode {
        SourceLocation location;
        std::string value;
        bool dotClass = false;
        bool dotModule = false;
        bool isPointer = false;
        int index = -1;
        bool isConst = false;

        VarExprNode(SourceLocation location, std::string value) : location(location), value(std::move(value)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct IndexExprNode {
        SourceLocation location;
        ExprNode indexExpr;
        ExprNode assign;
        bool isConst = false;

        IndexExprNode(SourceLocation location, ExprNode indexExpr, ExprNode assign) : location(location),
                                                                                      indexExpr(std::move(indexExpr)),
                                                                                      assign(std::move(assign)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct IndexesExprNode {
        SourceLocation location;
        std::vector<ExprNode> indexes;
        ExprNode assign;
        bool isConst = false;

        IndexesExprNode(SourceLocation location, std::vector<ExprNode> indexes, ExprNode assign) : location(location),
                                                                                                   indexes(std::move(
                                                                                                           indexes)),
                                                                                                   assign(std::move(
                                                                                                           assign)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct UnaryOperatorExprNode {
        SourceLocation location;
        TokenType op{};
        ExprNode expr;
        bool isConst = false;

        UnaryOperatorExprNode(SourceLocation location, TokenType op, ExprNode expr) : location(location), op(op),
                                                                                      expr(std::move(expr)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct OperatorExprNode {
        SourceLocation location;
        TokenType op{};
        ExprNode left, right;
        bool isConst = false;

        OperatorExprNode(SourceLocation location, TokenType op, ExprNode left, ExprNode right) : location(location),
                                                                                                 op(op),
                                                                                                 left(std::move(left)),
                                                                                                 right(std::move(
                                                                                                         right)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct CallExprNode {
        SourceLocation location;
        VariableExprNode name;
        std::vector<ExprNode> args;
        bool isConst = false;

        CallExprNode(SourceLocation location, VariableExprNode name, std::vector<ExprNode> args) : location(location),
                                                                                                   name(std::move(
                                                                                                           name)),
                                                                                                   args(std::move(
                                                                                                           args)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct DeleteExprNode {
        SourceLocation location;
        ExprNode expr;

        DeleteExprNode(SourceLocation location, ExprNode expr) : location(location), expr(std::move(expr)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct BlockExprNode {
        SourceLocation location;
        std::vector<StatementNode> statements;
        bool isConst = false;

        BlockExprNode(SourceLocation location, std::vector<StatementNode> statements) : location(location), statements(
                std::move(statements)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct AssignExprNode {
        SourceLocation location;
        VariableExprNode left;
        ExprNode right;

        AssignExprNode(SourceLocation location, VariableExprNode left, ExprNode right) : location(location),
                                                                                         left(std::move(left)),
                                                                                         right(std::move(right)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct FuncParamDecStatementNode {
        SourceLocation location;
        std::string name;
        ParameterType parameterType{};
        ExprNode type;
        ExprNode expr;

        FuncParamDecStatementNode(SourceLocation location, std::string name, ParameterType parameterType, ExprNode type,
                                  ExprNode expr)
                : location(location), name(std::move(name)), parameterType(parameterType), type(std::move(type)),
                  expr(std::move(expr)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct FuncExprNode {
        SourceLocation location;
        ExprNode type;
        bool isFunction = true;
        std::vector<std::unique_ptr<FuncParamDecStatementNode>> params;
        bool isConst = false;

        FuncExprNode(SourceLocation location, ExprNode type, bool isFunction,
                     std::vector<std::unique_ptr<FuncParamDecStatementNode>> params)
                : location(location), type(std::move(type)), isFunction(isFunction), params(std::move(params)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct ReturnStatementNode {
        SourceLocation location;
        ExprNode expr;

        ReturnStatementNode(SourceLocation location, ExprNode expr) : location(location), expr(std::move(expr)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct OutputStatementNode {
        SourceLocation location;
        ExprNode expr;

        OutputStatementNode(SourceLocation location, ExprNode expr) : location(location), expr(std::move(expr)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct InputStatementNode {
        SourceLocation location;
        ExprNode expr;

        InputStatementNode(SourceLocation location, ExprNode expr) : location(location), expr(std::move(expr)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct VarDecStatementNode {
        SourceLocation location;
        std::string name;
        std::string type;
        ExprNode expr;
        bool isGlobal = false;
        bool isPrivate = false;
        bool isExtern = false;

        VarDecStatementNode(SourceLocation location, std::string name, std::string type, ExprNode expr,
                            bool isGlobal = false, bool isPrivate = false, bool isExtern = false)
                : location(location), name(std::move(name)), type(std::move(type)), expr(std::move(expr)),
                  isGlobal(isGlobal), isPrivate(isPrivate), isExtern(isExtern) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct FuncPointerStatementNode {
        SourceLocation location;
        std::string name;
        ExprNode type;
        std::vector<std::unique_ptr<FuncParamDecStatementNode>> args;
        bool isFunction = false;
        bool isGlobal = false;
        bool isPrivate = false;
        bool isExtern = false;
        ExprNode expr;

        FuncPointerStatementNode(SourceLocation location, std::string name, ExprNode type,
                                 std::vector<std::unique_ptr<FuncParamDecStatementNode>> args, ExprNode expr,
                                 bool isFunction = false, bool isGlobal = false, bool isPrivate = false,
                                 bool isExtern = false)
                : location(location), name(std::move(name)), type(std::move(type)), args(std::move(args)),
                  isFunction(isFunction), isGlobal(isGlobal), isPrivate(isPrivate), isExtern(isExtern),
                  expr(std::move(expr)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct ArrayDecStatementNode {
        SourceLocation location;
        std::string name;
        bool isGlobal = false;
        bool isPrivate = false;
        std::unique_ptr<ArrayExprNode> expr;
        ExprNode assignExpr;
        int indicesCount = 1;
        bool isExtern = false;
        bool isConst = false;

        ArrayDecStatementNode(SourceLocation location, std::string name, std::unique_ptr<ArrayExprNode> expr,
                              bool isGlobal = false, bool isPrivate = false, bool isExtern = false,
                              bool isConst = false)
                : location(location), name(std::move(name)), isGlobal(isGlobal), isPrivate(isPrivate),
                  expr(std::move(expr)), isExtern(isExtern), isConst(isConst) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct FuncDecStatementNode {
        SourceLocation location;
        std::string name;
        ExprNode type;
        std::vector<std::unique_ptr<FuncParamDecStatementNode>> args;
        std::unique_ptr<BlockExprNode> block;
        bool isPrivate = false;
        bool isFunction = false;

        FuncDecStatementNode(SourceLocation location, std::string name, ExprNode type,
                             std::vector<std::unique_ptr<FuncParamDecStatementNode>> args,
                             std::unique_ptr<BlockExprNode> block, bool isPrivate = false, bool isFunction = false)
                : location(location), name(std::move(name)), type(std::move(type)), args(std::move(args)),
                  block(std::move(block)), isPrivate(isPrivate), isFunction(isFunction) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct FieldVarDecNode {
        SourceLocation location;
        std::string name;
        std::string typeName;
        bool isPrivate;
        int index;
        std::string type;
        ExprNode expr;

        FieldVarDecNode(SourceLocation location, std::string name, std::string typeName, bool isPrivate, int index,
                        std::string type, ExprNode expr)
                : location(location), name(std::move(name)), typeName(std::move(typeName)), isPrivate(isPrivate),
                  index(index), type(std::move(type)), expr(std::move(expr)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct FieldArrayVarDecNode {
        SourceLocation location;
        std::string name;
        std::string typeName;
        bool isPrivate;
        int index;
        std::unique_ptr<ArrayDecStatementNode> var;

        FieldArrayVarDecNode(SourceLocation location, std::string name, std::string typeName, bool isPrivate, int index,
                             std::unique_ptr<ArrayDecStatementNode> var)
                : location(location), name(std::move(name)), typeName(std::move(typeName)), isPrivate(isPrivate),
                  index(index), var(std::move(var)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct MethodDecNode {
        SourceLocation location;
        std::string name;
        ExprNode type;
        VariableExprNode thisName;
        std::vector<std::unique_ptr<FuncParamDecStatementNode>> args;
        std::unique_ptr<BlockExprNode> block;
        bool isPrivate = false;
        bool isFunction = false;

        MethodDecNode(SourceLocation location, std::string name, ExprNode type, VariableExprNode thisName,
                      std::vector<std::unique_ptr<FuncParamDecStatementNode>> args,
                      std::unique_ptr<BlockExprNode> block, bool isPrivate = false, bool isFunction = false)
                : location(location), name(std::move(name)), type(std::move(type)), thisName(std::move(thisName)),
                  args(std::move(args)), block(std::move(block)), isPrivate(isPrivate), isFunction(isFunction) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct TypeDecStatementNode {
        SourceLocation location;
        std::string name;
        std::vector<DeclarationNode> fields;
        std::vector<std::unique_ptr<MethodDecNode>> methods;
        std::unique_ptr<TypeDecStatementNode> parentType;
        bool isExtern = false;
        bool isPrivate = false;

        TypeDecStatementNode(SourceLocation location, std::string name, std::vector<DeclarationNode> fields,
                             std::vector<std::unique_ptr<MethodDecNode>> methods,
                             std::unique_ptr<TypeDecStatementNode> parentType, bool isExtern = false,
                             bool isPrivate = false)
                : location(location), name(std::move(name)), fields(std::move(fields)), methods(std::move(methods)),
                  parentType(std::move(parentType)), isExtern(isExtern), isPrivate(isPrivate) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct ExternFuncDecStatementNode {
        SourceLocation location;
        std::string name;
        ExprNode type;
        std::vector<std::unique_ptr<FuncParamDecStatementNode>> args;
        bool isPrivate = false;
        bool isFunction = false;

        ExternFuncDecStatementNode(SourceLocation location, std::string name, ExprNode type,
                                   std::vector<std::unique_ptr<FuncParamDecStatementNode>> args, bool isPrivate = false,
                                   bool isFunction = false)
                : location(location), name(std::move(name)), type(std::move(type)), args(std::move(args)),
                  isPrivate(isPrivate), isFunction(isFunction) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct ElseIfStatementNode {
        SourceLocation location;
        ExprNode condExpr;
        std::unique_ptr<BlockExprNode> trueBlock;

        ElseIfStatementNode(SourceLocation location, ExprNode condExpr, std::unique_ptr<BlockExprNode> trueBlock)
                : location(location), condExpr(std::move(condExpr)), trueBlock(std::move(trueBlock)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct IfStatementNode {
        SourceLocation location;
        ExprNode condExpr;
        std::unique_ptr<BlockExprNode> trueBlock;
        std::unique_ptr<BlockExprNode> falseBlock;
        std::vector<std::unique_ptr<ElseIfStatementNode>> elseIfNodes;

        IfStatementNode(SourceLocation location, ExprNode condExpr, std::unique_ptr<BlockExprNode> trueBlock,
                        std::unique_ptr<BlockExprNode> falseBlock,
                        std::vector<std::unique_ptr<ElseIfStatementNode>> elseIfNodes)
                : location(location), condExpr(std::move(condExpr)), trueBlock(std::move(trueBlock)),
                  falseBlock(std::move(falseBlock)), elseIfNodes(std::move(elseIfNodes)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct WhileStatementNode {
        SourceLocation location;
        ExprNode whileExpr;
        std::unique_ptr<BlockExprNode> block;

        WhileStatementNode(SourceLocation location, ExprNode whileExpr, std::unique_ptr<BlockExprNode> block)
                : location(location), whileExpr(std::move(whileExpr)), block(std::move(block)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct ModuleStatementNode {
        SourceLocation location;
        std::string name;
        std::unique_ptr<BlockExprNode> block;

        ModuleStatementNode(SourceLocation location, std::string name, std::unique_ptr<BlockExprNode> block)
                : location(location), name(std::move(name)), block(std::move(block)) {}

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

#pragma endregion

    struct IsExprConstVisitor {
        auto operator()(const auto &x) const -> bool {
            return x.get()->isConst;
        }
    };

    static auto isConstExpr(const ExprNode &expr) -> bool {
        return std::visit(IsExprConstVisitor{}, expr);
    }

    struct DeclarationNameVisitor {
        auto operator()(const auto &x) const -> std::string_view {
            return x.get()->name;
        }
    };

    static auto getDeclarationName(const DeclarationNode &declaration) -> std::string_view {
        return std::visit(DeclarationNameVisitor{}, declaration);
    }
}


#endif //SLANGCREFACTORED_AST_HPP
