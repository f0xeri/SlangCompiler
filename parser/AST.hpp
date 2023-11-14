//
// Created by f0xeri on 05.05.2023.
//

#ifndef SLANGCREFACTORED_AST_HPP
#define SLANGCREFACTORED_AST_HPP

#include <memory>
#include <utility>
#include <variant>
#include <memory>
#include <vector>
#include <optional>
#include <iostream>
#include "llvm/IR/Value.h"
#include "lexer/TokenType.hpp"
#include "codegen/CodeGenContext.hpp"
#include "ASTFwdDecl.hpp"
#include "analysis/Context.hpp"


namespace Slangc {
    struct ExprTypeVisitor {
        const Context &analysis;
        std::vector<ErrorMessage>& errors;
        auto operator()(const auto &x) const -> std::optional<ExprPtrVariant> {
            return x->getType(analysis, errors);
        }
    };

    static auto getExprType(const ExprPtrVariant &expr, const Context &analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> {
        return std::visit(ExprTypeVisitor{analysis, errors}, expr);
    }

    struct DeclTypeVisitor {
        const Context &analysis;
        std::vector<ErrorMessage>& errors;
        auto operator()(const auto &x) const -> std::optional<ExprPtrVariant> {
            return x->getType(analysis, errors);
        }
    };

    static auto getDeclType(const DeclPtrVariant &expr, const Context &analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> {
        return std::visit(DeclTypeVisitor{analysis, errors}, expr);
    }

#pragma region Nodes declarations
    // codegen methods are defined in codegen/CodeGen.cpp

    struct TypeExprNode {
        SourceLoc loc{0, 0};
        std::string type;
        bool isConst = true;
        TypeExprNode(SourceLoc loc, std::string type) : loc(loc), type(std::move(type)) {};
        TypeExprNode(std::string type) : type(std::move(type)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<TypeExprNode>(*this); }
    };

    struct IntExprNode {
        SourceLoc loc{0, 0};
        int value{};
        bool isConst = true;

        IntExprNode(SourceLoc loc, int value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant>  { return std::make_unique<TypeExprNode>("integer"); }
    };

    struct FloatExprNode {
        SourceLoc loc{0, 0};
        float value{};
        bool isConst = true;

        FloatExprNode(SourceLoc loc, float value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<TypeExprNode>("float"); }
    };

    struct RealExprNode {
        SourceLoc loc{0, 0};
        double value{};
        bool isConst = true;

        RealExprNode(SourceLoc loc, double value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<TypeExprNode>("real"); }
    };

    struct CharExprNode {
        SourceLoc loc{0, 0};
        char value{};
        bool isConst = true;

        CharExprNode(SourceLoc loc, char value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<TypeExprNode>("character"); }
    };

    struct StringExprNode {
        SourceLoc loc{0, 0};
        std::string value;
        bool isConst = true;

        StringExprNode(SourceLoc loc, std::string value) : loc(loc), value(std::move(value)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> {
            return std::make_unique<ArrayExprNode>(loc, std::nullopt,
                                                   std::make_unique<TypeExprNode>("character"),
                                                   std::make_unique<IntExprNode>(loc, value.size()));
        }
    };

    struct NilExprNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant type;
        bool isConst = true;

        NilExprNode(SourceLoc loc, ExprPtrVariant type) : loc(loc), type(type) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return type; }
    };

    struct ArrayExprNode {
        SourceLoc loc{0, 0};
        std::optional<std::vector<ExprPtrVariant>> values;
        ExprPtrVariant type;
        ExprPtrVariant size;
        bool isConst = false;

        ArrayExprNode(SourceLoc loc, std::optional<std::vector<ExprPtrVariant>> values, ExprPtrVariant type, ExprPtrVariant size) :
            loc(loc), values(std::move(values)), type(std::move(type)), size(std::move(size)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<ArrayExprNode>(*this); }
        [[nodiscard]] auto getIndicesCount() const -> uint64_t {
            auto finalType = type;
            auto sz = 1;
            while (auto arrayType = std::get_if<ArrayExprPtr>(&finalType)) {
                finalType = arrayType->get()->type;
                sz++;
            }
            return sz;
        }
    };

    struct BooleanExprNode {
        SourceLoc loc{0, 0};
        bool value{};
        bool isConst = true;

        BooleanExprNode(SourceLoc loc, bool value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<TypeExprNode>("boolean"); }
    };

    struct VarExprNode {
        SourceLoc loc{0, 0};
        std::string name;
        bool dotClass = false;
        bool dotModule = false;
        bool isPointer = false;
        int index = -1;
        bool isConst = false;

        VarExprNode(SourceLoc loc, std::string name, bool dotClass = false, bool dotModule = false, bool isPointer = false, int index = -1)
            : loc(loc), name(std::move(name)), dotClass(dotClass), dotModule(dotModule), isPointer(isPointer), index(index) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return getDeclType(*analysis.lookup(name), analysis, errors); }
    };

    struct IndexExprNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant expr;
        ExprPtrVariant indexExpr;
        std::optional<ExprPtrVariant> assign = std::nullopt;
        bool dotClass = false;
        bool dotModule = false;
        bool isPointer = false;
        int index = -1;
        bool isConst = false;

        IndexExprNode(SourceLoc loc, ExprPtrVariant expr, ExprPtrVariant indexExpr, std::optional<ExprPtrVariant> assign = std::nullopt,
                      bool dotClass = false, bool dotModule = false, bool isPointer = false, int index = -1)
            : loc(loc), expr(std::move(expr)), indexExpr(std::move(indexExpr)), assign(std::move(assign)), dotClass(dotClass),
              dotModule(dotModule), isPointer(isPointer), index(index) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> {
            ExprPtrVariant tmp = getExprType(expr, analysis, errors).value();
            if (std::holds_alternative<ArrayExprPtr>(tmp))
                return std::get<ArrayExprPtr>(tmp)->type;
            else
                return std::nullopt;
        }
    };

    struct IndexesExprNode {
        SourceLoc loc{0, 0};
        std::vector<ExprPtrVariant> indexes;
        std::optional<ExprPtrVariant> assign = std::nullopt;
        bool isConst = false;

        IndexesExprNode(SourceLoc loc, std::vector<ExprPtrVariant> indexes, std::optional<ExprPtrVariant> assign = std::nullopt)
            : loc(loc), indexes(std::move(indexes)), assign(std::move(assign)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return getExprType(indexes.back(), analysis, errors); }
    };

    struct UnaryOperatorExprNode {
        SourceLoc loc{0, 0};
        TokenType op{};
        ExprPtrVariant expr;
        bool isConst = false;

        UnaryOperatorExprNode(SourceLoc loc, TokenType op, ExprPtrVariant expr)
            : loc(loc), op(op), expr(std::move(expr)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return getExprType(expr, analysis, errors); }
    };

    struct OperatorExprNode {
        SourceLoc loc{0, 0};
        TokenType op{};
        ExprPtrVariant left, right;
        bool isConst = false;

        OperatorExprNode(SourceLoc loc, TokenType op, ExprPtrVariant left, ExprPtrVariant right)
            : loc(loc), op(op), left(std::move(left)), right(std::move(right)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> {
            if (op == TokenType::Less || op == TokenType::LessOrEqual || op == TokenType::Greater || op == TokenType::GreaterOrEqual ||
                op == TokenType::Equal || op == TokenType::NotEqual || op == TokenType::And || op == TokenType::Or) {
                return std::make_unique<TypeExprNode>("boolean");
            }
            return getExprType(left, analysis, errors);
        }
    };

    struct FuncExprNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant type;
        bool isFunction = true;
        std::vector<FuncParamDecStmtPtr> params;
        bool isConst = false;

        FuncExprNode(SourceLoc loc, ExprPtrVariant type, std::vector<FuncParamDecStmtPtr> params, bool isFunction = true)
                : loc(loc), type(std::move(type)), isFunction(isFunction), params(std::move(params)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return getExprType(type, analysis, errors); }
    };

    struct CallExprNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant name;        // TODO: change to VarExprPtrVariant?
        std::vector<ExprPtrVariant> args;
        bool isConst = false;

        CallExprNode(SourceLoc loc, ExprPtrVariant name, std::vector<ExprPtrVariant> args)
            : loc(loc), name(std::move(name)), args(std::move(args)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> {
            auto type = getExprType(name, analysis, errors);
            if (!type.has_value()) return std::nullopt;
            if (auto funcType = std::get_if<FuncExprPtr>(&type.value())) {
                return (*funcType)->type;
            }
            return getExprType(name, analysis, errors);
        }
    };

    struct TypeDecStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        std::vector<DeclPtrVariant> fields;
        std::vector<MethodDecPtr> methods;
        std::optional<std::string> parentTypeName = std::nullopt;
        bool isExtern = false;
        bool isPrivate = false;

        TypeDecStatementNode(SourceLoc loc, std::string name, std::vector<DeclPtrVariant> fields,
                             std::vector<MethodDecPtr> methods, std::optional<std::string> parentTypeName = std::nullopt,
                             bool isExtern = false, bool isPrivate = false)
                : loc(loc), name(std::move(name)), fields(std::move(fields)), methods(std::move(methods)),
                  parentTypeName(std::move(parentTypeName)), isExtern(isExtern), isPrivate(isPrivate) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<TypeExprNode>(name); }
    };

    struct FieldVarDecNode {
        SourceLoc loc{0, 0};
        std::string name;
        TypeExprNode typeName;
        bool isPrivate;
        uint32_t index;
        TypeExprNode typeExpr;
        std::optional<ExprPtrVariant> expr;

        FieldVarDecNode(SourceLoc loc, std::string name, std::string typeName, bool isPrivate, int index, std::string type, std::optional<ExprPtrVariant> expr = std::nullopt)
                : loc(loc), name(std::move(name)), typeName(std::move(typeName)), isPrivate(isPrivate), index(index), typeExpr(std::move(type)), expr(std::move(expr)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<TypeExprNode>(typeExpr); }
    };

    struct FieldArrayVarDecNode {
        SourceLoc loc{0, 0};
        std::string name;
        TypeExprNode typeName;
        uint32_t index;
        ArrayExprPtr expr;
        std::optional<ExprPtrVariant> assignExpr;
        uint32_t indicesCount = 1;
        bool isPrivate = false;
        bool isConst = false;

        FieldArrayVarDecNode(SourceLoc loc, std::string name, std::string typeName, uint32_t index, ArrayExprPtr expr,
                             std::optional<ExprPtrVariant> assignExpr = std::nullopt, uint32_t indicesCount = 1,
                             bool isPrivate = false, bool isConst = false)
                : loc(loc), name(std::move(name)), typeName(std::move(typeName)), index(index), expr(std::move(expr)),
                  assignExpr(std::move(assignExpr)), indicesCount(indicesCount),
                  isPrivate(isPrivate), isConst(isConst) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return getExprType(expr, analysis, errors); }
    };

    struct FieldFuncPointerStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        TypeExprNode typeName;
        uint32_t index;
        FuncExprPtr expr;
        bool isFunction = false;
        bool isPrivate = false;
        std::optional<ExprPtrVariant> assignExpr;

        FieldFuncPointerStatementNode(SourceLoc loc, std::string name, std::string typeName, uint32_t index, FuncExprPtr expr,
                                      bool isFunction = false, bool isPrivate = false, std::optional<ExprPtrVariant> assignExpr = std::nullopt)
                : loc(loc), name(std::move(name)), typeName(std::move(typeName)), index(index), expr(std::move(expr)),
                  isFunction(isFunction), isPrivate(isPrivate), assignExpr(std::move(assignExpr)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return expr; }
    };

    struct MethodDecNode {
        SourceLoc loc{0, 0};
        std::string name;
        FuncExprPtr expr;
        std::string thisName;
        std::optional<BlockStmtPtr> block;
        bool isPrivate = false;
        bool isFunction = false;

        MethodDecNode(SourceLoc loc, std::string name, FuncExprPtr expr, std::string thisName,
                      std::optional<BlockStmtPtr> block = std::nullopt, bool isPrivate = false, bool isFunction = false)
                : loc(loc), name(std::move(name)), expr(std::move(expr)), thisName(std::move(thisName)),
                  block(std::move(block)), isPrivate(isPrivate), isFunction(isFunction) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return expr->type; }
    };

    struct AccessExprNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant expr;
        std::string name;
        bool isConst = false;

        AccessExprNode(SourceLoc loc, ExprPtrVariant expr, std::string name)
            : loc(loc), expr(std::move(expr)), name(std::move(name)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> {
            auto exprType = getExprType(expr, analysis, errors);
            std::optional<ExprPtrVariant> result;
            if (!exprType.has_value()) {
                errors.emplace_back("Failed to get type of expression.", loc, false, true);
                return std::nullopt;
            }
            if (std::holds_alternative<TypeExprPtr>(exprType.value())) {
                if (auto typeOpt = analysis.lookupType(std::get<TypeExprPtr>(exprType.value())->type)) {
                    auto type = typeOpt.value();
                    auto found = false;
                    for (const auto &field: type->fields) {
                        if (auto fieldVar = std::get_if<FieldVarDecPtr>(&field)) {
                            found = ((*fieldVar)->name == name);
                        } else if (const auto &fieldArrayVar = std::get_if<FieldArrayVarDecPtr>(&field)) {
                            found = ((*fieldArrayVar)->name == name);
                        } else if (auto fieldFuncPointer = std::get_if<FieldFuncPointerStmtPtr>(&field)) {
                            found = ((*fieldFuncPointer)->name == name);
                        }

                        if (found) {
                            result = getDeclType(field, analysis, errors);
                            break;
                        }
                    }
                    if (!found) {
                        for (const auto &method: type->methods) {
                            if (method->name == type->name + "." + name) {
                                found = true;
                                result = getDeclType(method, analysis, errors);
                                break;
                            }
                        }
                    }
                    if (!found) {
                        errors.emplace_back("Type '" + std::get<TypeExprPtr>(exprType.value())->type + "' does not have field or method called '" + name + "'.", loc, false, true);
                        return std::nullopt;
                    }
                }
            }
            else {
                errors.emplace_back("Type of expression is not accessible.", loc, false, true);
                return std::nullopt;
            }
            return result;
        }
    };

    struct DeleteStmtNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant expr;

        DeleteStmtNode(SourceLoc loc, ExprPtrVariant expr) : loc(loc), expr(std::move(expr)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct BlockStmtNode {
        SourceLoc loc{0, 0};
        std::vector<StmtPtrVariant> statements;
        bool isConst = false;

        BlockStmtNode(SourceLoc loc, std::vector<StmtPtrVariant> statements) : loc(loc), statements(std::move(statements)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct AssignExprNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant left;
        ExprPtrVariant right;

        AssignExprNode(SourceLoc loc, ExprPtrVariant left, ExprPtrVariant right)
            : loc(loc), left(std::move(left)), right(std::move(right)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct FuncParamDecStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        ParameterType parameterType{};
        ExprPtrVariant type;
        std::optional<ExprPtrVariant> expr;

        FuncParamDecStatementNode(SourceLoc loc, std::string name, ParameterType parameterType, ExprPtrVariant type, std::optional<ExprPtrVariant> expr = std::nullopt)
            : loc(loc), name(std::move(name)), parameterType(parameterType), type(std::move(type)), expr(std::move(expr)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return type; }
    };

    struct ReturnStatementNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant expr;

        ReturnStatementNode(SourceLoc loc, ExprPtrVariant expr) : loc(loc), expr(std::move(expr)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct OutputStatementNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant expr;

        OutputStatementNode(SourceLoc loc, ExprPtrVariant expr) : loc(loc), expr(std::move(expr)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct InputStatementNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant expr;

        InputStatementNode(SourceLoc loc, ExprPtrVariant expr) : loc(loc), expr(std::move(expr)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct VarDecStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        TypeExprNode typeExpr;
        std::optional<ExprPtrVariant> expr;
        bool isGlobal = false;
        bool isPrivate = false;
        bool isExtern = false;

        VarDecStatementNode(SourceLoc loc, std::string name, std::string type, std::optional<ExprPtrVariant> expr = std::nullopt, bool isGlobal = false, bool isPrivate = false, bool isExtern = false)
            : loc(loc), name(std::move(name)), typeExpr(std::move(type)), expr(std::move(expr)), isGlobal(isGlobal), isPrivate(isPrivate), isExtern(isExtern) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<TypeExprNode>(typeExpr); }
    };

    struct FuncPointerStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        FuncExprPtr expr;
        bool isFunction = false;
        bool isGlobal = false;
        bool isPrivate = false;
        bool isExtern = false;
        std::optional<ExprPtrVariant> assignExpr;

        FuncPointerStatementNode(SourceLoc loc, std::string name, FuncExprPtr expr,
                                 std::optional<ExprPtrVariant> value = std::nullopt, bool isFunction = false, bool isGlobal = false,
                                 bool isPrivate = false, bool isExtern = false)
            : loc(loc), name(std::move(name)), expr(std::move(expr)), assignExpr(std::move(value)), isFunction(isFunction),
              isGlobal(isGlobal), isPrivate(isPrivate), isExtern(isExtern) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return expr; }
    };

    struct ArrayDecStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        ArrayExprPtr expr;
        std::optional<ExprPtrVariant> assignExpr;
        int indicesCount = 1;
        bool isGlobal = false;
        bool isPrivate = false;
        bool isExtern = false;
        bool isConst = false;

        ArrayDecStatementNode(SourceLoc loc, std::string name, ArrayExprPtr expr, std::optional<ExprPtrVariant> assignExpr,
                              int indicesCount = 1, bool isGlobal = false, bool isPrivate = false,
                              bool isExtern = false, bool isConst = false)
            : loc(loc), name(std::move(name)), expr(std::move(expr)), assignExpr(std::move(assignExpr)), indicesCount(indicesCount),
            isGlobal(isGlobal), isPrivate(isPrivate), isExtern(isExtern), isConst(isConst) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return getExprType(expr, analysis, errors); }
    };

    struct FuncDecStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        FuncExprPtr expr;
        std::optional<BlockStmtPtr> block;
        bool isPrivate = false;
        bool isFunction = false;

        FuncDecStatementNode(SourceLoc loc, std::string name, FuncExprPtr expr, std::optional<BlockStmtPtr> block = std::nullopt, bool isPrivate = false, bool isFunction = false)
            : loc(loc), name(std::move(name)), expr(std::move(expr)), block(std::move(block)), isPrivate(isPrivate), isFunction(isFunction) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return expr; }
    };

    struct ExternFuncDecStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        FuncExprPtr expr;
        bool isPrivate = false;
        bool isFunction = false;

        ExternFuncDecStatementNode(SourceLoc loc, std::string name, FuncExprPtr expr,
                                   bool isPrivate = false, bool isFunction = false)
                                   : loc(loc), name(std::move(name)), expr(std::move(expr)),
                                     isPrivate(isPrivate), isFunction(isFunction) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return expr->type; }
    };

    struct ElseIfStatementNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant condition;
        BlockStmtPtr trueBlock;

        ElseIfStatementNode(SourceLoc loc, ExprPtrVariant condExpr, BlockStmtPtr trueBlock)
            : loc(loc), condition(std::move(condExpr)), trueBlock(std::move(trueBlock)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct IfStatementNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant condition;
        BlockStmtPtr trueBlock;
        BlockStmtPtr falseBlock;
        std::vector<ElseIfStatementPtr> elseIfNodes;

        IfStatementNode(SourceLoc loc, ExprPtrVariant condExpr, BlockStmtPtr trueBlock, BlockStmtPtr falseBlock,
                        std::vector<ElseIfStatementPtr> elseIfNodes)
            : loc(loc), condition(std::move(condExpr)), trueBlock(std::move(trueBlock)),
              falseBlock(std::move(falseBlock)), elseIfNodes(std::move(elseIfNodes)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct WhileStatementNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant condition;
        BlockStmtPtr block;

        WhileStatementNode(SourceLoc loc, ExprPtrVariant whileExpr, BlockStmtPtr block)
            : loc(loc), condition(std::move(whileExpr)), block(std::move(block)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct ModuleDeclNode {
        SourceLoc loc{0, 0};
        std::string name;
        BlockStmtPtr block;

        ModuleDeclNode(SourceLoc loc, std::string name, BlockStmtPtr block)
            : loc(loc), name(std::move(name)), block(std::move(block)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const Context& analysis, std::vector<ErrorMessage>& errors) -> std::optional<ExprPtrVariant> { return std::make_unique<TypeExprNode>(name); }
    };

#pragma endregion

    template<typename T, typename... Args>
    auto create(Args&&... args) -> std::shared_ptr<T> {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename VARIANT_T>
    struct is_variant_member;

    template<typename T, typename... ALL_T>
    struct is_variant_member<T, std::variant<ALL_T...>>
            : public std::disjunction<std::is_same<T, ALL_T>...> {};

    template<typename T, typename... Args>
    requires is_variant_member<std::shared_ptr<T>, ExprPtrVariant>::value
    auto createExpr(Args&&... args) -> ExprPtrVariant {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    requires is_variant_member<std::shared_ptr<T>, VarExprPtrVariant>::value
    auto createVarExpr(Args&&... args) -> VarExprPtrVariant {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    requires is_variant_member<std::shared_ptr<T>, DeclPtrVariant>::value
    auto createDecl(Args&&... args) -> DeclPtrVariant {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    requires is_variant_member<std::shared_ptr<T>, StmtPtrVariant>::value
    auto createStmt(Args&&... args) -> StmtPtrVariant {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    struct IsExprConstVisitor {
        auto operator()(const auto &x) const -> bool {
            return x->isConst;
        }
    };

    static auto isConstExpr(const ExprPtrVariant &expr) -> bool {
        return std::visit(IsExprConstVisitor{}, expr);
    }

    struct DeclarationNameVisitor {
        auto operator()(const auto &x) const -> std::string_view {
            return x->name;
        }
    };

    static auto getDeclarationName(const DeclPtrVariant &declaration) -> std::string_view {
        return std::visit(DeclarationNameVisitor{}, declaration);
    }

    static auto getFieldIndex(const std::string &name, const std::vector<DeclPtrVariant> &fields) -> int {
        for (auto &field : fields) {
            if (std::get<FieldVarDecPtr>(field)->name == name) return std::get<FieldVarDecPtr>(field)->index;
        }
        return -1;
    }

    static bool compareTypes(const ExprPtrVariant &type1, const ExprPtrVariant &type2) {
        if (auto type1Ptr = std::get_if<TypeExprPtr>(&type1)) {
            if (auto type2Ptr = std::get_if<TypeExprPtr>(&type2)) {
                return type1Ptr->get()->type == type2Ptr->get()->type;
            }
        }
        if (auto type1Ptr = std::get_if<ArrayExprPtr>(&type1)) {
            if (auto type2Ptr = std::get_if<ArrayExprPtr>(&type2)) {
                return compareTypes(type1Ptr->get()->type, type2Ptr->get()->type);
            }
        }
        if (std::holds_alternative<FuncExprPtr>(type1)) {
            if (std::holds_alternative<FuncExprPtr>(type2)) {
                return compareFuncSignatures(std::get<FuncExprPtr>(type1), std::get<FuncExprPtr>(type2));
            }
        }

        return false;
    }

    // checks signatures WITHOUT return type
    static bool compareFuncSignatures(const FuncExprPtr &func1, const FuncExprPtr &func2) {
        if (func1->params.size() != func2->params.size()) return false;
        for (int i = 0; i < func1->params.size(); i++) {
            if (!compareTypes(func1->params[i]->type, func2->params[i]->type)) return false;
            // if parameterType is None, it means that it is not specified and we can cast
            if (func1->params[i]->parameterType != None && func2->params[i]->parameterType != None) {
                if (func1->params[i]->parameterType != func2->params[i]->parameterType) return false;
            }
        }
        return true;
    }

    static bool compareFuncSignatures(const FuncDecStatementPtr &func1, const FuncDecStatementPtr &func2) {
        return compareFuncSignatures(func1->expr, func2->expr);
    }

    static bool compareFuncSignatures(const ExternFuncDecStmtPtr &func1, const ExternFuncDecStmtPtr &func2) {
        return compareFuncSignatures(func1->expr, func2->expr);
    }

    static bool compareFuncSignatures(const MethodDecPtr &func1, const MethodDecPtr &func2) {
        return compareFuncSignatures(func1->expr, func2->expr);
    }

    static bool compareFuncSignatures(const FuncDecStatementPtr &func1, const FuncExprPtr &func2) {
        return compareFuncSignatures(func1->expr, func2);
    }

    static std::string parameterTypeToString(ParameterType parameterType) {
        switch (parameterType) {
            case In:
                return "in ";
            case Out:
                return "out ";
            case Var:
                return "var ";
            case None:
                return "";
        }
    }

    static std::string typeToString(ExprPtrVariant type, ParameterType parameterType) {
        std::string result = parameterTypeToString(parameterType);
        if (auto typePtr = std::get_if<TypeExprPtr>(&type)) {
            return result + typePtr->get()->type;
        }
        if (auto typePtr = std::get_if<ArrayExprPtr>(&type)) {
            result += "array[] ";
            return result + typeToString(typePtr->get()->type);
        }
        if (std::holds_alternative<FuncExprPtr>(type)) {
            auto funcExpr = std::get<FuncExprPtr>(type);
            result += "function(";
            for (int i = 0; i < funcExpr->params.size(); i++) {
                result += typeToString(funcExpr->params[i]->type, funcExpr->params[i]->parameterType);
                if (i != funcExpr->params.size() - 1) result += ", ";
            }
            result += "): " + typeToString(funcExpr->type);
            return result;
        }
        return "unknown";
    }

    static std::string typeToString(TypeExprNode &type, ParameterType parameterType = ParameterType::None) {
        return parameterTypeToString(parameterType) + type.type;
    }
}


#endif //SLANGCREFACTORED_AST_HPP
