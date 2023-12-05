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
        auto irType = context.loadAsRvalue ? getIRType(type.value(), context) : getIRType(type.value(), context)->getPointerTo();
        return Constant::getNullValue(irType);
    }

    auto ArrayExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto BooleanExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return ConstantInt::get(Type::getInt1Ty(*context.llvmContext), value);
    }

    auto VarExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto var = context.localsLookup(name);
        std::optional<ExprPtrVariant> declType;
        bool isOut = false;
        bool isVar = false;
        bool isFunc = false;
        bool isCustomType = false;
        if (auto localVar = context.localsDeclsLookup(name)) {
            declType = getDeclType(localVar.value(), context.context, errors);
            if (std::holds_alternative<TypeExprPtr>(declType.value()) && context.allocatedClassesDecls.contains(std::get<TypeExprPtr>(declType.value())->type)) {
                isCustomType = true;
            }
            else
            if (auto funcParam = std::get_if<FuncParamDecStmtPtr>(&localVar.value())) {
                if (funcParam->get()->parameterType == Out) {
                    isOut = true;
                }
                else if (funcParam->get()->parameterType == Var) {
                    isVar = true;
                }
            }
        }
        else if (auto func = context.context.symbolTable.lookupFunc(name, context.currentFuncSignature.value(), context.context)) {
            declType = getDeclType(*func, context.context, errors);
            var = getFuncFromExpr(*func, context);
            isFunc = true;
        }
        auto type = getIRType(declType.value(), context);
        // TODO: it looks really really bad
        if ((context.loadAsRvalue && !isFunc) || isVar /*|| isCustomType*/) {
            if (isVar || isOut) type = type->getPointerTo();
            var = context.builder->CreateLoad(type, var);
            if (context.loadAsRvalue && isVar) {
                type = getIRType(declType.value(), context);
                var = context.builder->CreateLoad(type, var);
            }
        }
        return var;
    }

    auto IndexExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto temp = context.loadAsRvalue;
        context.loadAsRvalue = true;
        auto var = processNode(expr, context, errors);
        auto varType = std::get<ArrayExprPtr>(getExprType(expr, context.context, errors).value());
        auto indexVal = processNode(indexExpr, context, errors);
        indexVal = typeCast(indexVal, Type::getInt64Ty(*context.llvmContext), context, errors, getExprLoc(indexExpr));
        context.loadAsRvalue = false;
        var = context.builder->CreateInBoundsGEP(getIRType(varType->type, context), var, indexVal);
        if (temp) {
            var = context.builder->CreateLoad(getIRType(varType->type, context), var);
        }
        context.loadAsRvalue = temp;
        return var;
    }

    auto UnaryOperatorExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto temp = context.loadAsRvalue;
        context.loadAsRvalue = true;
        auto val = processNode(expr, context, errors);
        context.loadAsRvalue = temp;
        if (val->getType()->isIntegerTy())
            return context.builder->CreateNeg(val);
        if (val->getType()->isFloatingPointTy())
            return context.builder->CreateFNeg(val);
        return nullptr;
    }

    auto OperatorExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto temp = context.loadAsRvalue;
        context.loadAsRvalue = true;
        auto leftValue = processNode(left, context, errors);
        auto rightValue = processNode(right, context, errors);
        context.loadAsRvalue = temp;

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
                case TokenType::Remainder:
                    return context.builder->CreateFRem(leftValue, rightValue);
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
        Value* func = nullptr;
        auto temp = context.loadAsRvalue;
        std::vector<Value*> argsRef;
        size_t argsOffset = 0;
        if (foundFunc.has_value()) {
            func = getFuncFromExpr(foundFunc.value(), context);
            if (std::holds_alternative<MethodDecPtr>(foundFunc.value()) && std::holds_alternative<AccessExprPtr>(expr)) {
                auto method = std::get<MethodDecPtr>(foundFunc.value());
                context.loadAsRvalue = true;  //"this" is a custom type, so it should be loaded anyway, if it's in array, it should not be loaded after gep
                auto thisArg = processNode(std::get<AccessExprPtr>(expr)->expr, context, errors);
                argsRef.push_back(thisArg);
                argsOffset = 1;

                if (method->isVirtual) {
                    auto vtableLoad = context.builder->CreateLoad(context.builder->getPtrTy(), thisArg);
                    auto vtablePtr = context.builder->CreateInBoundsGEP(context.builder->getPtrTy(), vtableLoad, ConstantInt::get(Type::getInt64Ty(*context.llvmContext), method->vtableIndex), "vtable_" + method->name);
                    func = context.builder->CreateLoad(context.builder->getPtrTy(), vtablePtr);
                }
                context.loadAsRvalue = temp;
            }
        }
        else {
            context.loadAsRvalue = true;
            func = processNode(expr, context, errors);
        }

        for (size_t i = 0; i < args.size(); ++i) {
            auto currCallArg = args[i];
            auto currExpectedArg = funcType.value()->params[i + argsOffset];
            auto tempSig = context.currentFuncSignature;
            if (std::holds_alternative<FuncExprPtr>(currExpectedArg->type)) {
                context.currentFuncSignature = std::get<FuncExprPtr>(currExpectedArg->type);
            }
            auto isOutVar = currExpectedArg->parameterType == Out || currExpectedArg->parameterType == Var;
            context.loadAsRvalue = !isOutVar;
            auto exprType = getExprType(currCallArg, context.context, errors).value();
            // We should load custom types even if they are out or var, because they are saved as pointer to pointer
            if (std::holds_alternative<TypeExprPtr>(exprType) && context.allocatedClassesDecls.contains(std::get<TypeExprPtr>(exprType)->type)) {
                context.loadAsRvalue = true;
            }
            auto argVal = processNode(currCallArg, context, errors);
            auto currExpectedIRType = isOutVar ? getIRType(currExpectedArg->type, context)->getPointerTo() : getIRType(currExpectedArg->type, context);
            argVal = typeCast(argVal, currExpectedIRType, context, errors, getExprLoc(currCallArg));
            argsRef.push_back(argVal);
            context.currentFuncSignature = tempSig;
        }
        context.loadAsRvalue = temp;
        return context.builder->CreateCall(getFuncType(funcType.value(), context), func, argsRef);
    }

    auto AccessExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto temp = context.loadAsRvalue;
        context.loadAsRvalue = true;
        auto var = processNode(expr, context, errors);
        context.loadAsRvalue = temp;
        auto varType = getExprType(expr, context.context, errors).value();
        auto typeName = std::get<TypeExprPtr>(varType)->type;
        // TODO: next block looks weird, probably needs refactoring
        if (context.currentFuncSignature.has_value()) {
            auto method = selectBestOverload(context.allocatedClassesDecls[typeName], typeName + "." + name, context.currentFuncSignature.value(), true, true, context.context);
            if (method.has_value()) {
                auto methodDecl = std::get<MethodDecPtr>(method.value());
                auto methodName = methodDecl->name + "." + getMangledFuncName(methodDecl->expr);
                return context.module->getFunction(methodName);
            }
        }

        auto elementPtr = context.builder->CreateStructGEP(context.allocatedClasses[accessedType], var, index, typeName + "." + name);
        auto type = getIRType(getExprType(shared_from_this(), context.context, errors).value(), context);
        if (context.loadAsRvalue) {
            elementPtr = context.builder->CreateLoad(type, elementPtr);
        }
        return elementPtr;
    }

    auto DeleteStmtNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        context.loadAsRvalue = true;    // TODO: not sure if it's correct
        auto var = processNode(expr, context, errors);
        context.loadAsRvalue = false;
        return context.builder->CreateFree(var);
    }

    auto BlockStmtNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto AssignExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto ltype = getIRType(getExprType(left, context.context, errors).value(), context);
        auto rtype = getIRType(getExprType(right, context.context, errors).value(), context);
        auto leftVal = processNode(left, context, errors);
        context.loadAsRvalue = true;
        auto tempSig = context.currentFuncSignature;
        if (std::holds_alternative<FuncExprPtr>(getExprType(left, context.context, errors).value())) {
            context.loadAsRvalue = false;
            context.currentFuncSignature = std::get<FuncExprPtr>(getExprType(left, context.context, errors).value());
        }
        auto rightVal = processNode(right, context, errors);
        context.currentFuncSignature = tempSig;
        context.loadAsRvalue = false;
        if (ltype != rightVal->getType()) {
            rightVal = typeCast(rightVal, ltype, context, errors, getExprLoc(right));
        }
        return context.builder->CreateStore(rightVal, leftVal);
    }

    auto FuncParamDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto FuncExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto ReturnStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        context.loadAsRvalue = true;
        auto type = getExprType(expr, context.context, errors).value();
        auto val = processNode(expr, context, errors);
        context.loadAsRvalue = false;
        val = typeCast(val, context.currentReturnType, context, errors, getExprLoc(expr));
        return context.builder->CreateRet(val);
    }

    auto OutputStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        // printf
        auto printfFunc = context.module->getOrInsertFunction("printf", FunctionType::get(Type::getInt32Ty(*context.llvmContext), PointerType::get(Type::getInt8Ty(*context.llvmContext), 0), true));
        auto temp = context.loadAsRvalue;
        context.loadAsRvalue = true;
        auto val = processNode(expr, context, errors);
        context.loadAsRvalue = temp;
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
        else if (val->getType()->isIntegerTy()) {
            formatStr = context.builder->CreateGlobalStringPtr("%d\n");
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
        type = Context::isBuiltInType(typeExpr.type) ? type : type->getPointerTo();
        Value* var = nullptr;
        Value* rightVal = nullptr;
        if (type) {
            var = context.builder->CreateAlloca(type, nullptr, name);
            if (expr.has_value()) {
                context.loadAsRvalue = true;
                rightVal = processNode(expr.value(), context, errors);
                context.loadAsRvalue = false;
                // TODO: do type cast
                if (rightVal->getType() != type) {
                    rightVal = typeCast(rightVal, type, context, errors, getExprLoc(expr.value()));
                }
                context.builder->CreateStore(rightVal, var);
            }
            else if (!Context::isBuiltInType(typeExpr.type)) {
                createMalloc(typeExpr.type, var, context);
                // call constructor
                auto arg = context.builder->CreateLoad(type, var);
                auto constructor = context.module->getFunction(typeExpr.type + "._default_constructor");
                if (constructor) context.builder->CreateCall(constructor, arg);
            }
        }
        context.locals()[name] = var;
        context.localsDecls()[name] = shared_from_this();
        return var;
    }

    auto FuncPointerStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(expr, context);
        Value* var = context.builder->CreateAlloca(type, nullptr, name);
        context.locals()[name] = var;
        context.localsDecls()[name] = shared_from_this();
        if (assignExpr.has_value()) {
            auto temp = context.loadAsRvalue;
            context.loadAsRvalue = false;
            auto tempSig = context.currentFuncSignature;
            context.currentFuncSignature = expr;
            auto assignVal = processNode(assignExpr.value(), context, errors);
            context.currentFuncSignature = tempSig;
            context.loadAsRvalue = temp;
            context.builder->CreateStore(assignVal, var);
        }
        return var;
    }

    auto ArrayDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(expr, context);
        Value* var = context.builder->CreateAlloca(type, nullptr, name);
        context.locals()[name] = var;
        context.localsDecls()[name] = shared_from_this();
        if (assignExpr.has_value()) {
            context.loadAsRvalue = true;
            auto assignVal = processNode(assignExpr.value(), context, errors);
            context.loadAsRvalue = false;
            context.builder->CreateStore(assignVal, var);
        }
        else createArrayMalloc(expr, var, context, errors);
        return var;
    }

    auto FuncDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto funcType = getFuncType(expr, context);
        auto funcName = isExtern ? name : name + "." + getMangledFuncName(expr);
        auto funcCallee = context.module->getOrInsertFunction(funcName, funcType);
        auto func = context.module->getFunction(funcName);
        if (context.currentDeclImported || isExtern) return func;
        context.context.enterScope(funcName);   // scope name is mangled
        if (block.has_value()) {
            BasicBlock *block = BasicBlock::Create(*context.llvmContext, "entry", context.module->getFunction(funcName));
            context.pushBlock(block);
            context.builder->SetInsertPoint(block);
            Function::arg_iterator argsValues = func->arg_begin();
            for (auto param = expr->params.begin(); param != expr->params.end(); ++param, ++argsValues) {
                Value* var;
                if (param->get()->parameterType == Out || param->get()->parameterType == Var)
                    var = context.builder->CreateAlloca(getIRType(param->get()->type, context)->getPointerTo(), nullptr, param->get()->name);
                else
                    var = context.builder->CreateAlloca(getIRType(param->get()->type, context), nullptr, param->get()->name);
                context.locals()[param->get()->name] = var;
                context.localsDecls()[param->get()->name] = *param;
                argsValues->setName(parameterTypeToString(param->get()->parameterType) + param->get()->name);
                context.builder->CreateStore(argsValues, var);
            }
        }
        bool hasReturn = false;
        for (auto &&stmt : block.value()->statements) {
            context.currentReturnType = getIRType(expr->type, context);
            if (std::holds_alternative<FuncExprPtr>(expr->type))
                context.currentFuncSignature = std::get<FuncExprPtr>(expr->type);

            auto val = processNode(stmt, context, errors);
            if (std::holds_alternative<ReturnStatementPtr>(stmt)) {
                hasReturn = true;
                break;
            }
            context.currentFuncSignature = std::nullopt;
        }
        context.currentReturnType = nullptr;
        if (!isFunction) context.builder->CreateRetVoid();
        else {
            if (context.currentBlock()->empty()) {
                context.builder->CreateCall(context.module->getOrInsertFunction("llvm.trap", FunctionType::getVoidTy(*context.llvmContext)), {});
                context.builder->CreateUnreachable();
            } else {
                if (context.currentBlock()->getTerminator() == nullptr) {
                    context.builder->CreateCall(context.module->getOrInsertFunction("llvm.trap", FunctionType::getVoidTy(*context.llvmContext)), {});
                    context.builder->CreateUnreachable();
                }
            }
        }
        context.context.exitScope();
        context.popBlock();
        return {};
    }

    auto FieldVarDecNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(typeExpr.type, context);
        Value* rightVal = nullptr;
        auto elementPtr = context.builder->CreateStructGEP(context.allocatedClasses[typeName.type], context.currentTypeLoad, index, typeName.type + "." + name);
        if (!Context::isBuiltInType(typeExpr.type)) {
            auto malloc = createMalloc(typeExpr.type, elementPtr, context);
            context.builder->CreateStore(malloc, elementPtr);
            // calling a constructor can cause an infinite recursion (A calls constr of B, B calls constr of A...)
            // TODO: make calling of constructor implicit, like variable-A a = new A;
            auto arg = context.builder->CreateLoad(type, elementPtr);
            auto constructor = context.module->getFunction(typeExpr.type + "._default_constructor");
            if (constructor) context.builder->CreateCall(constructor, arg);
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
        auto elementPtr = context.builder->CreateStructGEP(context.allocatedClasses[typeName.type], context.currentTypeLoad, index, typeName.type + "." + name);

        if (assignExpr.has_value()) {
            context.loadAsRvalue = true;
            auto assignVal = processNode(assignExpr.value(), context, errors);
            context.loadAsRvalue = false;
            context.builder->CreateStore(assignVal, elementPtr);
        }
        else createArrayMalloc(expr, elementPtr, context, errors);
        return elementPtr;
    }

    auto FieldFuncPointerStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(expr, context);
        auto elementPtr = context.builder->CreateStructGEP(context.allocatedClasses[typeName.type], context.currentTypeLoad, index, typeName.type + "." + name);
        if (assignExpr.has_value()) {
            auto temp = context.loadAsRvalue;
            context.loadAsRvalue = false;
            context.currentFuncSignature = expr;
            auto assignVal = processNode(assignExpr.value(), context, errors);
            context.currentFuncSignature = std::nullopt;
            context.loadAsRvalue = temp;
            context.builder->CreateStore(assignVal, elementPtr);
        }
        return elementPtr;
    }

    auto MethodDecNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto temp = create<FuncDecStatementNode>(loc, name, expr, block, true, isFunction, false);
        auto res = temp->codegen(context, errors);
        return res;
    }

    auto TypeDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (name == "Object") return nullptr;
        context.allocatedClasses[name] = StructType::create(*context.llvmContext, name);
        context.allocatedClasses[name]->setName(name);
        context.allocatedClassesDecls[name] = shared_from_this();
        std::vector<Type*> types;
        size_t index = 0;
        if (parentTypeName != "Object") {
            types.push_back(context.allocatedClasses.at(parentTypeName.value()));
            ++index;
        }
        // vtable pointer stored only in base class
        if (vtableRequired && parentTypeName == "Object") {
            types.push_back(PointerType::get(StructType::create(*context.llvmContext, "vtable_" + name), 0));
        }
        for (; index < fields.size(); ++index) {
            types.push_back(getIRType(getDeclType(fields[index], context.context, errors).value(), context));
        }
        context.allocatedClasses[name]->setBody(types);
        // create definition of default constructor
        //createDefaultConstructor(this, context, errors, true);
        context.context.enterScope(name);
        for (auto &method : methods) {
            method->codegen(context, errors);
        }

        if (vtableRequired) createVTable(this, context, errors);
        createDefaultConstructor(this, context, errors, context.currentDeclImported);
        context.context.exitScope();
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
        auto temp = context.loadAsRvalue;
        context.loadAsRvalue = true;
        context.builder->CreateCondBr(processNode(condition, context, errors), trueBB, falseBB);
        context.loadAsRvalue = temp;
        context.pushBlock(trueBB);
        context.builder->SetInsertPoint(trueBB);
        context.context.enterScope("if#" + std::to_string(loc.line) + ":" + std::to_string(loc.column));
        bool hasReturn = false;
        for (auto &&stmt : trueBlock->statements) {
            auto val = processNode(stmt, context, errors);
            if (std::holds_alternative<ReturnStatementPtr>(stmt)) hasReturn = true;
        }
        if (!hasReturn) context.builder->CreateBr(endBB);
        context.popBlock();
        context.context.exitScope();

        context.pushBlock(falseBB);
        context.builder->SetInsertPoint(falseBB);

        //elseif blocks
        for (auto &&elseif : elseIfNodes) {
            auto elseifBBTrue = BasicBlock::Create(*context.llvmContext, "elseifthen", func);
            auto elseifBBFalse = BasicBlock::Create(*context.llvmContext, "elseifelse", func);
            context.loadAsRvalue = true;
            context.builder->CreateCondBr(processNode(elseif->condition, context, errors), elseifBBTrue, elseifBBFalse);
            context.loadAsRvalue = temp;
            context.pushBlock(elseifBBTrue);
            context.builder->SetInsertPoint(elseifBBTrue);
            hasReturn = false;
            context.context.enterScope("else-if#" + std::to_string(elseif->loc.line) + ":" + std::to_string(elseif->loc.column));
            for (auto &&stmt : elseif->trueBlock->statements) {
                auto val = processNode(stmt, context, errors);
                if (std::holds_alternative<ReturnStatementPtr>(stmt)) hasReturn = true;
            }
            if (!hasReturn) context.builder->CreateBr(endBB);
            context.popBlock();
            context.context.exitScope();
            context.pushBlock(elseifBBFalse);
            context.builder->SetInsertPoint(elseifBBFalse);
        }

        // else block
        if (falseBlock.has_value()) {
            hasReturn = false;
            context.context.enterScope("else#" + std::to_string(loc.line) + ":" + std::to_string(loc.column));
            for (auto &&stmt : falseBlock.value()->statements) {
                auto val = processNode(stmt, context, errors);
                if (std::holds_alternative<ReturnStatementPtr>(stmt)) hasReturn = true;
            }
            context.context.exitScope();
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
        auto temp = context.loadAsRvalue;
        context.loadAsRvalue = true;
        context.builder->CreateCondBr(processNode(condition, context, errors), whileIter, whileEnd);
        context.loadAsRvalue = temp;
        context.popBlock();
        context.pushBlock(whileIter);
        context.builder->SetInsertPoint(whileIter);
        bool hasReturn = false;
        context.context.enterScope("while#" + std::to_string(loc.line) + ":" + std::to_string(loc.column));
        for (auto &&stmt : block->statements) {
            auto val = processNode(stmt, context, errors);
            if (std::holds_alternative<ReturnStatementPtr>(stmt)) hasReturn = true;
        }
        context.context.exitScope();
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