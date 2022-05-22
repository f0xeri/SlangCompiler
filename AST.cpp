//
// Created by f0xeri on 23.01.2022.
//

#include "AST.hpp"

using namespace llvm;

static Value* mycast(Value* value, Type* type, CodeGenContext& cgcontext) {
    if (type == value->getType())
        return value;
    if (type == Type::getDoubleTy(*cgcontext.context)) {
        if (value->getType() == Type::getInt64Ty(*cgcontext.context) || value->getType() == Type::getInt8Ty(*cgcontext.context))
            value = new SIToFPInst(value, type, "", cgcontext.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    }
    else if (type == Type::getInt64Ty(*cgcontext.context)) {
        if (value->getType() == Type::getDoubleTy(*cgcontext.context))
            value = new FPToSIInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt8Ty(*cgcontext.context))
            value = new SExtInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt32Ty(*cgcontext.context))
            value = new ZExtInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt8PtrTy(*cgcontext.context))
            value = new PtrToIntInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt64PtrTy(*cgcontext.context))
            value = new PtrToIntInst(value, type, "", cgcontext.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    } else if (type == Type::getInt8Ty(*cgcontext.context)) {
        if (value->getType() == Type::getDoubleTy(*cgcontext.context))
            value = new FPToSIInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt64Ty(*cgcontext.context))
            value = new TruncInst(value, type, "", cgcontext.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    } else
        llvm::errs() << "[ERROR] Cannot cast this value.\n";
    return value;
}

llvm::Value *IntExprNode::codegen(CodeGenContext &cgcontext) {
    return ConstantInt::get(Type::getInt64Ty(*cgcontext.context), value, true);
}

llvm::Value *RealExprNode::codegen(CodeGenContext &cgcontext) {
    return ConstantFP::get(Type::getDoubleTy(*cgcontext.context), value);
}

llvm::Value *CharExprNode::codegen(CodeGenContext &cgcontext) {
    return ConstantInt::get(Type::getInt8Ty(*cgcontext.context), value, true);
}

llvm::Value *StringExprNode::codegen(CodeGenContext &cgcontext) {
    return cgcontext.builder->CreateGlobalStringPtr(value);
}

llvm::Value *ArrayExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *BooleanExprNode::codegen(CodeGenContext &cgcontext) {
    return ConstantInt::get(Type::getInt1Ty(*cgcontext.context), value, false);
}

llvm::Value *VariableExprNode::codegen(CodeGenContext &cgcontext) {
    Value* val;
    Type *type = nullptr;
    // check local variables
    val = cgcontext.localsLookup(value);
    // check global variables if there is no local
    if (val == nullptr)
    {
        // check global variable declared in current module
        if(cgcontext.globals().find(cgcontext.moduleName + "." + value) == cgcontext.globals().end())
        {
            // check other global variables
            if (cgcontext.globals().find(value) == cgcontext.globals().end())
                llvm::errs() << "[ERROR] Undeclared Variable \"" << value << "\".\n";
            else
                val = cgcontext.globals()[value];
        }
        else
        {
            val = cgcontext.globals()[cgcontext.moduleName + "." + value];
        }
    }
    type = val->getType()->getPointerElementType();
    return new LoadInst(type, val, "", false, cgcontext.currentBlock());
}

llvm::Value *UnaryOperatorExprNode::codegen(CodeGenContext &cgcontext) {
    return BinaryOperator::CreateNeg(right->codegen(cgcontext), "", cgcontext.currentBlock());
}

llvm::Value *OperatorExprNode::codegen(CodeGenContext &cgcontext) {
    Value* leftVal = left->codegen(cgcontext);
    Value* rightVal = right->codegen(cgcontext);
    bool floatOp = false;
    if (leftVal->getType()->isDoubleTy() || rightVal->getType()->isDoubleTy())
    {
        leftVal = mycast(leftVal, Type::getDoubleTy(*cgcontext.context), cgcontext);
        rightVal = mycast(rightVal, Type::getDoubleTy(*cgcontext.context), cgcontext);
        floatOp = true;
    }
    else if (leftVal->getType() == rightVal->getType())
    {

    }
    else {
        leftVal = mycast(leftVal, Type::getInt64Ty(*cgcontext.context), cgcontext);
        rightVal = mycast(rightVal, Type::getInt64Ty(*cgcontext.context), cgcontext);
    }
    if (!leftVal || !rightVal)
        return nullptr;
    if (!floatOp)
    {
        switch (op) {
            case TokenType::Plus:
                return BinaryOperator::Create(Instruction::Add, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Minus:
                return BinaryOperator::Create(Instruction::Sub, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Multiplication:
                return BinaryOperator::Create(Instruction::Mul, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Division:
                return BinaryOperator::Create(Instruction::SDiv, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Equal:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::ICMP_EQ, leftVal, rightVal, "");
            case TokenType::NotEqual:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::ICMP_NE, leftVal, rightVal, "");
            case TokenType::Greater:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::ICMP_SGT, leftVal, rightVal, "");
            case TokenType::GreaterOrEqual:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::ICMP_SGE, leftVal, rightVal, "");
            case TokenType::Less:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::ICMP_SLT, leftVal, rightVal, "");
            case TokenType::LessOrEqual:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::ICMP_SLE, leftVal, rightVal, "");
            case TokenType::And:
                return BinaryOperator::Create(Instruction::And, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Or:
                return BinaryOperator::Create(Instruction::Or, leftVal, rightVal, "", cgcontext.currentBlock());
        }
    }
    else
    {
        switch (op) {
            case TokenType::Plus:
                return BinaryOperator::Create(Instruction::FAdd, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Minus:
                return BinaryOperator::Create(Instruction::FSub, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Multiplication:
                return BinaryOperator::Create(Instruction::FMul, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Division:
                return BinaryOperator::Create(Instruction::FDiv, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Equal:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OEQ, leftVal, rightVal, "");
            case TokenType::NotEqual:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_ONE, leftVal, rightVal, "");
            case TokenType::Greater:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OGT, leftVal, rightVal, "");
            case TokenType::GreaterOrEqual:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OGE, leftVal, rightVal, "");
            case TokenType::Less:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OLT, leftVal, rightVal, "");
            case TokenType::LessOrEqual:
                return new ICmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OLE, leftVal, rightVal, "");
        }
    }
    llvm::errs() << "[ERROR] Unknown or wrong operator.\n";
    return nullptr;
}

llvm::Value *ConditionalExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *CallExprNode::codegen(CodeGenContext &cgcontext) {
    Function *function = cgcontext.mModule->getFunction(name->value);
    if (function == nullptr)
        llvm::errs() << "[ERROR] Codegen - no such function \"" << name->value << "\".\n";
    std::vector<Value*> argsRef;
    for (auto it = args->begin(); it != args->end(); it++)
        argsRef.push_back((*it)->codegen(cgcontext));
    CallInst *call = CallInst::Create(function, makeArrayRef(argsRef), "", cgcontext.currentBlock());
    return call;
}

llvm::Value *BlockExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *AssignExprNode::codegen(CodeGenContext &cgcontext) {
    Value* var;
    auto assignData = right->codegen(cgcontext);
    var = cgcontext.localsLookup(left->value);
    if (var == nullptr)
    {
        if(cgcontext.globals().find(cgcontext.moduleName + "." + left->value) == cgcontext.globals().end())
        {
            if (cgcontext.globals().find(left->value) == cgcontext.globals().end())
                llvm::errs() << "[ERROR] Undeclared Variable \"" << left->value << "\".\n";
            else
                var = cgcontext.globals()[left->value];
        }
        else
        {
            var = cgcontext.globals()[cgcontext.moduleName + "." + left->value];
        }
    }
    if (var == nullptr)
    {
        llvm::errs() << "[ERROR] Undeclared Variable \"" << left->value << "\".\n";
        return nullptr;
    }

    if (dynamic_cast<IndexExprNode*>(left) != nullptr)
    {
        auto loadArr = cgcontext.builder->CreateLoad(var);
        auto elementPtr = cgcontext.builder->CreateInBoundsGEP(loadArr, dynamic_cast<IndexExprNode*>(left)->indexExpr->codegen(cgcontext));
        auto ret = cgcontext.builder->CreateStore(assignData, elementPtr);
        return nullptr;
    }

    return new StoreInst(assignData, var, false, cgcontext.currentBlock());
}

llvm::Value *FuncExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *CastExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *ExprStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *ReturnStatementNode::codegen(CodeGenContext &cgcontext) {
    return cgcontext.builder->CreateRet(expr->codegen(cgcontext));
}

llvm::Value *OutputStatementNode::codegen(CodeGenContext &cgcontext) {
    Value *value = expr->codegen(cgcontext);
    std::vector<Value *> printArgs;
    Value *formatStr;
    if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() == 64) {
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%d\n");
    }
    else if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() == 8) {
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%c\n");
    }
    else if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() == 1) {
        value = cgcontext.builder->CreateIntCast(value, Type::getInt64Ty(*cgcontext.context), false);
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%d\n");
    }
    else if (value->getType()->isDoubleTy()) {
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%f\n");
    }
    else if (value->getType()->isPointerTy()) {
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%s\n");
    }
    printArgs.push_back(formatStr);
    printArgs.push_back(value);
    llvm::Value* res = cgcontext.builder->CreateCall(cgcontext.mModule->getFunction("printf"), printArgs);
    return res;
}

llvm::Value *DeclarationNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

static Type *typeOf(CodeGenContext &cgcontext, const std::string &var) {
    Type *type = nullptr;
    if (var == "integer")
        type = Type::getInt64Ty(*cgcontext.context);
    else if (var == "character")
        type = Type::getInt8Ty(*cgcontext.context);
    else if (var == "real")
        type = Type::getDoubleTy(*cgcontext.context);
    else if (var == "boolean")
        type = Type::getInt1Ty(*cgcontext.context);
    else if (var.empty())
        type = Type::getVoidTy(*cgcontext.context);
    else
    {
        if (cgcontext.allocatedClasses.contains(var))
            type = cgcontext.allocatedClasses[var];
    }
    return type;
}

static Type *ptrToTypeOf(CodeGenContext &cgcontext, const std::string &var) {
    Type *type = nullptr;
    if (var == "integer")
        type = Type::getInt64PtrTy(*cgcontext.context);
    else if (var == "character")
        type = Type::getInt8PtrTy(*cgcontext.context);
    else if (var == "real")
        type = Type::getDoublePtrTy(*cgcontext.context);
    else if (var.empty())
        type = Type::getVoidTy(*cgcontext.context);
    else
    {
        if (cgcontext.allocatedClasses.contains(var))
            type = cgcontext.allocatedClasses[var]->getPointerTo();
    }
    return type;
}

llvm::Value *VarDecStatementNode::codegen(CodeGenContext &cgcontext) {
    Value* newVar = nullptr;
    Value* rightVal = nullptr;
    if (isGlobal)
    {
        cgcontext.mModule->getOrInsertGlobal(name->value, typeOf(cgcontext, type));
        auto gVar = cgcontext.mModule->getNamedGlobal(name->value);
        gVar->setLinkage(GlobalValue::ExternalLinkage);
        if (expr != NULL)
        {
            rightVal = expr->codegen(cgcontext);
            gVar->setInitializer(static_cast<Constant *>(rightVal));
        }
        gVar->setAlignment(Align(8));
        cgcontext.globals()[name->value] = gVar;
    }
    else
    {
        newVar = cgcontext.builder->CreateAlloca(typeOf(cgcontext, type), 0, nullptr, name->value);
        cgcontext.locals()[name->value] = newVar;
        if (expr != NULL) {
            rightVal = expr->codegen(cgcontext);
            //new StoreInst(rightVal, newVar, false, cgcontext.currentBlock());
            cgcontext.builder->CreateStore(rightVal, newVar);
        }
    }

    return newVar;
}

llvm::Value *FuncParamDecStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *ArrayDecStatementNode::codegen(CodeGenContext &cgcontext) {
    auto exprNode = dynamic_cast<ArrayExprNode*>(expr);
    auto arrExpr = exprNode;
    if (isGlobal)
    {
        if (!exprNode->size->isConst) {
            llvm::errs() << "[ERROR] Codegen - global array " << name->value << " size should be constant.\n";
            exit(-1);
        }
    }
    auto size = exprNode->size->codegen(cgcontext);
    auto arraySize = size;
    if (arrExpr->type == "array")
    {
        for (auto &slice : *arrExpr->values)
        {
            auto castedSlice = dynamic_cast<ArrayExprNode*>(slice);
            auto sliceSize = castedSlice->size->codegen(cgcontext);
            auto newArraySize = BinaryOperator::Create(Instruction::Mul, sliceSize, arraySize, "", cgcontext.currentBlock());
            arraySize = newArraySize;
            arrExpr = castedSlice;
        }
    }
    auto type = typeOf(cgcontext, arrExpr->type);
    llvm::Type* int64type = llvm::Type::getInt64Ty(*cgcontext.context);

    if (isGlobal)
    {
        cgcontext.mModule->getOrInsertGlobal(name->value, type->getPointerTo());
        auto gVar = cgcontext.mModule->getNamedGlobal(name->value);
        gVar->setLinkage(GlobalValue::ExternalLinkage);
        gVar->setInitializer(ConstantPointerNull::get(PointerType::get(type, 0)));
        gVar->setAlignment(Align(8));
        gVar->setDSOLocal(true);
        cgcontext.globals()[name->value] = gVar;

        FunctionType* type = FunctionType::get(Type::getVoidTy(*cgcontext.context), {}, false);
        Function *constructorFunc = Function::Create(type, Function::ExternalLinkage, Twine(name->value + "_constructor"), cgcontext.mModule);
        BasicBlock *bb = BasicBlock::Create(*cgcontext.context, name->value + "_constructorEntry", constructorFunc, nullptr);
        cgcontext.pushBlock(bb);
        cgcontext.builder->SetInsertPoint(bb);

        getOrCreateSanitizerCtorAndInitFunctions(
                *cgcontext.mModule, cgcontext.moduleName + "_GLOBAL_", name->value + "_constructor", {}, {},
                // This callback is invoked when the functions are created the first
                // time. Hook them into the global ctors list in that case:
                [&](Function *Ctor, FunctionCallee) { appendToGlobalCtors(*cgcontext.mModule, Ctor, 65535); });
    }
    else
    {
        auto lVar = cgcontext.builder->CreateAlloca(type->getPointerTo(), 0, nullptr, name->value);
        cgcontext.locals()[name->value] = lVar;
    }

    auto elementSize = ConstantInt::get(int64type, cgcontext.dataLayout->getTypeAllocSize(type));
    auto allocSize = BinaryOperator::Create(Instruction::Mul, elementSize, arraySize, "", cgcontext.currentBlock());
    // GC_malloc
    //auto arr = cgcontext.builder->CreateCall(cgcontext.mModule->getFunction("GC_malloc"), {allocSize});
    // malloc
    auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int64type, type, allocSize, nullptr, nullptr, "");
    cgcontext.builder->Insert(arr);
    if (isGlobal)
    {
        auto gVar = cgcontext.mModule->getNamedGlobal(name->value);
        cgcontext.builder->CreateStore(arr, gVar);
        cgcontext.builder->CreateRetVoid();
        cgcontext.popBlock();
        return gVar;
    }
    else
    {
        auto lVar = cgcontext.locals()[name->value];
        cgcontext.builder->CreateStore(arr, lVar);
        return lVar;
    }
    //ArrayType *arrayType = ArrayType::get(type, reinterpret_cast<ConstantInt*>(arraySize)->getZExtValue());
    //AllocaInst *alloc = new AllocaInst(arrayType, value->value, cgcontext.currentBlock());
    // auto var = new AllocaInst(arrayType, 0, value->value, cgcontext.currentBlock());
    return nullptr;
}

llvm::Value *IndexExprNode::codegen(CodeGenContext &cgcontext) {
    Value* var;
    var = cgcontext.localsLookup(value);
    if (var == nullptr)
    {
        if(cgcontext.globals().find(cgcontext.moduleName + "." + value) == cgcontext.globals().end())
        {
            if (cgcontext.globals().find(value) == cgcontext.globals().end())
                llvm::errs() << "[ERROR] Undeclared Variable \"" << value << "\".\n";
            else
                var = cgcontext.globals()[value];
        }
        else
        {
            var = cgcontext.globals()[cgcontext.moduleName + "." + value];
        }
    }
    if (var == nullptr)
    {
        llvm::errs() << "[ERROR] Undeclared Variable \"" << value << "\".\n";
        return nullptr;
    }
    auto loadArr = cgcontext.builder->CreateLoad(var);
    auto elementPtr = cgcontext.builder->CreateInBoundsGEP(loadArr, indexExpr->codegen(cgcontext));
    return cgcontext.builder->CreateLoad(elementPtr, false);
}

std::string getParameterTypeName(ParameterType type)
{
    if (type == In) return "in";
    if (type == Out) return "out";
    if (type == Var) return "var";
    return "";
}

llvm::Value *FuncDecStatementNode::codegen(CodeGenContext &cgcontext) {
    std::vector<llvm::Type*> argTypes;
    std::vector<int> refParams;
    int i = 1;
    for (auto arg : *args)
    {
        Type* argType = typeOf(cgcontext, arg->type);
        if (arg->parameterType == ParameterType::Out || arg->parameterType == ParameterType::Var)
        {
            argTypes.push_back(ptrToTypeOf(cgcontext, arg->type));
            refParams.push_back(i);
        }
        else
        {
            argTypes.push_back(typeOf(cgcontext, arg->type));
        }
        i++;
    }
    FunctionType* funcType = FunctionType::get(typeOf(cgcontext, type), argTypes, false);
    auto function = Function::Create(funcType, GlobalValue::ExternalLinkage, name->value, cgcontext.mModule);
    for (auto paramID : refParams)
    {
        // TODO Set Dereferenceable attribute for ref params
        //function->addAttribute(paramID, Attribute::Dereferenceable);
    }
    if (block != nullptr) {
        BasicBlock *bb = BasicBlock::Create(*cgcontext.context, cgcontext.moduleName + type + name->value + "Entry",
                                            function, 0);
        cgcontext.pushBlock(bb);
        cgcontext.builder->SetInsertPoint(bb);
        Function::arg_iterator argsValues = function->arg_begin();
        for (auto it = args->begin(); it != args->end(); it++, argsValues++) {
            if ((*it)->parameterType == ParameterType::Out || (*it)->parameterType == ParameterType::Var) {
                auto var = new AllocaInst(ptrToTypeOf(cgcontext, (*it)->type), 0, nullptr, (*it)->name->value, bb);
                cgcontext.locals()[(*it)->name->value] = var;
                Value *argumentValue = &(*argsValues);
                argumentValue->setName(getParameterTypeName((*it)->parameterType) + (*it)->name->value);
                StoreInst *inst = new StoreInst(argumentValue, cgcontext.locals()[(*it)->name->value], false, bb);
            } else {
                auto var = new AllocaInst(typeOf(cgcontext, (*it)->type), 0, nullptr, (*it)->name->value, bb);
                cgcontext.locals()[(*it)->name->value] = var;
                Value *argumentValue = &(*argsValues);
                argumentValue->setName(getParameterTypeName((*it)->parameterType) + (*it)->name->value);
                StoreInst *inst = new StoreInst(argumentValue, cgcontext.locals()[(*it)->name->value], false, bb);
            }
        }

        bool isRet = false;
        for (auto statement: *block->statements) {
            //if (isRet) llvm::errs() << "[WARN] Code after return statement in block is unreachable.\n";
            statement->codegen(cgcontext);
            if (dynamic_cast<ReturnStatementNode *>(statement) != nullptr) {
                isRet = true;
                break;
            }
        }

        if (!isFunction) cgcontext.builder->CreateRetVoid();
        else {
            if (cgcontext.currentBlock()->empty()) {
                cgcontext.builder->CreateCall(cgcontext.mModule->getFunction("llvm.trap"), {});
                cgcontext.builder->CreateUnreachable();
            } else {
                if (cgcontext.currentBlock()->getTerminator() == nullptr) {
                    cgcontext.builder->CreateCall(cgcontext.mModule->getFunction("llvm.trap"), {});
                    cgcontext.builder->CreateUnreachable();
                }
            }
        }

        cgcontext.popBlock();
    }
    //cgcontext.builder->SetInsertPoint(cgcontext.currentBlock());
    return nullptr;
}

llvm::Value *FieldVarDecNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *FieldArrayVarDecNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *MethodDecNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *TypeDecStatementNode::codegen(CodeGenContext &cgcontext) {
    if (name->value == "Object") return nullptr;
    cgcontext.allocatedClasses[name->value] = StructType::create(*cgcontext.context, name->value);
    cgcontext.allocatedClasses[name->value]->setName(name->value);
    std::vector<Type*> dataTypes;
    for (auto field : *fields)
    {
        Type* type;
        if (dynamic_cast<FieldArrayVarDecNode*>(field))
        {
            auto exprNode = dynamic_cast<FieldArrayVarDecNode*>(field)->var;
            auto arrExpr = dynamic_cast<ArrayExprNode*>(exprNode->expr);
            auto size = arrExpr->size->codegen(cgcontext);
            auto arraySize = size;
            if (arrExpr->type == "array")
            {
                for (auto &slice : *arrExpr->values)
                {
                    auto castedSlice = dynamic_cast<ArrayExprNode*>(slice);
                    auto sliceSize = castedSlice->size->codegen(cgcontext);
                    auto newArraySize = BinaryOperator::Create(Instruction::Mul, sliceSize, arraySize, "", cgcontext.currentBlock());
                    arraySize = newArraySize;
                    arrExpr = castedSlice;
                }
            }
            type = ptrToTypeOf(cgcontext, arrExpr->type);
        }
        else
            type = typeOf(cgcontext, field->name->value);
        dataTypes.push_back(type);
        // ... array, class, string
    }
    cgcontext.allocatedClasses[name->value]->setBody(dataTypes);
    return nullptr;
}

llvm::Value *ExternFuncDecStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *ElseIfStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *IfStatementNode::codegen(CodeGenContext &cgcontext) {
    Function *TheFunction = cgcontext.builder->GetInsertBlock()->getParent();
    BasicBlock* ifTrue = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
    BasicBlock* ifFalse = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
    BasicBlock *ifEnd = nullptr;
    BranchInst::Create(ifTrue, ifFalse, condExpr->codegen(cgcontext), cgcontext.currentBlock());
    // Entering IF
    cgcontext.pushBlock(ifTrue);
    cgcontext.builder->SetInsertPoint(ifTrue);
    bool isRetTrue = false;
    if (trueBlock != nullptr)
    {
        for (auto statement : *trueBlock->statements)
        {
            //if (isRetTrue) llvm::errs() << "[WARN] Code after return statement in block is unreachable.\n";
            statement->codegen(cgcontext);
            if (dynamic_cast<ReturnStatementNode*>(statement) != nullptr)
            {
                isRetTrue = true;
                break;
            }
        }
    }
    // JMP to END
    if (!isRetTrue)
    {
        if (ifEnd == nullptr) ifEnd = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
        BranchInst::Create(ifEnd, cgcontext.currentBlock());
    }
    cgcontext.popBlock();
    // Entering ELSE
    cgcontext.pushBlock(ifFalse);
    cgcontext.builder->SetInsertPoint(ifFalse);
    bool isRetFalse = false;
    if (falseBlock != nullptr)
    {
        for (auto statement : *falseBlock->statements)
        {
            //if (isRetFalse) llvm::errs() << "[WARN] Code after return statement in block is unreachable.\n";
            statement->codegen(cgcontext);
            if (dynamic_cast<ReturnStatementNode*>(statement) != nullptr)
            {
                isRetFalse = true;
                break;
            }
        }
    }
    // JMP to END
    if (!isRetFalse)
    {
        if (ifEnd == nullptr) ifEnd = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
        BranchInst::Create(ifEnd, cgcontext.currentBlock());
    }
    cgcontext.popBlock();
    // Return END
    if (!isRetTrue || !isRetFalse)
    {
        cgcontext.ret(ifEnd);
        cgcontext.builder->SetInsertPoint(ifEnd);
    }

    return ifEnd;
}

llvm::Value *ForStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *WhileStatementNode::codegen(CodeGenContext &cgcontext) {
    Function *TheFunction = cgcontext.builder->GetInsertBlock()->getParent();
    BasicBlock* whileIter = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
    BasicBlock* whileEnd = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
    BasicBlock* whileCheck = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
    // Check condition satisfaction
    BranchInst::Create(whileCheck, cgcontext.currentBlock());
    cgcontext.pushBlock(whileCheck);
    cgcontext.builder->SetInsertPoint(whileCheck);
    // Whether break the loop
    BranchInst::Create(whileIter, whileEnd, whileExpr->codegen(cgcontext), cgcontext.currentBlock());
    cgcontext.popBlock();
    // Entering loop block
    cgcontext.pushBlock(whileIter);
    cgcontext.builder->SetInsertPoint(whileIter);
    bool isRet = false;
    if (block != nullptr)
    {
        for (auto statement : *block->statements)
        {
            //if (isRet) llvm::errs() << "[WARN] Code after return statement in block is unreachable.\n";
            statement->codegen(cgcontext);
            if (dynamic_cast<ReturnStatementNode*>(statement) != nullptr)
            {
                isRet = true;
                break;
            }
        }
    }
    // Jump back to condition checking
    if (!isRet)
    {
        BranchInst::Create(whileCheck, cgcontext.currentBlock());
    }
    cgcontext.popBlock();
    // Return END
    cgcontext.ret(whileEnd);
    cgcontext.builder->SetInsertPoint(whileEnd);
    return whileEnd;
}

llvm::Value *ModuleStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *ImportStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}