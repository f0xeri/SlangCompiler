//
// Created by user on 19.11.2023.
//

#include "CodeGen.hpp"
#include "CodeGenContext.hpp"
#include "parser/AST.hpp"

namespace Slangc {
    Value* typeCast(Value* value, Type* type, CodeGenContext &context, std::vector<ErrorMessage> &errors, SourceLoc loc) {
        if (value->getType() == type) return value;
        if (value->getType()->isIntegerTy() && type->isIntegerTy()) {
            return context.builder->CreateIntCast(value, type, true);
        }
        if (value->getType()->isIntegerTy() && type->isFloatingPointTy()) {
            return context.builder->CreateSIToFP(value, type);
        }
        if (value->getType()->isFloatingPointTy() && type->isIntegerTy()) {
            return context.builder->CreateFPToSI(value, type);
        }
        if (value->getType()->isFloatingPointTy() && type->isFloatingPointTy()) {
            return context.builder->CreateFPCast(value, type);
        }
        std::string type_str = "";
        raw_string_ostream rso(type_str);
        value->getType()->print(rso);
        rso << " to ";
        type->print(rso);
        errors.emplace_back(context.context.filename, "Cannot cast from " + type_str + " .", loc, false, true);
        return nullptr;
    }

    Type* getIRType(const std::string& type, CodeGenContext& context) {
        if (type == "integer")
            return Type::getInt32Ty(*context.llvmContext);
        if (type == "real")
            return Type::getDoubleTy(*context.llvmContext);
        if (type == "float")
            return Type::getFloatTy(*context.llvmContext);
        if (type == "boolean")
            return Type::getInt1Ty(*context.llvmContext);
        if (type == "character")
            return Type::getInt8Ty(*context.llvmContext);
        if (type == "void" || type == "")
            return Type::getVoidTy(*context.llvmContext);
        if (context.allocatedClasses.contains(type))
            return context.allocatedClasses[type]->getPointerTo();
        return nullptr;
    }

    Type* getIRPtrType(const std::string& type, CodeGenContext& context) {
        return getIRType(type, context)->getPointerTo();
    }

    Type* getIRType(const ExprPtrVariant& expr, CodeGenContext& context) {
        if (auto type = std::get_if<TypeExprPtr>(&expr)) {
            return getIRType(type->get()->type, context);
        }
        if (auto arr = std::get_if<ArrayExprPtr>(&expr)) {
            return getIRType(arr->get()->type, context)->getPointerTo();
        }
        return nullptr;
    }

    Value* createMalloc(const std::string &type, Value* var, CodeGenContext &context) {
        auto intType = Type::getInt32Ty(*context.llvmContext);
        auto structType = getIRType(type, context);
        auto mallocCall = context.builder->CreateMalloc(intType, structType, ConstantInt::get(intType, context.module->getDataLayout().getTypeAllocSize(structType)), nullptr);
        context.builder->CreateStore(mallocCall, var);
        return var;
    }

    void createMallocLoops(int i, ArrayExprPtr &array, int indicesCount, Value *var, std::vector<Value*> jvars, std::vector<Value*> sizes, CodeGenContext &context, std::vector<ErrorMessage> &errors) {
        if (i == indicesCount) return;
        auto intType = Type::getInt32Ty(*context.llvmContext);
        auto func = context.builder->GetInsertBlock()->getParent();
        auto whileCheck = BasicBlock::Create(*context.llvmContext, "whilecheck", func);
        auto whileIter = BasicBlock::Create(*context.llvmContext, "whileiter", func);
        auto whileEnd = BasicBlock::Create(*context.llvmContext, "whileend", func);
        context.builder->CreateBr(whileCheck);
        context.pushBlock(whileCheck);
        context.builder->SetInsertPoint(whileCheck);

        auto jvar = context.builder->CreateLoad(intType, jvars[i]);
        auto arraySize = sizes[i - 1];
        auto cmp = context.builder->CreateICmpSLT(jvar, arraySize, "j_less_arrSz");
        context.builder->CreateCondBr(cmp, whileIter, whileEnd);
        context.popBlock();
        context.pushBlock(whileIter);
        context.builder->SetInsertPoint(whileIter);

        auto arrayType = getIRType(array->type, context);
        auto loadArrType = getIRType(array, context);

        auto allocSize = context.builder->CreateMul(sizes[i], ConstantInt::get(intType, context.module->getDataLayout().getTypeAllocSize(arrayType)));
        auto mallocCall = context.builder->CreateMalloc(intType, arrayType, allocSize, nullptr);

        auto arrLoad = context.builder->CreateLoad(loadArrType, var);
        Value* arrPtr = nullptr;
        for (int k = 1; k <= i; ++k) {
            jvar = context.builder->CreateLoad(intType, jvars[k]);
            auto sext = context.builder->CreateSExt(jvar, Type::getInt64Ty(*context.llvmContext));
            arrPtr = context.builder->CreateGEP(loadArrType, arrLoad, sext);
            arrLoad = context.builder->CreateLoad(loadArrType, arrPtr);
        }
        context.builder->CreateStore(mallocCall, arrPtr);

        if (i == indicesCount - 1) {
            jvar = context.builder->CreateLoad(intType, jvars[i]);
            auto add = context.builder->CreateAdd(jvar, ConstantInt::get(intType, 1));
            context.builder->CreateStore(add, jvars[i]);
        }
        else context.builder->CreateStore(ConstantInt::get(intType, 0), jvars[i + 1]);

        auto nextArray = std::get_if<ArrayExprPtr>(&array->type);
        createMallocLoops(i + 1, *nextArray, indicesCount, var, jvars, sizes, context, errors);

        context.builder->CreateBr(whileCheck);
        context.popBlock();
        context.ret(whileEnd);
        context.builder->SetInsertPoint(whileEnd);

        if (i > 1) {
            auto endJvar = context.builder->CreateLoad(intType, jvars[i - 1]);
            auto endAdd = context.builder->CreateAdd(endJvar, ConstantInt::get(intType, 1));
            context.builder->CreateStore(endAdd, jvars[i - 1]);
        }
    }

    Value* createArrayMalloc(ArrayExprPtr& array, Value* var, CodeGenContext &context, std::vector<ErrorMessage> &errors) {
        auto intType = Type::getInt32Ty(*context.llvmContext);
        auto structType = getIRType(array->type, context);
        auto arraySize = processNode(array->size, context, errors);
        auto allocSize = context.builder->CreateMul(arraySize, ConstantInt::get(intType, context.module->getDataLayout().getTypeAllocSize(structType)));
        auto mallocCall = context.builder->CreateMalloc(intType, structType, allocSize, nullptr);
        auto indicesCount = array->getIndicesCount();
        context.builder->CreateStore(mallocCall, var);
        if (indicesCount == 1) return var;
        std::vector<Value*> sizes;
        sizes.reserve(indicesCount);
        sizes.push_back(arraySize);
        auto currArray = array->type;
        while (auto arr = std::get_if<ArrayExprPtr>(&currArray)) {
            sizes.push_back(processNode(arr->get()->size, context, errors));
            currArray = arr->get()->type;
        }
        std::vector<Value*> jvars;
        jvars.reserve(indicesCount);
        for (int i = 0; i < indicesCount; ++i) {
            jvars.push_back(context.builder->CreateAlloca(intType, nullptr, "j" + std::to_string(i)));
            context.builder->CreateStore(ConstantInt::get(intType, 0), jvars[i]);
        }
        int i = 1;
        createMallocLoops(i, std::get<ArrayExprPtr>(array->type), indicesCount, var, jvars, sizes, context, errors);
        return var;
    }

    Function* createDefaultConstructor(TypeDecStatementNode* type, CodeGenContext &context, std::vector<ErrorMessage>& errors) {
        auto structType = getIRType(type->name, context);
        auto intType = Type::getInt32Ty(*context.llvmContext);
        auto constructorType = FunctionType::get(Type::getVoidTy(*context.llvmContext), {structType->getPointerTo()}, false);
        auto constructor = Function::Create(constructorType, Function::ExternalLinkage, type->name + "._default_constructor", context.module.get());
        auto block = BasicBlock::Create(*context.llvmContext, "entry", constructor);
        context.builder->SetInsertPoint(block);
        context.pushBlock(block);

        auto typeAlloc = context.builder->CreateAlloca(structType);
        auto storeThis = context.builder->CreateStore(constructor->getArg(0), typeAlloc);
        auto loadThis = context.builder->CreateLoad(structType->getPointerTo(), typeAlloc);
        context.currentTypeLoad = loadThis;
        for (auto &&field : type->fields) {
            processNode(field, context, errors);
        }
        context.builder->CreateRetVoid();
        context.popBlock();
        return constructor;
    }
}
