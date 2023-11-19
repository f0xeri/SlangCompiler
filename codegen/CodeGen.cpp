//
// Created by f0xeri on 05.05.2023.
//

// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppMemberFunctionMayBeStatic
#include <iostream>
#include "CodeGen.hpp"
#include "parser/AST.hpp"


namespace Slangc {
    using namespace llvm;
    auto IntExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantInt::get(Type::getInt32Ty(*context.llvmContext), value, true);
    }

    auto FloatExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantFP::get(Type::getDoubleTy(*context.llvmContext), value);
    }

    auto RealExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantFP::get(Type::getDoubleTy(*context.llvmContext), value);
    }

    auto CharExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantInt::get(Type::getInt8Ty(*context.llvmContext), value);
    }

    auto StringExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantDataArray::getString(*context.llvmContext, value);
    }

    auto TypeExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto NilExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto ArrayExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto BooleanExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto VarExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto IndexExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto IndexesExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto UnaryOperatorExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto OperatorExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto CallExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto AccessExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto DeleteStmtNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto BlockStmtNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto AssignExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto FuncParamDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto FuncExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto ReturnStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto OutputStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto InputStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto VarDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = typeOf(typeToString(typeExpr), context);
        Value* rightVal = nullptr;
        if (type) {
            auto var = context.builder->CreateAlloca(type, nullptr, name);
            if (expr) {
                rightVal = processNode(expr.value(), context, errors);
                if (rightVal) context.builder->CreateStore(rightVal, var);
            }
        }
        return nullptr;
    }

    auto FuncPointerStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto ArrayDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto FuncDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto FieldVarDecNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto FieldArrayVarDecNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto FieldFuncPointerStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto MethodDecNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto TypeDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        context.allocatedClasses[name] = StructType::create(*context.llvmContext, name);
        context.allocatedClasses[name]->setName(name);
        std::vector<Type*> types;
        for (auto &field : fields) {
            types.push_back(typeOf(typeToString(getDeclType(field, context.context, errors).value()), context));
        }
        context.allocatedClasses[name]->setBody(types);
        return nullptr;
    }

    auto ElseIfStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto IfStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto WhileStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto ModuleDeclNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

} // Slangc