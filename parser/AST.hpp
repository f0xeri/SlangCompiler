//
// Created by f0xeri on 05.05.2023.
//

#ifndef SLANGCREFACTORED_AST_HPP
#define SLANGCREFACTORED_AST_HPP

#include <utility>
#include <variant>
#include <memory>
#include <vector>
#include <optional>
#include "llvm/IR/Value.h"
#include "lexer/TokenType.hpp"
#include "codegen/CodeGenContext.hpp"
#include "ASTFwdDecl.hpp"
#include "analysis/BasicAnalysis.hpp"

namespace Slangc {
    enum ParameterType {
        In,
        Out,
        Var
    };

    struct ExprTypeVisitor {
        const BasicAnalysis &analysis;
        auto operator()(const auto &x) const -> Type {
            return x->getType(analysis);
        }
    };

    static auto getExprType(const ExprPtrVariant &expr, const BasicAnalysis &analysis) -> Type {
        return std::visit(ExprTypeVisitor{analysis}, expr);
    }

    struct DeclTypeVisitor {
        const BasicAnalysis &analysis;
        auto operator()(const auto &x) const -> Type {
            return x->getType(analysis);
        }
    };

    static auto getDeclType(const DeclPtrVariant &expr, const BasicAnalysis &analysis) -> Type {
        return std::visit(DeclTypeVisitor{analysis}, expr);
    }

#pragma region Nodes declarations
    // codegen methods are defined in codegen/CodeGen.cpp

    struct TypeExprNode {
        SourceLoc loc{0, 0};
        std::string type;
        bool isConst = true;

        TypeExprNode(SourceLoc loc, std::string type) : loc(loc), type(std::move(type)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return {type}; }
    };

    struct IntExprNode {
        SourceLoc loc{0, 0};
        int value{};
        bool isConst = true;

        IntExprNode(SourceLoc loc, int value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        Type getType(const BasicAnalysis& analysis)  { return {"integer"}; }
    };

    struct FloatExprNode {
        SourceLoc loc{0, 0};
        float value{};
        bool isConst = true;

        FloatExprNode(SourceLoc loc, float value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return {"float"}; }
    };

    struct RealExprNode {
        SourceLoc loc{0, 0};
        double value{};
        bool isConst = true;

        RealExprNode(SourceLoc loc, double value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return {"real"}; }
    };

    struct CharExprNode {
        SourceLoc loc{0, 0};
        char value{};
        bool isConst = true;

        CharExprNode(SourceLoc loc, char value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return {"character"}; }
    };

    struct StringExprNode {
        SourceLoc loc{0, 0};
        std::string value;
        bool isConst = true;

        StringExprNode(SourceLoc loc, std::string value) : loc(loc), value(std::move(value)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return {"array"}; }
    };

    struct NilExprNode {
        SourceLoc loc{0, 0};
        Type type;
        bool isConst = true;

        NilExprNode(SourceLoc loc, Type type) : loc(loc), type(type) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return type; }
    };

    struct ArrayExprNode {
        SourceLoc loc{0, 0};
        std::vector<ExprPtrVariant> values;
        std::string type;
        ExprPtrVariant size;
        bool isConst = false;

        ArrayExprNode(SourceLoc loc, std::vector<ExprPtrVariant> values, std::string type, ExprPtrVariant size) :
            loc(loc), values(std::move(values)), type(std::move(type)), size(std::move(size)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const BasicAnalysis& analysis) -> Type { return {"array"}; }
    };

    struct BooleanExprNode {
        SourceLoc loc{0, 0};
        bool value{};
        bool isConst = true;

        BooleanExprNode(SourceLoc loc, bool value) : loc(loc), value(value) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const BasicAnalysis& analysis) -> Type { return {"boolean"}; }
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

        auto getType(const BasicAnalysis& analysis) -> Type { return getDeclType(*analysis.lookup(name), analysis); }
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

        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(expr, analysis); }
    };

    struct IndexesExprNode {
        SourceLoc loc{0, 0};
        std::vector<ExprPtrVariant> indexes;
        std::optional<ExprPtrVariant> assign = std::nullopt;
        bool isConst = false;

        IndexesExprNode(SourceLoc loc, std::vector<ExprPtrVariant> indexes, std::optional<ExprPtrVariant> assign = std::nullopt)
            : loc(loc), indexes(std::move(indexes)), assign(std::move(assign)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const BasicAnalysis& analysis) -> Type { return {"-"}; }
    };

    struct UnaryOperatorExprNode {
        SourceLoc loc{0, 0};
        TokenType op{};
        ExprPtrVariant expr;
        bool isConst = false;

        UnaryOperatorExprNode(SourceLoc loc, TokenType op, ExprPtrVariant expr)
            : loc(loc), op(op), expr(std::move(expr)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(expr, analysis); }
    };

    struct OperatorExprNode {
        SourceLoc loc{0, 0};
        TokenType op{};
        ExprPtrVariant left, right;
        bool isConst = false;

        OperatorExprNode(SourceLoc loc, TokenType op, ExprPtrVariant left, ExprPtrVariant right)
            : loc(loc), op(op), left(std::move(left)), right(std::move(right)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(left, analysis); }
    };

    struct CallExprNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant name;        // TODO: change to VarExprPtrVariant?
        std::vector<ExprPtrVariant> args;
        bool isConst = false;

        CallExprNode(SourceLoc loc, ExprPtrVariant name, std::vector<ExprPtrVariant> args)
            : loc(loc), name(std::move(name)), args(std::move(args)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(name, analysis); }
    };

    struct AccessExprNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant expr;
        std::string name;
        bool isConst = false;

        AccessExprNode(SourceLoc loc, ExprPtrVariant expr, std::string name)
            : loc(loc), expr(std::move(expr)), name(std::move(name)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;

        auto getType(const BasicAnalysis& analysis) -> Type { return {"-"}; }
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
        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(type, analysis); }
    };

    struct FuncExprNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant type;
        bool isFunction = true;
        std::vector<FuncParamDecStmtPtr> params;
        bool isConst = false;

        FuncExprNode(SourceLoc loc, ExprPtrVariant type, bool isFunction, std::vector<FuncParamDecStmtPtr> params)
            : loc(loc), type(std::move(type)), isFunction(isFunction), params(std::move(params)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(type, analysis); }
    };

    struct ReturnStatementNode {
        SourceLoc loc{0, 0};
        std::optional<ExprPtrVariant> expr;

        ReturnStatementNode(SourceLoc loc, std::optional<ExprPtrVariant> expr) : loc(loc), expr(std::move(expr)) {};

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
        std::string type;
        std::optional<ExprPtrVariant> expr;
        bool isGlobal = false;
        bool isPrivate = false;
        bool isExtern = false;

        VarDecStatementNode(SourceLoc loc, std::string name, std::string type, std::optional<ExprPtrVariant> expr = std::nullopt, bool isGlobal = false, bool isPrivate = false, bool isExtern = false)
            : loc(loc), name(std::move(name)), type(std::move(type)), expr(std::move(expr)), isGlobal(isGlobal), isPrivate(isPrivate), isExtern(isExtern) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return {type}; }
    };

    struct FuncPointerStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        ExprPtrVariant type;
        std::vector<FuncParamDecStmtPtr> args;
        bool isFunction = false;
        bool isGlobal = false;
        bool isPrivate = false;
        bool isExtern = false;
        ExprPtrVariant expr;

        FuncPointerStatementNode(SourceLoc loc, std::string name, ExprPtrVariant type, std::vector<FuncParamDecStmtPtr> args,
                                 ExprPtrVariant expr, bool isFunction = false, bool isGlobal = false, bool isPrivate = false, bool isExtern = false)
            : loc(loc), name(std::move(name)), type(std::move(type)), args(std::move(args)), expr(std::move(expr)),
            isFunction(isFunction), isGlobal(isGlobal), isPrivate(isPrivate), isExtern(isExtern) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(type, analysis); }
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
        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(expr, analysis); }
    };

    struct FuncDecStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        ExprPtrVariant type;
        std::vector<FuncParamDecStmtPtr> args;
        std::optional<BlockStmtPtr> block;
        bool isPrivate = false;
        bool isFunction = false;

        FuncDecStatementNode(SourceLoc loc, std::string name, ExprPtrVariant type, std::vector<FuncParamDecStmtPtr> args, std::optional<BlockStmtPtr> block = std::nullopt, bool isPrivate = false, bool isFunction = false)
            : loc(loc), name(std::move(name)), type(std::move(type)), args(std::move(args)), block(std::move(block)), isPrivate(isPrivate), isFunction(isFunction) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(type, analysis); }
    };

    struct FieldVarDecNode {
        SourceLoc loc{0, 0};
        std::string name;
        std::string typeName;
        bool isPrivate;
        int index;
        std::string type;
        ExprPtrVariant expr;

        FieldVarDecNode(SourceLoc loc, std::string name, std::string typeName, bool isPrivate, int index, std::string type, ExprPtrVariant expr)
            : loc(loc), name(std::move(name)), typeName(std::move(typeName)), isPrivate(isPrivate), index(index), type(std::move(type)), expr(std::move(expr)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return {type}; }
    };

    struct FieldArrayVarDecNode {
        SourceLoc loc{0, 0};
        std::string name;
        std::string typeName;
        bool isPrivate;
        int index;
        ArrayDecStatementPtr var;

        FieldArrayVarDecNode(SourceLoc loc, std::string name, std::string typeName, bool isPrivate, int index, ArrayDecStatementPtr var)
            : loc(loc), name(std::move(name)), typeName(std::move(typeName)), isPrivate(isPrivate), index(index), var(std::move(var)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(var->expr, analysis); }
    };

    struct MethodDecNode {
        SourceLoc loc{0, 0};
        std::string name;
        ExprPtrVariant type;
        std::string thisName;
        std::vector<FuncParamDecStmtPtr> args;
        BlockStmtPtr block;
        bool isPrivate = false;
        bool isFunction = false;

        MethodDecNode(SourceLoc loc, std::string name, ExprPtrVariant type, std::string thisName, std::vector<FuncParamDecStmtPtr> args,
                      BlockStmtPtr block, bool isPrivate = false, bool isFunction = false)
            : loc(loc), name(std::move(name)), type(std::move(type)), thisName(std::move(thisName)), args(std::move(args)),
              block(std::move(block)), isPrivate(isPrivate), isFunction(isFunction) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(type, analysis); }
    };

    struct TypeDecStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        std::vector<DeclPtrVariant> fields;
        std::vector<MethodDecPtr> methods;
        std::optional<TypeDecStatementPtr> parentType = std::nullopt;
        bool isExtern = false;
        bool isPrivate = false;

        TypeDecStatementNode(SourceLoc loc, std::string name, std::vector<DeclPtrVariant> fields,
                             std::vector<MethodDecPtr> methods, std::optional<TypeDecStatementPtr> parentType = std::nullopt,
                             bool isExtern = false, bool isPrivate = false)
                             : loc(loc), name(std::move(name)), fields(std::move(fields)), methods(std::move(methods)),
                               parentType(std::move(parentType)), isExtern(isExtern), isPrivate(isPrivate) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return {name}; }
    };

    struct ExternFuncDecStatementNode {
        SourceLoc loc{0, 0};
        std::string name;
        ExprPtrVariant type;
        std::vector<FuncParamDecStmtPtr> args;
        bool isPrivate = false;
        bool isFunction = false;

        ExternFuncDecStatementNode(SourceLoc loc, std::string name, ExprPtrVariant type, std::vector<FuncParamDecStmtPtr> args,
                                   bool isPrivate = false, bool isFunction = false)
                                   : loc(loc), name(std::move(name)), type(std::move(type)), args(std::move(args)),
                                     isPrivate(isPrivate), isFunction(isFunction) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return getExprType(type, analysis); }
    };

    struct ElseIfStatementNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant condExpr;
        BlockStmtPtr trueBlock;

        ElseIfStatementNode(SourceLoc loc, ExprPtrVariant condExpr, BlockStmtPtr trueBlock)
            : loc(loc), condExpr(std::move(condExpr)), trueBlock(std::move(trueBlock)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct IfStatementNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant condExpr;
        BlockStmtPtr trueBlock;
        BlockStmtPtr falseBlock;
        std::vector<ElseIfStatementPtr> elseIfNodes;

        IfStatementNode(SourceLoc loc, ExprPtrVariant condExpr, BlockStmtPtr trueBlock, BlockStmtPtr falseBlock,
                        std::vector<ElseIfStatementPtr> elseIfNodes)
            : loc(loc), condExpr(std::move(condExpr)), trueBlock(std::move(trueBlock)),
            falseBlock(std::move(falseBlock)), elseIfNodes(std::move(elseIfNodes)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct WhileStatementNode {
        SourceLoc loc{0, 0};
        ExprPtrVariant whileExpr;
        BlockStmtPtr block;

        WhileStatementNode(SourceLoc loc, ExprPtrVariant whileExpr, BlockStmtPtr block)
            : loc(loc), whileExpr(std::move(whileExpr)), block(std::move(block)) {};
        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
    };

    struct ModuleDeclNode {
        SourceLoc loc{0, 0};
        std::string name;
        BlockStmtPtr block;

        ModuleDeclNode(SourceLoc loc, std::string name, BlockStmtPtr block)
            : loc(loc), name(std::move(name)), block(std::move(block)) {};

        auto codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value>;
        auto getType(const BasicAnalysis& analysis) -> Type { return {name}; }
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

    static auto getArrayFinalType(ArrayExprNode* exprNode, const BasicAnalysis &analysis) -> Type {
        if (exprNode->type != "array") return {exprNode->type, true, false, 1};
        Type type;
        for (auto &slice : exprNode->values) {
                type = getExprType(slice, analysis);
        }
        return type;
    }
}


#endif //SLANGCREFACTORED_AST_HPP
