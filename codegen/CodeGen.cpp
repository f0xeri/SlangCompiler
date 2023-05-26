//
// Created by f0xeri on 05.05.2023.
//

#include <iostream>
#include "CodeGen.hpp"
#include "parser/AST.hpp"


namespace Slangc {

    auto IntExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto FloatExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto RealExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto CharExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto StringExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto NilExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto ArrayExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto BooleanExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto VarExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto IndexExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto IndexesExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto UnaryOperatorExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto OperatorExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto CallExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto AccessExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto DeleteStmtNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto BlockStmtNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto AssignExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto FuncParamDecStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto FuncExprNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto ReturnStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto OutputStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto InputStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto VarDecStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto FuncPointerStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto ArrayDecStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto FuncDecStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto FieldVarDecNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto FieldArrayVarDecNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto MethodDecNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto TypeDecStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto ExternFuncDecStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto ElseIfStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto IfStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto WhileStatementNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

    auto ModuleDeclNode::codegen(CodeGenContext &context) -> std::shared_ptr<llvm::Value> {
        return {};
    }

} // Slangc