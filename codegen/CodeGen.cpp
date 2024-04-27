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
        if (context.debug) context.debugBuilder->emitLocation(loc);
        return ConstantInt::get(Type::getInt32Ty(*context.llvmContext), value, true);
    }

    auto FloatExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        return ConstantFP::get(Type::getFloatTy(*context.llvmContext), value);
    }

    auto RealExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        return ConstantFP::get(Type::getDoubleTy(*context.llvmContext), value);
    }

    auto CharExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        return ConstantInt::get(Type::getInt8Ty(*context.llvmContext), value);
    }

    auto StringExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto str = context.builder->CreateGlobalStringPtr(value, "", 0, context.module.get());
        return str;
        /* TODO: malloc strings is a good idea if we want to assign to array of chars, but bad in other scenarios like output
           do we need malloc if we pass string to func? */
        /*auto size = ConstantInt::get(Type::getInt32Ty(*context.llvmContext), value.size());
        auto charPtr = PointerType::get(Type::getInt8Ty(*context.llvmContext), 0);
        auto buffer = context.builder->CreateMalloc(Type::getInt32Ty(*context.llvmContext), charPtr, size, nullptr, context.mallocFunc);
        FunctionType *strcpyType = FunctionType::get(charPtr, {charPtr, charPtr}, false);
        auto strcpyFunc = context.module->getOrInsertFunction("strcpy", strcpyType);
        auto call = context.builder->CreateCall(strcpyFunc, {buffer, str});
        return buffer;*/
    }

    auto FormattedStringExprNode::codegen(Slangc::CodeGenContext &context, std::vector<ErrorMessage> &errors) -> llvm::Value * {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        // llvm ir code do something like this:
        /*
                size_t needed = snprintf(NULL, 0, "%s: %s (%d)", msg, strerror(errno), errno) + 1;
                char  *buffer = malloc(needed);
                sprintf(buffer, "%s: %s (%d)", msg, strerror(errno), errno);
                return buffer;
         */
        // int snprintf (char* s, size_t n, const char* format, ...);
        auto snprintfFunc = context.module->getOrInsertFunction(
                "snprintf",
                FunctionType::get(
                        Type::getInt32Ty(*context.llvmContext),{
                                PointerType::get(Type::getInt8Ty(*context.llvmContext), 0),
                                Type::getInt64Ty(*context.llvmContext),
                                PointerType::get(Type::getInt8Ty(*context.llvmContext), 0)
                        },
                        true
                )
        );

        std::vector<Value*> args;
        std::vector<Type*> argTypes;
        Value* formatString = nullptr;
        std::string fmtString;
        // build format string and args
        for (auto& arg: values) {
            auto temp = context.loadValue;
            Value* val = nullptr;

            auto exprType = getExprType(arg, context.context, errors).value();
            bool charArray = false;
            if (auto arr = std::get_if<ArrayExprPtr>(&exprType)) {
                if (auto type = std::get_if<TypeExprPtr>(&arr->get()->type)) {
                    if (type->get()->type == getBuiltInTypeName(BuiltInType::Char)) {
                        charArray = true;
                    }
                }
            }
            if (auto arr = std::get_if<StringExprPtr>(&arg)) {
                // do not malloc string literals here, we will create new string anyway
                val = context.builder->CreateGlobalStringPtr(arr->get()->value, "", 0, context.module.get());
                charArray = true;
            }
            else {
                context.loadValue = true;
                val = processNode(arg, context, errors);
                context.loadValue = temp;
            }

            if (val->getType()->isFloatingPointTy()) {
                val = context.builder->CreateFPExt(val, Type::getDoubleTy(*context.llvmContext));
                fmtString +="%f";
            }
            else if (val->getType()->isPointerTy() && charArray) {
                fmtString += "%s";
            }
            else if (val->getType()->isPointerTy()) {
                fmtString += "%p";
            }
            else if (val->getType()->isIntegerTy(8)) {
                fmtString += "%c";
            }
            else if (val->getType()->isIntegerTy(1)) {
                val = context.builder->CreateIntCast(val, Type::getInt32Ty(*context.llvmContext), false);
                fmtString += "%d";
            }
            else if (val->getType()->isIntegerTy()) {
                fmtString += "%d";
            }
            else {
                fmtString += "%s";
            }

            args.push_back(val);
            argTypes.push_back(val->getType());
        }
        formatString = context.builder->CreateGlobalStringPtr(fmtString, "", 0, context.module.get());
        args.insert(args.begin(), formatString);
        args.insert(args.begin(), ConstantInt::get(Type::getInt64Ty(*context.llvmContext), 0));
        args.insert(args.begin(), ConstantPointerNull::get(PointerType::get(Type::getInt8Ty(*context.llvmContext), 0)));

        auto needed = context.builder->CreateCall(snprintfFunc, args);
        auto neededPlusOne = context.builder->CreateAdd(needed, ConstantInt::get(Type::getInt32Ty(*context.llvmContext), 1));
        auto buffer = context.builder->CreateMalloc(Type::getInt32Ty(*context.llvmContext), Type::getInt8Ty(*context.llvmContext), neededPlusOne, nullptr, context.mallocFunc);
        args[0] = buffer;
        args[1] = context.builder->CreateIntCast(neededPlusOne, Type::getInt64Ty(*context.llvmContext), false);
        auto call = context.builder->CreateCall(snprintfFunc, args);
        return buffer;
    }

    auto TypeExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto NilExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto irType = context.loadValue ? getIRType(type.value(), context) : getIRType(type.value(), context)->getPointerTo();
        return irType->isVoidTy() ? Constant::getNullValue(context.builder->getPtrTy()) : Constant::getNullValue(irType);
    }

    auto ArrayExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto BooleanExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        return ConstantInt::get(Type::getInt1Ty(*context.llvmContext), value);
    }

    auto VarExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto var = context.localsLookup(name);
        if (!var) var = context.globals()[name];
        std::optional<ExprPtrVariant> declType;
        bool isOut = false;
        bool isVar = false;
        bool isFunc = false;

        if (auto localVar = context.localsDeclsLookup(name)) {
            if (context.referencing) {
                context.setReferenced(name, true);
            }
            declType = getDeclType(localVar.value(), context.context, errors);
            if (auto funcParam = std::get_if<FuncParamDecStmtPtr>(&localVar.value())) {
                if (funcParam->get()->parameterType == Out) {
                    isOut = true;
                }
                else if (funcParam->get()->parameterType == Var) {
                    isVar = true;
                }
            }
        }
        else if (auto globalVar = context.globalsDeclsLookup(name)) {
            declType = getDeclType(globalVar.value(), context.context, errors);
            if (auto funcParam = std::get_if<FuncParamDecStmtPtr>(&globalVar.value())) {
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
        if ((context.loadValue && !isFunc) || isVar) {
            if (isVar || isOut)
                type = getIRPtrType(declType.value(), context);
            var = context.builder->CreateLoad(type, var);
            if (context.loadValue && isVar) {
                type = getIRType(declType.value(), context);
                var = context.builder->CreateLoad(type, var);
            }
        }
        return var;
    }

    auto IndexExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto temp = context.loadValue;
        context.loadValue = true;
        auto var = processNode(expr, context, errors);
        auto varType = std::get<ArrayExprPtr>(getExprType(expr, context.context, errors).value());
        auto indexVal = processNode(indexExpr, context, errors);
        indexVal = typeCast(indexVal, Type::getInt64Ty(*context.llvmContext), context, errors, getExprLoc(indexExpr));
        context.loadValue = false;
        var = context.builder->CreateInBoundsGEP(getIRType(varType->type, context), var, indexVal);
        if (temp) {
            var = context.builder->CreateLoad(getIRType(varType->type, context), var);
        }
        context.loadValue = temp;
        return var;
    }

    auto UnaryOperatorExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto temp = context.loadValue;
        context.loadValue = true;
        auto val = processNode(expr, context, errors);
        context.loadValue = temp;
        if (val->getType()->isIntegerTy())
            return context.builder->CreateNeg(val);
        if (val->getType()->isFloatingPointTy())
            return context.builder->CreateFNeg(val);
        return nullptr;
    }

    auto OperatorExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto temp = context.loadValue;
        context.loadValue = true;
        auto leftValue = processNode(left, context, errors);
        auto rightValue = processNode(right, context, errors);
        context.loadValue = temp;

        if (leftValue->getType() != rightValue->getType()) {
            auto leftType = typeToString(getExprType(left, context.context, errors).value());
            auto rightType = typeToString(getExprType(right, context.context, errors).value());
            if (Context::isCastToLeft(leftType, rightType, context.context)) {
                rightValue = typeCast(rightValue, leftValue->getType(), context, errors, getExprLoc(right));
            }
            else {
                leftValue = typeCast(leftValue, rightValue->getType(), context, errors, getExprLoc(left));
            }
        }

        if (!leftValue || !rightValue) return nullptr;
        // TODO: pointers can be compared only with == and !=, not with other operators
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
        else if (leftValue->getType()->isPointerTy() && rightValue->getType()->isPointerTy()) {
            switch (op) {
                case TokenType::Equal:
                    return context.builder->CreateICmpEQ(leftValue, rightValue);
                case TokenType::NotEqual:
                    return context.builder->CreateICmpNE(leftValue, rightValue);
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
        if (context.debug) context.debugBuilder->emitLocation(loc);
        Value* func = nullptr;
        auto temp = context.loadValue;
        std::vector<Value*> argsRef;
        size_t argsOffset = 0;
        if (foundFunc.has_value()) {
            func = getFuncFromExpr(foundFunc.value(), context);
            if (std::holds_alternative<MethodDecPtr>(foundFunc.value()) && std::holds_alternative<AccessExprPtr>(expr)) {
                auto method = std::get<MethodDecPtr>(foundFunc.value());
                context.loadValue = true;  //"this" is a custom type, so it should be loaded anyway, if it's in array, it should not be loaded after gep
                auto thisArg = processNode(std::get<AccessExprPtr>(expr)->expr, context, errors);
                argsRef.push_back(thisArg);
                argsOffset = 1;

                if (method->isVirtual) {
                    auto vtableLoad = context.builder->CreateLoad(context.builder->getPtrTy(), thisArg);
                    auto vtablePtr = context.builder->CreateInBoundsGEP(context.builder->getPtrTy(), vtableLoad, ConstantInt::get(Type::getInt64Ty(*context.llvmContext), method->vtableIndex), "vtable_" + method->name);
                    func = context.builder->CreateLoad(context.builder->getPtrTy(), vtablePtr);
                }
                context.loadValue = temp;
            }
        }
        else {
            context.loadValue = true;
            func = processNode(expr, context, errors);
        }

        for (size_t i = 0; i < args.size(); ++i) {
            auto currCallArg = args[i];
            auto currExpectedArg = funcType.value()->params[i + argsOffset];
            auto tempSig = context.currentFuncSignature;
            if (std::holds_alternative<FuncExprPtr>(currExpectedArg->type)) {
                context.currentFuncSignature = std::get<FuncExprPtr>(currExpectedArg->type);
            }
            auto exprType = getExprType(currCallArg, context.context, errors).value();
            auto isOutVar = currExpectedArg->parameterType == Out || currExpectedArg->parameterType == Var;
            bool isVoid = std::holds_alternative<TypeExprPtr>(currExpectedArg->type) && std::get<TypeExprPtr>(currExpectedArg->type)->type == getBuiltInTypeName(BuiltInType::Void);

            bool isArgCustomType = std::holds_alternative<TypeExprPtr>(exprType) && context.allocatedClassesDecls.contains(std::get<TypeExprPtr>(exprType)->type);
            context.loadValue = !isOutVar;

            // if func argument is void* and not build-in type, load it - TODO: check if it's correct
            if (!(std::holds_alternative<TypeExprPtr>(exprType) && Context::isBuiltInType(std::get<TypeExprPtr>(exprType)->type))) {
                if (isVoid && isOutVar)
                    context.loadValue = true;
            }

            // We should load custom types even if they are out or var, because they are saved as pointer to pointer
            if (isArgCustomType) {
                context.loadValue = true;
            }
            auto argVal = processNode(currCallArg, context, errors);

            auto currExpectedIRType = isOutVar ? getIRPtrType(currExpectedArg->type, context) : getIRType(currExpectedArg->type, context);
            // if func argument is void*, we should not try to cast
            if (!(isVoid && isOutVar)) argVal = typeCast(argVal, currExpectedIRType, context, errors, getExprLoc(currCallArg));
            argsRef.push_back(argVal);
            context.currentFuncSignature = tempSig;
        }
        context.loadValue = temp;
        return context.builder->CreateCall(getFuncType(funcType.value(), context), func, argsRef);
    }

    auto NewExprNode::codegen(Slangc::CodeGenContext &context, std::vector<ErrorMessage> &errors) -> llvm::Value * {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto irType = getIRType(type, context);
        Value* val = context.builder->CreateAlloca(irType);
        if (auto arr = std::get_if<ArrayExprPtr>(&type)) {
            createArrayMalloc(*arr, val, context, errors);
        }
        else if (auto typeExpr = std::get_if<TypeExprPtr>(&type)) {
            if (!Context::isBuiltInType(typeExpr->get()->type)) {
                createMalloc(typeExpr->get()->type, val, context);
                auto arg = context.builder->CreateLoad(irType, val);
                auto constructor = context.module->getFunction(typeExpr->get()->type + "._default_constructor");
                if (constructor) context.builder->CreateCall(constructor, arg);
            }
        }
        val = context.builder->CreateLoad(irType, val);
        return val;
    }

    auto AccessExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto temp = context.loadValue;
        context.loadValue = true;
        auto var = processNode(expr, context, errors);
        context.loadValue = temp;
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
        if (context.loadValue) {
            elementPtr = context.builder->CreateLoad(type, elementPtr);
        }
        return elementPtr;
    }

    auto DeleteStmtNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        context.loadValue = false;    // TODO: not sure if it's correct
        auto var = processNode(expr, context, errors);
        context.loadValue = false;

        auto type = getExprType(expr, context.context, errors).value();
        if (auto typeExpr = std::get_if<TypeExprPtr>(&type)) {
            cleanupVar(typeExpr->get()->type, var, context, errors);
        }
        else if (auto arr = std::get_if<ArrayExprPtr>(&type)) {
            createArrayFree(*arr, var, context, errors);
            return var;
        }
        else {
            context.builder->CreateCall(context.freeFunc, var);
            auto notLoadedVar = processNode(expr, context, errors);
            return context.builder->CreateStore(Constant::getNullValue(notLoadedVar->getType()), notLoadedVar);
        }
    }

    auto BlockStmtNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto AssignExprNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto ltype = getIRType(getExprType(left, context.context, errors).value(), context);
        auto rtype = getIRType(getExprType(right, context.context, errors).value(), context);
        auto leftVal = processNode(left, context, errors);
        context.loadValue = true;
        auto tempSig = context.currentFuncSignature;
        if (std::holds_alternative<FuncExprPtr>(getExprType(left, context.context, errors).value())) {
            context.loadValue = false;
            context.currentFuncSignature = std::get<FuncExprPtr>(getExprType(left, context.context, errors).value());
        }
        context.referencing = true;
        auto rightVal = processNode(right, context, errors);
        context.referencing = false;
        // if right is new, clean up left
        /*if (auto newExpr = std::get_if<NewExprPtr>(&right)) {
            if (auto arr = std::get_if<ArrayExprPtr>(&newExpr->get()->type)) {
                createArrayFree(*arr, leftVal, context, errors);
            }
            else if (auto typeExpr = std::get_if<TypeExprPtr>(&newExpr->get()->type)) {
                cleanupVar(typeExpr->get()->type, leftVal, context, errors);
            }
        }*/
        context.currentFuncSignature = tempSig;
        context.loadValue = false;
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
        if (context.debug) context.debugBuilder->emitLocation(loc);
        if (std::holds_alternative<TypeExprPtr>(expr) && std::get<TypeExprPtr>(expr)->type == getBuiltInTypeName(BuiltInType::Void))
            return context.builder->CreateRetVoid();
        context.loadValue = true;
        context.referencing = true;
        auto type = getExprType(expr, context.context, errors).value();
        auto val = processNode(expr, context, errors);
        context.referencing = false;
        context.loadValue = false;
        val = typeCast(val, context.currentReturnType, context, errors, getExprLoc(expr));
        // cleanupCurrentScope(context, errors);
        return context.builder->CreateRet(val);
    }

    auto OutputStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto printfFunc = context.module->getOrInsertFunction("printf", FunctionType::get(Type::getInt32Ty(*context.llvmContext), PointerType::get(Type::getInt8Ty(*context.llvmContext), 0), true));
        auto temp = context.loadValue;
        bool charArray = false;
        auto exprType = getExprType(expr, context.context, errors).value();
        if (auto arr = std::get_if<ArrayExprPtr>(&exprType)) {
            if (auto type = std::get_if<TypeExprPtr>(&arr->get()->type)) {
                if (type->get()->type == getBuiltInTypeName(BuiltInType::Char)) {
                    charArray = true;
                }
            }
        }
        Value* val = nullptr;
        // we don't want to malloc string in output statement
        if (auto strLiteral = std::get_if<StringExprPtr>(&expr))
            val = context.builder->CreateGlobalStringPtr(strLiteral->get()->value, "", 0, context.module.get());
        else {
            context.loadValue = true;
            val = processNode(expr, context, errors);
            context.loadValue = temp;
        }
        if (auto arr = std::get_if<StringExprPtr>(&expr)) charArray = true;

        std::vector<Value*> printArgs;
        Value *formatStr;
        if (val->getType()->isFloatingPointTy()) {
            val = context.builder->CreateFPExt(val, Type::getDoubleTy(*context.llvmContext));
            formatStr = context.builder->CreateGlobalStringPtr("%f");
        }
        else if (val->getType()->isPointerTy() && charArray) {
            formatStr = context.builder->CreateGlobalStringPtr("%s");
        }
        else if (val->getType()->isPointerTy()) {
            formatStr = context.builder->CreateGlobalStringPtr("%p");
        }
        else if (val->getType()->isIntegerTy(8)) {
            formatStr = context.builder->CreateGlobalStringPtr("%c");
        }
        else if (val->getType()->isIntegerTy(1)) {
            val = context.builder->CreateIntCast(val, Type::getInt32Ty(*context.llvmContext), false);
            formatStr = context.builder->CreateGlobalStringPtr("%d");
        }
        else if (val->getType()->isIntegerTy()) {
            formatStr = context.builder->CreateGlobalStringPtr("%d");
        }
        else {
            formatStr = context.builder->CreateGlobalStringPtr("%s");
        }
        printArgs.push_back(formatStr);
        printArgs.push_back(val);
        auto printfCall = context.builder->CreateCall(printfFunc, printArgs);

        // deallocate formatted string
        if (auto fmtString = std::get_if<FormattedStringExprPtr>(&expr)) {
            context.builder->CreateCall(context.freeFunc, val);
        }

        return printfCall;
    }

    auto InputStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto scanfFunc = context.module->getOrInsertFunction("scanf", FunctionType::get(Type::getInt32Ty(*context.llvmContext), PointerType::get(Type::getInt8Ty(*context.llvmContext), 0), true));
        auto temp = context.loadValue;
        context.loadValue = false;
        auto val = processNode(expr, context, errors);
        context.loadValue = temp;
        bool charArray = false;
        auto exprType = getExprType(expr, context.context, errors).value();
        if (auto arr = std::get_if<ArrayExprPtr>(&exprType)) {
            if (auto type = std::get_if<TypeExprPtr>(&arr->get()->type)) {
                if (type->get()->type == getBuiltInTypeName(BuiltInType::Char)) {
                    charArray = true;
                    val = context.builder->CreateLoad(PointerType::get(*context.llvmContext, 0), val);
                }
            }
        }
        std::string type = "";
        if (auto typeExpr = std::get_if<TypeExprPtr>(&exprType)) {
            type = typeExpr->get()->type;
        }

        std::vector<Value*> printArgs;
        Value *formatStr;
        if (type == getBuiltInTypeName(BuiltInType::Real)) {
            formatStr = context.builder->CreateGlobalStringPtr("%lf");
        }
        else if (type == getBuiltInTypeName(BuiltInType::Float)) {
            formatStr = context.builder->CreateGlobalStringPtr("%f");
        }
        else if (charArray) {
            formatStr = context.builder->CreateGlobalStringPtr(" %[^\n]s");
        }
        else if (type == getBuiltInTypeName(BuiltInType::Char)) {
            formatStr = context.builder->CreateGlobalStringPtr(" %c");
        }
        else if (type == getBuiltInTypeName(BuiltInType::Bool)) {
            formatStr = context.builder->CreateGlobalStringPtr("%d");
        }
        else if (type == getBuiltInTypeName(BuiltInType::Int)) {
            formatStr = context.builder->CreateGlobalStringPtr("%d");
        }
        else {
            formatStr = context.builder->CreateGlobalStringPtr("%s");
        }
        printArgs.push_back(formatStr);
        printArgs.push_back(val);
        return context.builder->CreateCall(scanfFunc, printArgs);
    }

    auto VarDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(typeExpr.type, context);
        type = (Context::isBuiltInType(typeExpr.type) && typeExpr.type != getBuiltInTypeName(BuiltInType::Void)) ? type : getIRPtrType(typeExpr.type, context);
        Value* var = nullptr;
        Value* rightVal = nullptr;
        if (!isGlobal) {
            if (context.debug) context.debugBuilder->emitLocation(loc);
            var = context.builder->CreateAlloca(type, nullptr, name);
            if (expr.has_value()) {
                context.loadValue = true;
                context.referencing = true;
                rightVal = processNode(expr.value(), context, errors);
                context.referencing = false;
                context.loadValue = false;
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
            context.locals()[name] = var;
            context.localsDecls()[name] = shared_from_this();
            context.referenced()[name] = false;
            if (context.debug) {
                context.debugBuilder->createLocalVar(name, getDebugType(typeExpr.type, context), var, loc);
            }
        }
        else if (isGlobal) {
            bool tempDebugFlag = context.debug;
            context.debug = false;
            auto global = new GlobalVariable(*context.module, type, false, GlobalValue::InternalLinkage, nullptr, name);
            global->setDSOLocal(true);
            if (isExtern) global->setLinkage(GlobalValue::ExternalLinkage);
            if (!isExtern) {
                if (expr.has_value()) {
                    context.loadValue = true;
                    rightVal = processNode(expr.value(), context, errors);
                    context.loadValue = false;
                    global->setInitializer(dyn_cast<Constant>(rightVal));
                }
                else {
                    global->setInitializer(Constant::getNullValue(type));
                }
            }
            if (!Context::isBuiltInType(typeExpr.type)) {
                auto globalCtorFuncCallee = context.module->getOrInsertFunction(name + "._global_constructor", FunctionType::get(Type::getVoidTy(*context.llvmContext), false));
                auto globalCtorFunc = context.module->getFunction(name + "._global_constructor");
                auto block = BasicBlock::Create(*context.llvmContext, "entry", globalCtorFunc);
                context.pushBlock(block);
                context.builder->SetInsertPoint(block);
                getOrCreateSanitizerCtorAndInitFunctions(
                        *context.module, context.context.moduleName + "__GLOBAL__" + name,
                        name + "._global_constructor", {}, {},
                        [&](Function *Ctor, FunctionCallee) { appendToGlobalCtors(*context.module, Ctor, 65535); });

                createMalloc(typeExpr.type, global, context);
                auto arg = context.builder->CreateLoad(type, global);
                auto constructor = context.module->getFunction(typeExpr.type + "._default_constructor");
                if (constructor) context.builder->CreateCall(constructor, arg);
                context.builder->CreateRetVoid();
                context.popBlock();
            }
            context.globals()[name] = global;
            context.globalsDecls()[name] = shared_from_this();
            context.debug = tempDebugFlag;
            if (context.debug) {
                auto dbgInfo = context.debugBuilder->createGlobalVar(name, getDebugType(typeExpr.type, context), loc, isPrivate);
                global->addDebugInfo(dbgInfo);
            }
            var = global;
        }
        return var;
    }

    auto FuncPointerStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(expr, context);
        Value* var = nullptr;
        Value* assignValue = nullptr;
        if (assignExpr.has_value()) {
            auto temp = context.loadValue;
            context.loadValue = false;
            auto tempSig = context.currentFuncSignature;
            context.currentFuncSignature = expr;
            assignValue = processNode(assignExpr.value(), context, errors);
            context.currentFuncSignature = tempSig;
            context.loadValue = temp;
        }
        if (!isGlobal) {
            if (context.debug) context.debugBuilder->emitLocation(loc);
            var = context.builder->CreateAlloca(type, nullptr, name);
            context.locals()[name] = var;
            context.localsDecls()[name] = shared_from_this();
            context.referenced()[name] = false;
            if (assignExpr.has_value())
                context.builder->CreateStore(assignValue, var);
            if (context.debug) {
                context.debugBuilder->createLocalVar(name, getDebugType(expr, context, errors), var, loc);
            }
        }
        else {
            auto global = new GlobalVariable(*context.module, type, false, GlobalValue::InternalLinkage, nullptr, name);
            global->setDSOLocal(true);
            if (isExtern) global->setLinkage(GlobalValue::ExternalLinkage);
            if (!isExtern) {
                if (assignExpr.has_value()) {
                    global->setInitializer(dyn_cast<Constant>(assignValue));
                }
                else {
                    global->setInitializer(Constant::getNullValue(type));
                }
            }
            context.globals()[name] = global;
            context.globalsDecls()[name] = shared_from_this();
            var = global;
            if (context.debug) {
                auto dbgInfo = context.debugBuilder->createGlobalVar(name, getDebugType(expr, context, errors), loc, isPrivate);
                global->addDebugInfo(dbgInfo);
            }
        }
        return var;
    }

    auto ArrayDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        auto type = getIRType(expr, context);
        Value* var = nullptr;
        if (!isGlobal) {
            if (context.debug) context.debugBuilder->emitLocation(loc);
            var = context.builder->CreateAlloca(type, nullptr, name);
            context.locals()[name] = var;
            context.localsDecls()[name] = shared_from_this();
            context.referenced()[name] = false;
            if (context.debug) {
                context.debugBuilder->createLocalVar(name, getDebugType(expr, context, errors), var, loc);
            }
            if (assignExpr.has_value()) {
                context.loadValue = true;
                context.referencing = true;
                auto assignVal = processNode(assignExpr.value(), context, errors);
                context.referencing = false;
                context.loadValue = false;
                context.builder->CreateStore(assignVal, var);
                //context.referenced()[name] = true;  // if string literal assigned, we should not try to clear this variable
            }
            else createArrayMalloc(expr, var, context, errors);
        }
        else {
            bool tempDebugFlag = context.debug;
            context.debug = false;
            auto global = new GlobalVariable(*context.module, type, false, GlobalValue::InternalLinkage, nullptr, name);
            global->setDSOLocal(true);
            if (isExtern) global->setLinkage(GlobalValue::ExternalLinkage);
            if (!isExtern) {
                global->setInitializer(Constant::getNullValue(type));
                auto globalCtorFuncCallee = context.module->getOrInsertFunction(name + "._global_constructor", FunctionType::get(Type::getVoidTy(*context.llvmContext), false));
                auto globalCtorFunc = context.module->getFunction(name + "._global_constructor");
                auto block = BasicBlock::Create(*context.llvmContext, "entry", globalCtorFunc);
                context.pushBlock(block);
                context.builder->SetInsertPoint(block);
                getOrCreateSanitizerCtorAndInitFunctions(
                        *context.module, context.context.moduleName + "__GLOBAL__" + name,
                        name + "._global_constructor", {}, {},
                        [&](Function *Ctor, FunctionCallee) { appendToGlobalCtors(*context.module, Ctor, 65535); });

                if (assignExpr.has_value()) {
                    context.loadValue = true;
                    auto assignVal = processNode(assignExpr.value(), context, errors);
                    context.loadValue = false;
                    context.builder->CreateStore(assignVal, global);
                }
                else createArrayMalloc(expr, global, context, errors);
                context.builder->CreateRetVoid();
                context.popBlock();
            }
            context.globals()[name] = global;
            context.globalsDecls()[name] = shared_from_this();
            var = global;
            context.debug = tempDebugFlag;
            if (context.debug) {
                auto dbgInfo = context.debugBuilder->createGlobalVar(name, getDebugType(expr, context, errors), loc, isPrivate);
                global->addDebugInfo(dbgInfo);
            }
        }
        return var;
    }

    auto FuncDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto funcType = getFuncType(expr, context);
        auto funcName = isExtern ? name : name + "." + getMangledFuncName(expr);
        auto funcCallee = context.module->getOrInsertFunction(funcName, funcType);
        auto func = context.module->getFunction(funcName);
        DISubprogram* dbgFunc = nullptr;
        if (context.debug) {
            dbgFunc = context.debugBuilder->createFunction(this, context, errors);
            context.debugBuilder->lexicalBlocks.push_back(dbgFunc);
            context.debugBuilder->emitLocation(loc);
        }
        if (context.currentDeclImported || (isExtern && block.value()->statements.empty())) return func;
        context.context.enterScope(funcName);   // scope name is mangled
        if (block.has_value()) {
            BasicBlock *block = BasicBlock::Create(*context.llvmContext, "entry", context.module->getFunction(funcName));
            context.pushBlock(block);
            context.builder->SetInsertPoint(block);
            Function::arg_iterator argsValues = func->arg_begin();
            uint64_t argNo = 1;
            for (auto param = expr->params.begin(); param != expr->params.end(); ++param, ++argsValues, ++argNo) {
                Value* var;
                if (param->get()->parameterType == Out || param->get()->parameterType == Var)
                    var = context.builder->CreateAlloca(getIRPtrType(param->get()->type, context), nullptr, param->get()->name);
                else
                    var = context.builder->CreateAlloca(getIRType(param->get()->type, context), nullptr, param->get()->name);
                context.locals()[param->get()->name] = var;
                context.localsDecls()[param->get()->name] = *param;
                context.referenced()[param->get()->name] = false;
                argsValues->setName(parameterTypeToString(param->get()->parameterType) + param->get()->name);
                context.builder->CreateStore(argsValues, var);

                if (context.debug) {
                    auto dbgType = getDebugType(param->get()->type, context, errors);
                    if (param->get()->parameterType != In) dbgType = context.debugBuilder->getPointerType(dbgType);
                    context.debugBuilder->createLocalFuncParam(param->get()->name, dbgType, var, loc, dbgFunc, argNo);
                }
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
        if (!hasReturn) {
            // cleanupCurrentScope(context, errors);
        }
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
        if (context.debug) {
            context.debugBuilder->lexicalBlocks.pop_back();
            func->setSubprogram(dbgFunc);
        }
        return {};
    }

    auto FieldVarDecNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto type = getIRType(typeExpr.type, context);
        Value* rightVal = nullptr;
        auto elementPtr = context.builder->CreateStructGEP(context.allocatedClasses[typeName.type], context.currentTypeLoad, index, typeName.type + "." + name);
        if (!Context::isBuiltInType(typeExpr.type) && !(expr.has_value() && std::holds_alternative<NilExprPtr>(expr.value()))) {
            auto malloc = createMalloc(typeExpr.type, elementPtr, context);
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
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto type = getIRType(expr, context);
        auto elementPtr = context.builder->CreateStructGEP(context.allocatedClasses[typeName.type], context.currentTypeLoad, index, typeName.type + "." + name);

        if (assignExpr.has_value()) {
            context.loadValue = true;
            auto assignVal = processNode(assignExpr.value(), context, errors);
            context.loadValue = false;
            context.builder->CreateStore(assignVal, elementPtr);
        }
        else createArrayMalloc(expr, elementPtr, context, errors);
        return elementPtr;
    }

    auto FieldFuncPointerStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto type = getIRType(expr, context);
        auto elementPtr = context.builder->CreateStructGEP(context.allocatedClasses[typeName.type], context.currentTypeLoad, index, typeName.type + "." + name);
        if (assignExpr.has_value()) {
            auto temp = context.loadValue;
            context.loadValue = false;
            context.currentFuncSignature = expr;
            auto assignVal = processNode(assignExpr.value(), context, errors);
            context.currentFuncSignature = std::nullopt;
            context.loadValue = temp;
            context.builder->CreateStore(assignVal, elementPtr);
        }
        return elementPtr;
    }

    auto MethodDecNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto temp = create<FuncDecStatementNode>(loc, name, expr, block, true, isFunction, false);
        auto res = temp->codegen(context, errors);
        return res;
    }

    auto TypeDecStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
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
        // predefine default constructor and destructor
        auto ctor = context.module->getOrInsertFunction(name + "._default_constructor", FunctionType::get(Type::getVoidTy(*context.llvmContext), PointerType::get(context.allocatedClasses[name], 0), false));
        auto dtor = context.module->getOrInsertFunction(name + "._default_destructor", FunctionType::get(Type::getVoidTy(*context.llvmContext), PointerType::get(context.allocatedClasses[name], 0), false));
        context.context.enterScope(name);
        for (auto &method : methods) {
            method->codegen(context, errors);
        }

        if (vtableRequired) createVTable(this, context, errors);
        if (context.debug) context.debugBuilder->createType(this, context, errors);
        createDefaultConstructor(this, context, errors, context.currentDeclImported);
        createDefaultDestructor(this, context, errors, context.currentDeclImported);
        context.context.exitScope();
        return nullptr;
    }

    auto ElseIfStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        return {};
    }

    auto IfStatementNode::codegen(CodeGenContext &context, std::vector<ErrorMessage>& errors) -> Value* {
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto func = context.builder->GetInsertBlock()->getParent();
        auto trueBB = BasicBlock::Create(*context.llvmContext, "then", func);
        auto falseBB = BasicBlock::Create(*context.llvmContext, "else", func);
        auto endBB = BasicBlock::Create(*context.llvmContext, "ifend", func);
        auto temp = context.loadValue;
        context.loadValue = true;
        context.builder->CreateCondBr(processNode(condition, context, errors), trueBB, falseBB);
        context.loadValue = temp;
        context.pushBlock(trueBB);
        context.builder->SetInsertPoint(trueBB);
        context.context.enterScope("if#" + std::to_string(loc.line) + ":" + std::to_string(loc.column));
        bool hasReturn = false;
        if (context.debug) context.debugBuilder->emitLocation(loc);
        for (auto &&stmt : trueBlock->statements) {
            auto val = processNode(stmt, context, errors);
            if (std::holds_alternative<ReturnStatementPtr>(stmt)) hasReturn = true;
        }
        if (!hasReturn) context.builder->CreateBr(endBB);
        context.popBlock();
        context.context.exitScope();

        context.pushBlock(falseBB);
        context.builder->SetInsertPoint(falseBB);
        if (context.debug) context.debugBuilder->emitLocation(loc);
        //elseif blocks
        for (auto &&elseif : elseIfNodes) {
            auto elseifBBTrue = BasicBlock::Create(*context.llvmContext, "elseifthen", func);
            auto elseifBBFalse = BasicBlock::Create(*context.llvmContext, "elseifelse", func);
            context.loadValue = true;
            context.builder->CreateCondBr(processNode(elseif->condition, context, errors), elseifBBTrue, elseifBBFalse);
            context.loadValue = temp;
            context.pushBlock(elseifBBTrue);
            context.builder->SetInsertPoint(elseifBBTrue);
            if (context.debug) context.debugBuilder->emitLocation(loc);
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
            if (context.debug) context.debugBuilder->emitLocation(loc);
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
        if (context.debug) context.debugBuilder->emitLocation(loc);
        if (context.debug) context.debugBuilder->emitLocation(loc);
        auto func = context.builder->GetInsertBlock()->getParent();
        auto whileCheck = BasicBlock::Create(*context.llvmContext, "whilecheck", func);
        auto whileIter = BasicBlock::Create(*context.llvmContext, "whileiter", func);
        auto whileEnd = BasicBlock::Create(*context.llvmContext, "whileend", func);
        context.builder->CreateBr(whileCheck);
        context.pushBlock(whileCheck);
        context.builder->SetInsertPoint(whileCheck);
        auto temp = context.loadValue;
        context.loadValue = true;
        context.builder->CreateCondBr(processNode(condition, context, errors), whileIter, whileEnd);
        context.loadValue = temp;
        context.popBlock();
        context.pushBlock(whileIter);
        context.builder->SetInsertPoint(whileIter);
        bool hasReturn = false;
        context.context.enterScope("while#" + std::to_string(loc.line) + ":" + std::to_string(loc.column));
        if (context.debug) context.debugBuilder->emitLocation(loc);
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