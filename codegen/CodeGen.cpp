//
// Created by f0xeri on 05.05.2023.
//

// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppLocalVariableMayBeConst
#include <iostream>
#include "CodeGen.hpp"
#include "parser/AST.hpp"


namespace Slangc {
    using namespace llvm;
    auto IntExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantInt::get(Type::getInt32Ty(*context.llvmContext), value, true);
    }

    auto FloatExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantFP::get(Type::getFloatTy(*context.llvmContext), value);
    }

    auto RealExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantFP::get(Type::getDoubleTy(*context.llvmContext), value);
    }

    auto CharExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantInt::get(Type::getInt8Ty(*context.llvmContext), value);
    }

    auto StringExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return context.builder->CreateGlobalStringPtr(value);
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
        return ConstantInt::get(Type::getInt1Ty(*context.llvmContext), value);
    }

    auto VarExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto var = context.localsLookup(name);
        auto declType = getDeclType(context.localsDeclsLookup(name), context.context, errors);
        auto type = getIRType(getDeclType(context.localsDeclsLookup(name), context.context, errors).value(), context);
        if (Context::isBuiltInType(typeToString(declType.value())))
            return context.builder->CreateLoad(type, var);
        return var;
    }

    auto IndexExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto IndexesExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto UnaryOperatorExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto val = processNode(expr, context, errors);
        if (val->getType()->isIntegerTy())
            return context.builder->CreateNeg(val);
        if (val->getType()->isFloatingPointTy())
            return context.builder->CreateFNeg(val);
        return nullptr;
    }

    auto OperatorExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto leftValue = processNode(left, context, errors);
        auto rightValue = processNode(right, context, errors);

        if (leftValue->getType() != rightValue->getType()) {
            rightValue = typeCast(rightValue, leftValue->getType(), context, errors, getExprLoc(left));
        }

        if (!leftValue || !rightValue) return nullptr;
        if (leftValue->getType()->isIntegerTy() && rightValue->getType()->isIntegerTy()) {
            switch (op) {
                case TokenType::Plus:
                    return context.builder->CreateAdd(leftValue, rightValue);
                case TokenType::Minus:
                    return context.builder->CreateSub(leftValue, rightValue);
                case TokenType::Multiplication:
                    return context.builder->CreateMul(leftValue, rightValue);
                case TokenType::Division:
                    return context.builder->CreateSDiv(leftValue, rightValue);
                case TokenType::Remainder:
                    return context.builder->CreateSRem(leftValue, rightValue);
                case TokenType::Equal:
                    return context.builder->CreateICmpEQ(leftValue, rightValue);
                case TokenType::NotEqual:
                    return context.builder->CreateICmpNE(leftValue, rightValue);
                case TokenType::Less:
                    return context.builder->CreateICmpSLT(leftValue, rightValue);
                case TokenType::LessOrEqual:
                    return context.builder->CreateICmpSLE(leftValue, rightValue);
                case TokenType::Greater:
                    return context.builder->CreateICmpSGT(leftValue, rightValue);
                case TokenType::GreaterOrEqual:
                    return context.builder->CreateICmpSGE(leftValue, rightValue);
                case TokenType::And:
                    return context.builder->CreateAnd(leftValue, rightValue);
                case TokenType::Or:
                    return context.builder->CreateOr(leftValue, rightValue);
            }
        }
        else {
            switch (op) {
                case TokenType::Plus:
                    return context.builder->CreateFAdd(leftValue, rightValue);
                case TokenType::Minus:
                    return context.builder->CreateFSub(leftValue, rightValue);
                case TokenType::Multiplication:
                    return context.builder->CreateFMul(leftValue, rightValue);
                case TokenType::Division:
                    return context.builder->CreateFDiv(leftValue, rightValue);
                case TokenType::Equal:
                    return context.builder->CreateFCmpOEQ(leftValue, rightValue);
                case TokenType::NotEqual:
                    return context.builder->CreateFCmpONE(leftValue, rightValue);
                case TokenType::Less:
                    return context.builder->CreateFCmpOLT(leftValue, rightValue);
                case TokenType::LessOrEqual:
                    return context.builder->CreateFCmpOLE(leftValue, rightValue);
                case TokenType::Greater:
                    return context.builder->CreateFCmpOGT(leftValue, rightValue);
                case TokenType::GreaterOrEqual:
                    return context.builder->CreateFCmpOGE(leftValue, rightValue);
            }
        }
        return nullptr;
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
        auto rightVal = processNode(right, context, errors);
        Value* leftVal;
        if (auto leftExpr = std::get_if<VarExprPtr>(&left)) {
            leftVal = context.localsLookup(leftExpr->get()->name);
            return context.builder->CreateStore(rightVal, leftVal);
        }
        return nullptr;
    }

    auto FuncParamDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto FuncExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto ReturnStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return context.builder->CreateRet(processNode(expr, context, errors));
    }

    auto OutputStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        // printf
        auto printfFunc = context.module->getOrInsertFunction("printf", FunctionType::get(Type::getInt32Ty(*context.llvmContext), PointerType::get(Type::getInt8Ty(*context.llvmContext), 0), true));
        auto val = processNode(expr, context, errors);
        bool charArray = false;
        auto exprType = getExprType(expr, context.context, errors).value();
        if (auto arr = std::get_if<ArrayExprPtr>(&exprType)) {
            if (auto type = std::get_if<TypeExprPtr>(&arr->get()->type)) {
                if (type->get()->type == "character") {
                    charArray = true;
                }
            }
        }
        if (auto arr = std::get_if<StringExprPtr>(&expr)) charArray = true;

        std::vector<Value*> printArgs;
        Value *formatStr;
        if (val->getType()->isFloatingPointTy()) {
            val = context.builder->CreateFPExt(val, Type::getDoubleTy(*context.llvmContext));
            formatStr = context.builder->CreateGlobalStringPtr("%f\n");
        }
        else if (val->getType()->isPointerTy() && charArray) {
            formatStr = context.builder->CreateGlobalStringPtr("%s\n");
        }
        else if (val->getType()->isPointerTy()) {
            formatStr = context.builder->CreateGlobalStringPtr("%p\n");
        }
        else if (val->getType()->isIntegerTy(8)) {
            formatStr = context.builder->CreateGlobalStringPtr("%c\n");
        }
        else if (val->getType()->isIntegerTy(1)) {
            formatStr = context.builder->CreateGlobalStringPtr("%s\n");
        }
        else {
            formatStr = context.builder->CreateGlobalStringPtr("%s\n");
        }
        printArgs.push_back(formatStr);
        printArgs.push_back(val);
        return context.builder->CreateCall(printfFunc, printArgs);
    }

    auto InputStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto VarDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(typeExpr.type, context);
        Value* var = nullptr;
        Value* rightVal = nullptr;
        if (type) {
            if (Context::isBuiltInType(typeExpr.type)) {
                var = context.builder->CreateAlloca(type, nullptr, name);
                if (expr) {
                    rightVal = processNode(expr.value(), context, errors);
                    if (rightVal->getType() != type) {
                        rightVal = typeCast(rightVal, type, context, errors, getExprLoc(expr.value()));
                    }
                    context.builder->CreateStore(rightVal, var);
                }
            }
            else {
                var = context.builder->CreateAlloca(type->getPointerTo(), nullptr, name);
                auto malloc = createMalloc(typeExpr.type, var, context);
            }
        }
        context.locals()[name] = var;
        context.localsDecls()[name] = shared_from_this();
        return var;
    }

    auto FuncPointerStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto ArrayDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(expr, context);
        auto var = context.builder->CreateAlloca(type, nullptr, name);
        createArrayMalloc(expr, var, context, errors);
        context.locals()[name] = var;
        context.localsDecls()[name] = shared_from_this();
        return var;
    }

    auto FuncDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto FieldVarDecNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(typeExpr.type, context);
        Value* var = nullptr;
        Value* rightVal = nullptr;
        auto elementPtr = context.builder->CreateStructGEP(context.allocatedClasses[typeName.type], context.currentTypeLoad, index, typeName.type + "." + name);
        if (!Context::isBuiltInType(typeExpr.type)) {
            var = context.builder->CreateAlloca(type->getPointerTo(), nullptr, name);
            auto malloc = createMalloc(typeExpr.type, var, context);
            // calling a constructor can cause an infinite recursion (A calls constr of B, B calls constr of A...)
            // TODO: make calling of constructor implicit, like variable-A a = new A;
        }
        if (expr.has_value()) {
            rightVal = processNode(expr.value(), context, errors);
            if (rightVal->getType() != type) {
                rightVal = typeCast(rightVal, type, context, errors, getExprLoc(expr.value()));
            }
            return context.builder->CreateStore(rightVal, elementPtr);
        }
        return elementPtr;
    }

    auto FieldArrayVarDecNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(expr, context);
        auto var = context.builder->CreateAlloca(type, nullptr, name);
        auto elementPtr = context.builder->CreateStructGEP(context.allocatedClasses[typeName.type], context.currentTypeLoad, index, typeName.type + "." + name);


        return elementPtr;
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
            types.push_back(getIRType(getDeclType(field, context.context, errors).value(), context));
        }
        context.allocatedClasses[name]->setBody(types);
        createDefaultConstructor(this, context, errors);
        return nullptr;
    }

    auto ElseIfStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto IfStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto func = context.builder->GetInsertBlock()->getParent();
        auto trueBB = BasicBlock::Create(*context.llvmContext, "then", func);
        auto falseBB = BasicBlock::Create(*context.llvmContext, "else", func);
        auto endBB = BasicBlock::Create(*context.llvmContext, "ifend", func);

        context.builder->CreateCondBr(processNode(condition, context, errors), trueBB, falseBB);
        context.pushBlock(trueBB);
        context.builder->SetInsertPoint(trueBB);
        bool hasReturn = false;
        for (auto &&stmt : trueBlock->statements) {
            auto val = processNode(stmt, context, errors);
            if (std::holds_alternative<ReturnStatementPtr>(stmt)) hasReturn = true;
        }
        if (!hasReturn) context.builder->CreateBr(endBB);
        context.popBlock();

        context.pushBlock(falseBB);
        context.builder->SetInsertPoint(falseBB);
        //elseif blocks
        for (auto &&elseif : elseIfNodes) {
            auto elseifBBTrue = BasicBlock::Create(*context.llvmContext, "elseifthen", func);
            auto elseifBBFalse = BasicBlock::Create(*context.llvmContext, "elseifelse", func);
            context.builder->CreateCondBr(processNode(elseif->condition, context, errors), elseifBBTrue, elseifBBFalse);
            context.pushBlock(elseifBBTrue);
            context.builder->SetInsertPoint(elseifBBTrue);
            hasReturn = false;
            for (auto &&stmt : elseif->trueBlock->statements) {
                auto val = processNode(stmt, context, errors);
                if (std::holds_alternative<ReturnStatementPtr>(stmt)) hasReturn = true;
            }
            if (!hasReturn) context.builder->CreateBr(endBB);
            context.popBlock();
            context.pushBlock(elseifBBFalse);
            context.builder->SetInsertPoint(elseifBBFalse);
        }

        // else block
        if (falseBlock.has_value()) {
            hasReturn = false;
            for (auto &&stmt : falseBlock.value()->statements) {
                auto val = processNode(stmt, context, errors);
                if (std::holds_alternative<ReturnStatementPtr>(stmt)) hasReturn = true;
            }
            if (!hasReturn) context.builder->CreateBr(endBB);
        }
        else {
            context.builder->CreateBr(endBB);
        }
        for (auto &elseif : elseIfNodes) {
            context.popBlock();
        }
        context.popBlock();
        context.ret(endBB);
        context.builder->SetInsertPoint(endBB);
        return endBB;
    }

    auto WhileStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto func = context.builder->GetInsertBlock()->getParent();
        auto whileCheck = BasicBlock::Create(*context.llvmContext, "whilecheck", func);
        auto whileIter = BasicBlock::Create(*context.llvmContext, "whileiter", func);
        auto whileEnd = BasicBlock::Create(*context.llvmContext, "whileend", func);
        context.builder->CreateBr(whileCheck);
        context.pushBlock(whileCheck);
        context.builder->SetInsertPoint(whileCheck);
        context.builder->CreateCondBr(processNode(condition, context, errors), whileIter, whileEnd);
        context.popBlock();
        context.pushBlock(whileIter);
        context.builder->SetInsertPoint(whileIter);
        bool hasReturn = false;
        for (auto &&stmt : block->statements) {
            auto val = processNode(stmt, context, errors);
            if (std::holds_alternative<ReturnStatementPtr>(stmt)) hasReturn = true;
        }
        if (!hasReturn) context.builder->CreateBr(whileCheck);
        context.popBlock();
        context.ret(whileEnd);
        context.builder->SetInsertPoint(whileEnd);
        return whileEnd;
    }

    auto ModuleDeclNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

} // Slangc