//
// Created by f0xeri on 05.05.2023.
//

#ifndef SLANGCREFACTORED_ASTBOX_HPP
#define SLANGCREFACTORED_ASTBOX_HPP

#include <utility>
#include <variant>
#include "common.hpp"
#include "lexer/TokenType.hpp"

namespace Slangc::ASTBox {
    enum ParameterType
    {
        In,
        Out,
        Var
    };

    using ExprNode = std::variant<
            Box<struct ArrayExprNode>,
            Box<struct BlockExprNode>,
            Box<struct BooleanExprNode>,
            Box<struct CharExprNode>,
            Box<struct FloatExprNode>,
            Box<struct FuncExprNode>,
            Box<struct IntExprNode>,
            Box<struct NilExprNode>,
            Box<struct OperatorExprNode>,
            Box<struct RealExprNode>,
            Box<struct StringExprNode>,
            Box<struct UnaryOperatorExprNode>,
            Box<struct VarExprNode>,
            Box<struct IndexesExprNode>,
            Box<struct IndexExprNode>,
            Box<struct CallExprNode>
    >;

    using VariableExprNode = std::variant<
            Box<struct IndexExprNode>,
            Box<struct IndexesExprNode>,
            Box<struct VarExprNode>
    >;

    using DeclarationNode = std::variant<
            Box<struct ArrayDecStatementNode>,
            Box<struct ExternFuncDecStatementNode>,
            Box<struct FieldArrayVarDecNode>,
            Box<struct FieldVarDecNode>,
            Box<struct FuncDecStatementNode>,
            Box<struct FuncParamDecStatementNode>,
            Box<struct FuncPointerStatementNode>,
            Box<struct MethodDecNode>,
            Box<struct TypeDecStatementNode>,
            Box<struct VarDecStatementNode>
    >;

    using StatementNode = std::variant<
            Box<struct AssignExprNode>,
            Box<struct CallExprNode>,
            Box<struct ArrayDecStatementNode>,
            Box<struct ExternFuncDecStatementNode>,
            Box<struct FieldArrayVarDecNode>,
            Box<struct FieldVarDecNode>,
            Box<struct FuncDecStatementNode>,
            Box<struct FuncParamDecStatementNode>,
            Box<struct FuncPointerStatementNode>,
            Box<struct MethodDecNode>,
            Box<struct TypeDecStatementNode>,
            Box<struct VarDecStatementNode>,
            Box<struct DeleteExprNode>,
            Box<struct ElseIfStatementNode>,
            Box<struct IfStatementNode>,
            Box<struct InputStatementNode>,
            Box<struct ModuleStatementNode>,
            Box<struct OutputStatementNode>,
            Box<struct ReturnStatementNode>,
            Box<struct VarExprNode>,
            Box<struct IndexesExprNode>,
            Box<struct IndexExprNode>,
            Box<struct WhileStatementNode>
    >;


    struct IntExprNode {
        int value;
        bool isConst = true;
    };

    struct FloatExprNode {
        float value;
        bool isConst = true;
    };

    struct RealExprNode {
        double value;
        bool isConst = true;
    };

    struct CharExprNode {
        char value;
        bool isConst = true;
    };

    struct StringExprNode {
        std::string value;
        bool isConst = true;
    };

    struct NilExprNode {
        ExprNode expr;
        bool isConst = true;
    };

    struct ArrayExprNode {
        std::vector<ExprNode> values;
        std::string type;
        ExprNode size;
        bool isConst = false;
    };

    struct BooleanExprNode {
        bool value{};
        bool isConst = true;
    };

    struct VarExprNode {
        std::string value;
        bool dotClass = false;
        bool dotModule = false;
        bool isPointer = false;
        int index = -1;

        bool isConst = false;
    };

    struct IndexExprNode {
        ExprNode indexExpr;
        ExprNode assign;

        bool isConst = false;
    };

    struct IndexesExprNode {
        std::vector<ExprNode> indexes;
        ExprNode assign;

        bool isConst = false;
    };

    struct UnaryOperatorExprNode {
        TokenType op{};
        ExprNode expr;
        bool isConst = false;
    };

    struct OperatorExprNode {
        TokenType op{};
        ExprNode left, right;
        bool isConst = false;
    };

    struct CallExprNode {
        VariableExprNode name;
        std::vector<ExprNode> args;

        bool isConst = false;

        //CallExprNode(const Box<VarExprNode>& name, std::vector<ExprNode> args): name(name), args(std::move(args)) {}
    };

    struct DeleteExprNode {
        ExprNode expr;
    };

    struct BlockExprNode {
        std::vector<StatementNode> statements;
        bool isConst = false;
    };

    struct AssignExprNode {
        VariableExprNode left;
        ExprNode right;
    };

    struct FuncParamDecStatementNode {
        VariableExprNode name;
        ParameterType parameterType{};
        ExprNode type;
        ExprNode expr;
    };

    struct FuncExprNode {
        ExprNode type;
        bool isFunction = true;
        std::vector<Box<FuncParamDecStatementNode>> params;

        bool isConst = false;
    };

    struct ReturnStatementNode {
        ExprNode expr;
    };

    struct OutputStatementNode {
        ExprNode expr;
    };

    struct InputStatementNode {
        ExprNode expr;
    };

    struct VarDecStatementNode {
        VariableExprNode name;
        std::string type;
        ExprNode expr;
        bool isGlobal = false;
        bool isPrivate = false;
        bool isExtern = false;
    };

    struct FuncPointerStatementNode {
        VariableExprNode name;
        ExprNode type;
        std::vector<Box<FuncParamDecStatementNode>> args;
        bool isFunction = false;
        bool isGlobal = false;
        bool isPrivate = false;
        bool isExtern = false;
        ExprNode expr;
    };

    struct ArrayDecStatementNode {
        VariableExprNode name;
        bool isGlobal = false;
        bool isPrivate = false;
        Box<ArrayExprNode> expr;
        ExprNode assignExpr;
        int indicesCount = 1;
        bool isExtern = false;

        bool isConst = false;
    };

    struct FuncDecStatementNode {
        VariableExprNode name;
        ExprNode type;
        std::vector<Box<FuncParamDecStatementNode>> args;
        Box<BlockExprNode> block;
        bool isPrivate = false;
        bool isFunction = false;
    };

    struct FieldVarDecNode {
        VariableExprNode name;
        std::string typeName;
        bool isPrivate;
        int index;
        std::string type;
        ExprNode expr;
    };

    struct FieldArrayVarDecNode {
        std::string typeName;
        bool isPrivate;
        int index;
        Box<ArrayDecStatementNode> var;
    };

    struct MethodDecNode {
        ExprNode type;
        VariableExprNode thisName;
        std::vector<Box<FuncParamDecStatementNode>> args;
        Box<BlockExprNode> block;
        bool isPrivate = false;
        bool isFunction = false;
    };

    struct TypeDecStatementNode {
        VariableExprNode name;
        std::vector<DeclarationNode> fields;
        std::vector<Box<MethodDecNode>> methods;
        Box<TypeDecStatementNode> parentType;
        bool isExtern = false;
        bool isPrivate = false;
    };

    struct ExternFuncDecStatementNode {
        VariableExprNode name;
        ExprNode type;
        std::vector<Box<FuncParamDecStatementNode>> args;
        bool isPrivate = false;
        bool isFunction = false;
    };

    struct ElseIfStatementNode {
        ExprNode condExpr;
        Box<BlockExprNode> trueBlock;
    };

    struct IfStatementNode {
        ExprNode condExpr;
        Box<BlockExprNode> trueBlock;
        Box<BlockExprNode> falseBlock;
        std::vector<Box<ElseIfStatementNode>> elseIfNodes;
    };

    struct WhileStatementNode {
        ExprNode whileExpr;
        Box<BlockExprNode> block;
    };

    struct ModuleStatementNode {
        VariableExprNode name;
        Box<BlockExprNode> block;
    };
}

#endif //SLANGCREFACTORED_ASTBOX_HPP
