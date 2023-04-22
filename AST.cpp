//
// Created by f0xeri on 23.01.2022.
//

#include "AST.hpp"

using namespace llvm;


static Type *typeOf(CodeGenContext &cgcontext, const std::string &var) {
    Type *type = nullptr;
    if (var == "integer")
        type = Type::getInt32Ty(*cgcontext.context);
    else if (var == "character")
        type = Type::getInt8Ty(*cgcontext.context);
    else if (var == "real")
        type = Type::getDoubleTy(*cgcontext.context);
    else if (var == "float")
        type = Type::getFloatTy(*cgcontext.context);
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
        type = Type::getInt32PtrTy(*cgcontext.context);
    else if (var == "character")
        type = Type::getInt8PtrTy(*cgcontext.context);
    else if (var == "real")
        type = Type::getDoublePtrTy(*cgcontext.context);
    else if (var == "float")
        type = Type::getFloatPtrTy(*cgcontext.context);
    else if (var.empty())
        type = Type::getVoidTy(*cgcontext.context);
    else
    {
        if (cgcontext.allocatedClasses.contains(var))
            type = cgcontext.allocatedClasses[var]->getPointerTo();
    }
    return type;
}

static DIType *dbgTypeOf(CodeGenContext &cgcontext, const std::string &var) {
    DIType *type = nullptr;
    if (var == "integer")
        type = cgcontext.dbgInfo.getIntegerTy();
    else if (var == "character")
        type = cgcontext.dbgInfo.getCharacterType();
    else if (var == "real")
        type = cgcontext.dbgInfo.getRealTy();
    else if (var == "float")
        type = cgcontext.dbgInfo.getFloatType();
    else if (var == "boolean")
        type = cgcontext.dbgInfo.getBooleanType();
    else if (var.empty())
        // ????
        //type = Type::getVoidTy(*cgcontext.context);
        type = type;
    else
    {
        if (cgcontext.dbgInfo.dbgClasses.contains(var))
            type = cgcontext.dbgInfo.dbgClasses[var];
    }
    return type;
}

static DIType *dbgPrToTypeOf(CodeGenContext &cgcontext, const std::string &var) {
    DIType *type = nullptr;
    if (var == "integer")
        type = cgcontext.dbgInfo.createPointerType(cgcontext.dbgInfo.getIntegerTy(), var);
    else if (var == "character")
        type = cgcontext.dbgInfo.createPointerType(cgcontext.dbgInfo.getCharacterType(), var);
    else if (var == "real")
        type = cgcontext.dbgInfo.createPointerType(cgcontext.dbgInfo.getRealTy(), var);
    else if (var == "float")
        type = cgcontext.dbgInfo.createPointerType(cgcontext.dbgInfo.getFloatType(), var);
    else if (var == "boolean")
        type = cgcontext.dbgInfo.createPointerType(cgcontext.dbgInfo.getBooleanType(), var);
    else if (var.empty())
        // ????
        //type = Type::getVoidTy(*cgcontext.context);
        type = type;
    else
    {
        if (cgcontext.dbgInfo.dbgClasses.contains(var))
            type = cgcontext.dbgInfo.createPointerType(cgcontext.dbgInfo.dbgClasses[var], var);
    }
    return type;
}

// array size is not calculating here!!!
Type* getTypeFromExprNode(CodeGenContext &cgcontext, ExprNode* node, ParameterType paramType = ParameterType::In)
{
    Type *retType = nullptr;
    std::string retTypeName;
    if (dynamic_cast<VariableExprNode*>(node) != nullptr)
    {
        retType = typeOf(cgcontext, dynamic_cast<VariableExprNode*>(node)->value);
        retTypeName = dynamic_cast<VariableExprNode*>(node)->value;
    }
    else if (dynamic_cast<ArrayExprNode*>(node) != nullptr)
    {
        auto exprNode = dynamic_cast<ArrayExprNode*>(node);
        auto arrExpr = exprNode;

        //auto size = exprNode->size->codegen(cgcontext);
        //auto arraySize = size;
        auto indicesCount = 1;
        if (arrExpr->type == "array")
        {
            for (auto &slice : *arrExpr->values)
            {
                auto castedSlice = dynamic_cast<ArrayExprNode*>(slice);
                //auto sliceSize = castedSlice->size->codegen(cgcontext);
                //auto newArraySize = BinaryOperator::Create(Instruction::Mul, sliceSize, arraySize, "", cgcontext.currentBlock());
                //arraySize = newArraySize;
                arrExpr = castedSlice;
                indicesCount++;
            }
        }

        retType = ptrToTypeOf(cgcontext, arrExpr->type);
        for (int i = 1; i < indicesCount; i++) {
            retType = retType->getPointerTo();
        }
        retTypeName = arrExpr->type + "Array";
    }
    else if (dynamic_cast<FuncExprNode*>(node) != nullptr)
    {
        auto exprNode = dynamic_cast<FuncExprNode*>(node);
        std::vector<llvm::Type*> argTypes;
        std::vector<int> refParams;
        int i = 1;
        for (auto arg : *exprNode->args)
        {
            Type* argType;
            if (arg->parameterType == ParameterType::Out || arg->parameterType == ParameterType::Var)
            {
                // not tested
                argType = getTypeFromExprNode(cgcontext, arg->type)->getPointerTo();
                argTypes.push_back(argType);
                refParams.push_back(i);
            }
            else
            {
                argType = getTypeFromExprNode(cgcontext, arg->type);
                argTypes.push_back(argType);
            }
            i++;
        }
        Type *retFuncType = getTypeFromExprNode(cgcontext, exprNode->type);
        if (retFuncType->isStructTy()) {
            retFuncType = retFuncType->getPointerTo();
        }

        FunctionType* funcType = FunctionType::get(retFuncType, argTypes, false);
        retType = funcType;
    }
    if (paramType == ParameterType::Out || paramType == ParameterType::Var)
    {
        retType = retType->getPointerTo();
    }
    return retType;
}

DIType* getDbgTypeFromExprNode(CodeGenContext &cgcontext, ExprNode* node, ParameterType paramType = ParameterType::In)
{
    DIType *retType = nullptr;
    std::string retTypeName;
    if (dynamic_cast<VariableExprNode*>(node) != nullptr)
    {
        if (paramType == ParameterType::In)
            retType = dbgTypeOf(cgcontext, dynamic_cast<VariableExprNode*>(node)->value);
        else
            retType = dbgPrToTypeOf(cgcontext, dynamic_cast<VariableExprNode*>(node)->value);
        retTypeName = dynamic_cast<VariableExprNode*>(node)->value;
    }
    else if (dynamic_cast<ArrayExprNode*>(node) != nullptr)
    {
        auto exprNode = dynamic_cast<ArrayExprNode*>(node);
        auto arrExpr = exprNode;

        //auto size = exprNode->size->codegen(cgcontext);
        //auto arraySize = size;
        auto indicesCount = 1;
        if (arrExpr->type == "array")
        {
            for (auto &slice : *arrExpr->values)
            {
                auto castedSlice = dynamic_cast<ArrayExprNode*>(slice);
                //auto sliceSize = castedSlice->size->codegen(cgcontext);
                //auto newArraySize = BinaryOperator::Create(Instruction::Mul, sliceSize, arraySize, "", cgcontext.currentBlock());
                //arraySize = newArraySize;
                arrExpr = castedSlice;
                indicesCount++;
            }
        }

        retType = dbgPrToTypeOf(cgcontext, arrExpr->type);
        for (int i = 1; i < indicesCount; i++) {
            retType = cgcontext.dbgInfo.createPointerType(retType, "array");
        }
        retTypeName = arrExpr->type + "Array";
    }
    else if (dynamic_cast<FuncExprNode*>(node) != nullptr)
    {
        auto exprNode = dynamic_cast<FuncExprNode*>(node);
        auto dbgRetFuncType = getDbgTypeFromExprNode(cgcontext, exprNode->type);
        retType = cgcontext.debugBuilder->createPointerType(dbgRetFuncType, dbgRetFuncType->getSizeInBits(), dbgRetFuncType->getAlignInBits());
    }
    return retType;
}

static Value* mycast(Value* value, Type* type, CodeGenContext& cgcontext) {
    if (type == value->getType())
        return value;
    if (type == Type::getDoubleTy(*cgcontext.context)) {
        if (value->getType() == Type::getInt32Ty(*cgcontext.context) || value->getType() == Type::getInt8Ty(*cgcontext.context))
            value = new SIToFPInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getFloatTy(*cgcontext.context))
            value = new FPExtInst(value, type, "", cgcontext.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    }
    else if (type == Type::getFloatTy(*cgcontext.context)) {
        if (value->getType() == Type::getInt32Ty(*cgcontext.context) || value->getType() == Type::getInt8Ty(*cgcontext.context))
            value = new SIToFPInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getDoubleTy(*cgcontext.context))
            value = new FPTruncInst(value, type, "", cgcontext.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    }
    else if (type == Type::getInt32Ty(*cgcontext.context)) {
        if (value->getType() == Type::getDoubleTy(*cgcontext.context))
            value = new FPToSIInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt8Ty(*cgcontext.context))
            value = new SExtInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt32Ty(*cgcontext.context))
            value = new ZExtInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt8PtrTy(*cgcontext.context))
            value = new PtrToIntInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt32PtrTy(*cgcontext.context))
            value = new PtrToIntInst(value, type, "", cgcontext.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    } else if (type == Type::getInt8Ty(*cgcontext.context)) {
        if (value->getType() == Type::getDoubleTy(*cgcontext.context))
            value = new FPToSIInst(value, type, "", cgcontext.currentBlock());
        else if (value->getType() == Type::getInt32Ty(*cgcontext.context))
            value = new TruncInst(value, type, "", cgcontext.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    } else if (type == Type::getInt8PtrTy(*cgcontext.context)) {
        value = cgcontext.builder->CreateBitCast(value, type);
    }
    //else
        //llvm::errs() << "[ERROR] Cannot cast this value.\n";
    return value;
}

llvm::Value *IntExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    return ConstantInt::get(Type::getInt32Ty(*cgcontext.context), value, true);
}

llvm::Value *RealExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    return ConstantFP::get(Type::getDoubleTy(*cgcontext.context), value);
}

llvm::Value *FloatExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    return ConstantFP::get(Type::getFloatTy(*cgcontext.context), value);
}

llvm::Value *CharExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    return ConstantInt::get(Type::getInt8Ty(*cgcontext.context), value, true);
}

llvm::Value *StringExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    return cgcontext.builder->CreateGlobalStringPtr(value);
}

llvm::Value *NilExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    if (type == nullptr) return nullptr;
    Type *retType = getTypeFromExprNode(cgcontext, type);
    llvm::Value* nil = nullptr;
    if (retType->isStructTy()) {
        nil = ConstantPointerNull::get(retType->getPointerTo());
    }
    else if (retType->isPointerTy()) {
        if (!retType->getPointerElementType()->isPointerTy()) {
            nil = ConstantPointerNull::get(static_cast<PointerType *>(retType));
        }
    }
    else {
        nil = ConstantPointerNull::get(retType->getPointerTo());
    }

    if (nil == nullptr) llvm::errs() << "[ERROR] Cannot cast this value.\n";
    return nil;
}

llvm::Value *ArrayExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *FuncExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *BooleanExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    return ConstantInt::get(Type::getInt1Ty(*cgcontext.context), value, false);
}

llvm::Value *VariableExprNode::codegen(CodeGenContext &cgcontext) {
    Value *val = nullptr;
    Type *type = nullptr;
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), (ExprNode*)this);
    // check is it function pointer
    //if (cgcontext.isFuncPointerAssignment)
    {
        auto func = cgcontext.lookupFuncs(value);
        if (func == nullptr) {
            func = cgcontext.lookupFuncs(cgcontext.moduleName + "." + value);
        }
        if (func != nullptr) {
            auto funcDecl = dynamic_cast<FuncDecStatementNode*>(func);
            auto externFuncDecl = dynamic_cast<ExternFuncDecStatementNode*>(func);
            auto methodDecl = dynamic_cast<MethodDecNode*>(func);

            Function *function = cgcontext.mModule->getFunction(value + cgcontext.currentNameAddition);

            if (function == nullptr) {
                function = cgcontext.mModule->getFunction(cgcontext.moduleName + "." + value + cgcontext.currentNameAddition);
                if (function == nullptr) {
                    //llvm::errs() << "[ERROR] Codegen - no such function \"" << value << "\".\n";
                    return nullptr;
                }
            }
            val = function;
            type = val->getType();
            return val;
        }
    }
    // check local variables
    val = cgcontext.localsLookup(value);
    // check global variables if there is no local
    if (val == nullptr)
    {
        // check global variable declared in current module
        if (cgcontext.globals().find(cgcontext.moduleName + "." + value) == cgcontext.globals().end())
        {
            // check other global variables
            if (cgcontext.globals().find(value) == cgcontext.globals().end())
            {
                // check type fields
                auto typeVarName = value.substr(0, value.rfind('.'));
                auto typeVar = cgcontext.localsLookup(typeVarName);
                if (typeVar == nullptr)
                {
                    if (cgcontext.globals().contains(cgcontext.moduleName + "." + typeVarName))
                        typeVar = cgcontext.globals().find(cgcontext.moduleName + "." + typeVarName)->second;

                }
                if (typeVar != nullptr)
                {
                    Type* typeOfTypeVar = nullptr;
                    // type variable exists, get type
                    // we should check is it pointer to pointer to struct or just pointer to struct
                    if (dotClass)
                    {
                        if (typeVar->getType()->getPointerElementType()->isStructTy()) {
                            typeOfTypeVar = typeVar->getType()->getPointerElementType();
                            auto elementPtr = cgcontext.builder->CreateStructGEP(typeOfTypeVar, typeVar, index);
                            val = cgcontext.builder->CreateLoad(elementPtr);
                        }
                        else if (typeVar->getType()->getPointerElementType()->getPointerElementType()->isStructTy()) {
                            typeOfTypeVar = typeVar->getType()->getPointerElementType()->getPointerElementType();
                            auto loadThis = cgcontext.builder->CreateLoad(typeVar);
                            cgcontext.currentTypeLoad = loadThis;
                            auto elementPtr = cgcontext.builder->CreateStructGEP(typeOfTypeVar, loadThis, index);
                            val = cgcontext.builder->CreateLoad(elementPtr);
                        }
                        if (dynamic_cast<IndexExprNode*>(this) != nullptr)
                        {
                            auto elementPtr = cgcontext.builder->CreateInBoundsGEP(val, dynamic_cast<IndexExprNode*>(this)->indexExpr->codegen(cgcontext));
                            return cgcontext.builder->CreateLoad(elementPtr);
                        }
                        else if (dynamic_cast<IndexesExprNode*>(this) != nullptr)
                        {
                            auto indexesExpr = dynamic_cast<IndexesExprNode*>(this);
                            /*auto arrExpr = dynamic_cast<ArrayDecStatementNode*>(cgcontext.localsExprsLookup(value));
                            if (arrExpr == nullptr) {
                                auto field = dynamic_cast<FieldArrayVarDecNode *>(cgcontext.localsExprsLookup(value));
                                if (field != nullptr) arrExpr = field->var;
                            }
                            if (arrExpr == nullptr) {
                                llvm::errs() << "[ERROR] Variable \"" << value << "\" is not an array.\n";
                            }
                            if (arrExpr->indicesCount > indexesExpr->indexes->size()) {
                                llvm::errs() << "[ERROR] Too much indexes in expression. (\"" << value << "\")n";
                            }*/
                            auto loadArr = val;
                            llvm::Value *elementPtr = nullptr;
                            for (int i = 0; i < indexesExpr->indexes->size(); i++) {
                                auto index = indexesExpr->indexes->at(i)->codegen(cgcontext);
                                elementPtr = cgcontext.builder->CreateInBoundsGEP(loadArr, index);
                                loadArr = cgcontext.builder->CreateLoad(elementPtr);
                            }
                            auto ret = cgcontext.builder->CreateLoad(elementPtr);

                            return ret;
                        }
                        else
                            return val;
                    }
                    else llvm::errs() << "[ERROR] Variable \"" << typeVarName << "\" is not a custom type.\n";
                    type = typeOfTypeVar;
                }
                else llvm::errs() << "[ERROR] Undeclared Variable \"" << value << "\".\n";
            }
            else
            {
                val = cgcontext.globals()[value];
            }
        }
        else
        {
            val = cgcontext.globals()[cgcontext.moduleName + "." + value];
        }
    }

    if (dynamic_cast<IndexExprNode*>(this) != nullptr)
    {
        auto loadArr = cgcontext.builder->CreateLoad(val);
        auto elementPtr = cgcontext.builder->CreateInBoundsGEP(loadArr, dynamic_cast<IndexExprNode*>(this)->indexExpr->codegen(cgcontext));
        return cgcontext.builder->CreateLoad(elementPtr);
    }

    if (dynamic_cast<IndexesExprNode*>(this) != nullptr)
    {
        auto indexesExpr = dynamic_cast<IndexesExprNode*>(this);
        /*auto arrExpr = dynamic_cast<ArrayDecStatementNode*>(cgcontext.localsExprsLookup(value));
        if (arrExpr == nullptr) {
            auto field = dynamic_cast<FieldArrayVarDecNode *>(cgcontext.localsExprsLookup(value));
            if (field != nullptr) arrExpr = field->var;
        }
        if (arrExpr == nullptr) {
            llvm::errs() << "[ERROR] Variable \"" << value << "\" is not an array.\n";
        }
        if (arrExpr->indicesCount > indexesExpr->indexes->size()) {
            llvm::errs() << "[ERROR] Too much indexes in expression. (\"" << value << "\")n";
        }*/
        auto loadArr = cgcontext.builder->CreateLoad(val);
        llvm::Value *elementPtr = nullptr;
        for (int i = 0; i < indexesExpr->indexes->size(); i++) {
            auto index = indexesExpr->indexes->at(i)->codegen(cgcontext);
            elementPtr = cgcontext.builder->CreateInBoundsGEP(loadArr, index);
            loadArr = cgcontext.builder->CreateLoad(elementPtr);
        }
        auto ret = cgcontext.builder->CreateLoad(elementPtr);

        return ret;
    }

    if (val == nullptr) return nullptr;
    if (type == nullptr)
        type = val->getType()->getPointerElementType();
    if (val->getType()->getPointerElementType()->isStructTy()) return val;
    if (cgcontext.localsExprs()[value] != nullptr && cgcontext.callingExpr)
    {
        if (dynamic_cast<ArrayDecStatementNode*>(cgcontext.localsExprs()[value]) != nullptr)
            if (val->getType()->getPointerElementType()->isPointerTy())
                if (val->getType()->getPointerElementType()->getPointerElementType()->isStructTy())
                    return val;
    }

    // if it's a "var" func param, load it
    // example:
    //     private procedure swap(var real a, var real b):
    //        variable-real temp := a;  // real temp = *a;
    //        let a := b;               // *a = *b;
    //        let b := temp;            // *b = temp;
    //    end swap;
    auto varExpr = cgcontext.localsExprsLookup(value);
    if (varExpr != nullptr) {
        if (dynamic_cast<FuncParamDecStatementNode*>(varExpr) != nullptr) {
            auto funcParam = dynamic_cast<FuncParamDecStatementNode*>(varExpr);
            if (funcParam->parameterType == ParameterType::Var) {
                if (val->getType()->getPointerElementType()->isPointerTy()) {
                    type = val->getType()->getPointerElementType()->getPointerElementType();
                    val = cgcontext.builder->CreateLoad(val);
                }
            }
        }
    }
    return new LoadInst(type, val, "", false, cgcontext.currentBlock());
}

llvm::Value *UnaryOperatorExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    auto val = right->codegen(cgcontext);
    if (val->getType()->isIntegerTy())
        return BinaryOperator::CreateNeg(right->codegen(cgcontext), "", cgcontext.currentBlock());
    else if (val->getType()->isFloatingPointTy())
        return BinaryOperator::CreateFSub(ConstantFP::get(val->getType(), 0), val, "", cgcontext.currentBlock());
    return nullptr;
}

llvm::Value *OperatorExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    Value* leftVal = left->codegen(cgcontext);
    Value* rightVal = right->codegen(cgcontext);
    bool castNeeded = true;
    /*
    if (dynamic_cast<NilExprNode*>(left) != nullptr && rightVal != nullptr)
    {
        if (rightVal->getType()->isStructTy())
            rightVal = cgcontext.builder->CreateLoad(rightVal);
        auto ptype = static_cast<PointerType*>(rightVal->getType()->getPointerElementType());
        leftVal = ConstantPointerNull::get(ptype);
        castNeeded = false;
    }
    if (dynamic_cast<NilExprNode*>(right) != nullptr && leftVal != nullptr)
    {
        if (leftVal->getType()->isStructTy())
            leftVal = cgcontext.builder->CreateLoad(leftVal);
        auto ptype = static_cast<PointerType*>(leftVal->getType()->getPointerElementType());
        rightVal = ConstantPointerNull::get(ptype);
        castNeeded = false;
    }
    leftVal->print(llvm::errs() << "\n");
    rightVal->print(llvm::errs() << "\n\n");*/
    bool floatOp = false;
    if (castNeeded)
    {
        if (leftVal->getType()->isDoubleTy() || rightVal->getType()->isDoubleTy() || leftVal->getType()->isFloatTy() || rightVal->getType()->isFloatTy())
        {
            leftVal = mycast(leftVal, Type::getDoubleTy(*cgcontext.context), cgcontext);
            rightVal = mycast(rightVal, Type::getDoubleTy(*cgcontext.context), cgcontext);
            floatOp = true;
        }
        else if (leftVal->getType() == rightVal->getType())
        {

        }
        else {
            leftVal = mycast(leftVal, Type::getInt32Ty(*cgcontext.context), cgcontext);
            rightVal = mycast(rightVal, Type::getInt32Ty(*cgcontext.context), cgcontext);
        }
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
            case TokenType::Remainder:
                return BinaryOperator::Create(Instruction::SRem, leftVal, rightVal, "", cgcontext.currentBlock());
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
            case TokenType::Remainder:
                return BinaryOperator::Create(Instruction::FRem, leftVal, rightVal, "", cgcontext.currentBlock());
            case TokenType::Equal:
                return new FCmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OEQ, leftVal, rightVal, "");
            case TokenType::NotEqual:
                return new FCmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_ONE, leftVal, rightVal, "");
            case TokenType::Greater:
                return new FCmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OGT, leftVal, rightVal, "");
            case TokenType::GreaterOrEqual:
                return new FCmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OGE, leftVal, rightVal, "");
            case TokenType::Less:
                return new FCmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OLT, leftVal, rightVal, "");
            case TokenType::LessOrEqual:
                return new FCmpInst(*cgcontext.currentBlock(), ICmpInst::FCMP_OLE, leftVal, rightVal, "");
        }
    }
    llvm::errs() << "[ERROR] Unknown or wrong operator.\n";
    return nullptr;
}

llvm::Value *ConditionalExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *CallExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), (ExprNode*)this);
    std::vector<Value*> argsRef;
    std::string nameAddition;
    llvm::raw_string_ostream nameAdditionStream(nameAddition);
    auto foundFunc = cgcontext.lookupFuncs(name->value);
    auto funcDecl = dynamic_cast<FuncDecStatementNode*>(foundFunc);
    auto externFuncDecl = dynamic_cast<ExternFuncDecStatementNode*>(foundFunc);
    auto methodDecl = dynamic_cast<MethodDecNode*>(foundFunc);
    auto funcPtrDecl = dynamic_cast<FuncPointerStatementNode*>(cgcontext.lookupFuncPointers(name->value));
    int i = 0;
    cgcontext.callingExpr = true;
    for (auto it = args->begin(); it != args->end(); it++, i++)
    {
        // TODO: CreateLoad if args is out/var (it should fix this.method() calls too
        auto var = (*it)->codegen(cgcontext);
        auto loadValue = var;
        if (var->getType()->isPointerTy())
        {
            if (var->getType()->getPointerElementType()->isStructTy())
            {
                loadValue->getType()->print(nameAdditionStream);
                argsRef.push_back(loadValue);
                continue;
            }
        }
        if (dynamic_cast<NilExprNode*>(*it) == nullptr)
        {
            if (funcDecl != nullptr) {
                if (i < funcDecl->args->size()) {
                    auto currArg = funcDecl->args->at(i);
                    auto argType = getTypeFromExprNode(cgcontext, currArg->type);
                    if (currArg->parameterType == ParameterType::Out || currArg->parameterType == ParameterType::Var) {
                        if (loadValue->getType()->isPointerTy()) {
                            if (!loadValue->getType()->getPointerElementType()->isFunctionTy())
                                loadValue = getPointerOperand(loadValue);
                        }
                        else {
                            loadValue = getPointerOperand(loadValue);
                        }
                    }
                    if (argType != loadValue->getType()) {
                        if (currArg->parameterType == ParameterType::Out || currArg->parameterType == ParameterType::Var) {
                            if (loadValue->getType() != getTypeFromExprNode(cgcontext, currArg->type)->getPointerTo())
                                loadValue = mycast(var, getTypeFromExprNode(cgcontext, currArg->type)->getPointerTo(), cgcontext);
                        }
                        else {
                            loadValue = mycast(loadValue, getTypeFromExprNode(cgcontext, currArg->type), cgcontext);
                        }
                    }
                }
            }
            if (externFuncDecl != nullptr) {
                if (i < externFuncDecl->args->size()) {
                    auto currArg = externFuncDecl->args->at(i);
                    auto argType = getTypeFromExprNode(cgcontext, currArg->type);
                    if (currArg->parameterType == ParameterType::Out || currArg->parameterType == ParameterType::Var) {
                        if (loadValue->getType()->isPointerTy()) {
                            if (!loadValue->getType()->getPointerElementType()->isFunctionTy())
                                loadValue = getPointerOperand(loadValue);
                        }
                        else {
                            loadValue = getPointerOperand(loadValue);
                        }
                    }
                    if (argType != loadValue->getType()) {
                        if (currArg->parameterType == ParameterType::Out || currArg->parameterType == ParameterType::Var) {
                            if (loadValue->getType() != getTypeFromExprNode(cgcontext, currArg->type)->getPointerTo())
                                loadValue = mycast(var, getTypeFromExprNode(cgcontext, currArg->type)->getPointerTo(), cgcontext);
                        }
                        else {
                            loadValue = mycast(loadValue, getTypeFromExprNode(cgcontext, currArg->type), cgcontext);
                        }
                    }
                }
            }
            if (methodDecl != nullptr) {
                if (i < methodDecl->args->size()) {
                    auto currArg = methodDecl->args->at(i);
                    auto argType = getTypeFromExprNode(cgcontext, currArg->type);
                    if (currArg->parameterType == ParameterType::Out || currArg->parameterType == ParameterType::Var) {
                        if (loadValue->getType()->isPointerTy()) {
                            if (!loadValue->getType()->getPointerElementType()->isFunctionTy())
                                loadValue = getPointerOperand(loadValue);
                        }
                        else {
                            loadValue = getPointerOperand(loadValue);
                        }
                    }
                    if (argType != loadValue->getType()) {
                        if (currArg->parameterType == ParameterType::Out || currArg->parameterType == ParameterType::Var) {
                            if (loadValue->getType() != getTypeFromExprNode(cgcontext, currArg->type)->getPointerTo())
                                loadValue = mycast(var, getTypeFromExprNode(cgcontext, currArg->type)->getPointerTo(), cgcontext);
                        }
                        else {
                            loadValue = mycast(loadValue, getTypeFromExprNode(cgcontext, currArg->type), cgcontext);
                        }
                    }
                }
            }
            if (funcPtrDecl != nullptr) {
                if (i < funcPtrDecl->args->size()) {
                    auto currArg = funcPtrDecl->args->at(i);
                    auto argType = getTypeFromExprNode(cgcontext, currArg->type, currArg->parameterType);
                    if (currArg->parameterType == ParameterType::Out || currArg->parameterType == ParameterType::Var) {
                        if (loadValue->getType()->isPointerTy()) {
                            if (!loadValue->getType()->getPointerElementType()->isFunctionTy())
                                loadValue = getPointerOperand(loadValue);
                        }
                        else {
                            loadValue = getPointerOperand(loadValue);
                        }
                    }
                    if (argType != loadValue->getType()) {
                        if (currArg->parameterType == ParameterType::Out || currArg->parameterType == ParameterType::Var) {
                            if (loadValue->getType() != getTypeFromExprNode(cgcontext, currArg->type)->getPointerTo())
                                loadValue = mycast(var, getTypeFromExprNode(cgcontext, currArg->type)->getPointerTo(), cgcontext);
                        }
                        else {
                            loadValue = mycast(loadValue, getTypeFromExprNode(cgcontext, currArg->type), cgcontext);
                        }
                    }
                }
            }
        }
        //auto arg = dynamic_cast<FuncParamDecStatementNode*>(*it);
        //if (arg->parameterType == ParameterType::Out || arg->parameterType == ParameterType::Var)
            //cgcontext.builder->CreateLoad(var);
        loadValue->getType()->print(nameAdditionStream);
        argsRef.push_back(loadValue);
    }
    cgcontext.callingExpr = false;
    Function *function = nullptr;
    function = cgcontext.mModule->getFunction(name->value + nameAddition);
    if (function == nullptr) {
        llvm::Value* var = name->codegen(cgcontext);
        bool err = false;
        if (argsRef.size() == var->getType()->getPointerElementType()->getFunctionNumParams()) {
            for (int j = 0; j < argsRef.size(); j++) {
                if (argsRef[j]->getType() != var->getType()->getPointerElementType()->getFunctionParamType(j)) {
                    err = true;
                    break;
                }
            }
        }
        else {
            err = true;
        }
        if (err) {
            llvm::errs() << "[ERROR] Calling a function " << name->value << " with bad signature.\n";
            return nullptr;
        }
        auto call = cgcontext.builder->CreateCall(static_cast<FunctionType*>(var->getType()->getPointerElementType()), var, makeArrayRef(argsRef));
        return call;
    }
    if (function == nullptr)
        llvm::errs() << "[ERROR] Codegen - no such function \"" << name->value + nameAddition << "\".\n";
    //CallInst *call = CallInst::Create(function, makeArrayRef(argsRef), "", cgcontext.currentBlock());
    auto call = cgcontext.builder->CreateCall(function, makeArrayRef(argsRef), "");
    return call;
}

llvm::Value *DeleteExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    auto var = expr->codegen(cgcontext);
    // GC_
    //auto call = cgcontext.builder->CreateCall(cgcontext.mModule->getFunction("GC_free"), {var});

    auto call = CallInst::CreateFree(var, cgcontext.currentBlock());
    cgcontext.builder->Insert(call);
    return call;
    //return nullptr;
}

llvm::Value *BlockExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

std::vector<ExprNode*>* getSizesOfArray(DeclarationNode* expr) {
    auto sizes = new std::vector<ExprNode*>();
    auto array = dynamic_cast<ArrayDecStatementNode*>(expr);
    auto exprNode = dynamic_cast<ArrayExprNode*>(array->expr);
    auto arrExpr = exprNode;
    sizes->push_back(arrExpr->size);
    if (arrExpr->type == "array")
    {
        for (auto &slice : *arrExpr->values)
        {
            sizes->push_back(dynamic_cast<ArrayExprNode*>(slice)->size);
        }
    }
    return sizes;
}

llvm::Value* calcIndex(IndexesExprNode* expr, CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *AssignExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    auto assignData = right->codegen(cgcontext);
    Value* var;
    Type *type = nullptr;
    // check local variables
    var = cgcontext.localsLookup(left->value);
    auto varExpr = cgcontext.localsExprsLookup(left->value);
    // check global variables if there is no local
    if (var == nullptr)
    {
        // check global variable declared in current module
        if (cgcontext.globals().find(cgcontext.moduleName + "." + left->value) == cgcontext.globals().end())
        {
            // check other global variables
            if (cgcontext.globals().find(left->value) == cgcontext.globals().end())
            {
                // check type fields
                auto typeVarName = left->value.substr(0, left->value.rfind('.'));
                auto typeVar = cgcontext.localsLookup(typeVarName);
                auto typeExpr = cgcontext.localsExprsLookup(typeVarName);
                if (typeVar != nullptr)
                {
                    Type* typeOfTypeVar = nullptr;
                    // type variable exists, get type
                    // we should check is it pointer to pointer to struct or just pointer to struct
                    if (left->dotClass)
                    {
                        if (typeVar->getType()->getPointerElementType()->isStructTy()) {
                            typeOfTypeVar = typeVar->getType()->getPointerElementType();
                            auto elementPtr = cgcontext.builder->CreateStructGEP(typeOfTypeVar, typeVar, left->index);
                            var = elementPtr;
                        }
                        else if (typeVar->getType()->getPointerElementType()->getPointerElementType()->isStructTy()) {
                            typeOfTypeVar = typeVar->getType()->getPointerElementType()->getPointerElementType();
                            auto loadThis = cgcontext.builder->CreateLoad(typeVar);
                            cgcontext.currentTypeLoad = loadThis;
                            auto elementPtr = cgcontext.builder->CreateStructGEP(typeOfTypeVar, loadThis, left->index);
                            var = elementPtr;
                        }

                    }
                    else llvm::errs() << "[ERROR] Variable \"" << typeVarName << "\" is not a custom type.\n";
                }
                else llvm::errs() << "[ERROR] Undeclared Variable \"" << left->value << "\".\n";
            }
            else
                var = cgcontext.globals()[left->value];
        }
        else
        {
            var = cgcontext.globals()[cgcontext.moduleName + "." + left->value];
        }
    }

    if (dynamic_cast<IndexExprNode*>(left) != nullptr)
    {
        auto loadArr = cgcontext.builder->CreateLoad(var);
        auto elementPtr = cgcontext.builder->CreateInBoundsGEP(loadArr, dynamic_cast<IndexExprNode*>(left)->indexExpr->codegen(cgcontext));
        if (assignData->getType() != var->getType()->getPointerElementType()) {
            assignData = mycast(assignData, elementPtr->getType()->getPointerElementType(), cgcontext);
        }
        auto ret = cgcontext.builder->CreateStore(assignData, elementPtr);
        return ret;
    }

    if (dynamic_cast<IndexesExprNode*>(left) != nullptr)
    {
        auto indexesExpr = dynamic_cast<IndexesExprNode*>(left);
        auto loadArr = cgcontext.builder->CreateLoad(var);
        llvm::Value *elementPtr = nullptr;
        for (int i = 0; i < indexesExpr->indexes->size(); i++) {
            auto index = indexesExpr->indexes->at(i)->codegen(cgcontext);
            elementPtr = cgcontext.builder->CreateInBoundsGEP(loadArr, index);
            loadArr = cgcontext.builder->CreateLoad(elementPtr);
        }
        if (assignData->getType() != var->getType()->getPointerElementType()) {
            assignData = mycast(assignData, elementPtr->getType()->getPointerElementType(), cgcontext);
        }
        auto ret = cgcontext.builder->CreateStore(assignData, elementPtr);

        return ret;
    }

    // TODO: at least it works for arrays, check for other types
    if (dynamic_cast<NilExprNode*>(right) != nullptr)
    {
        auto ptype = static_cast<PointerType*>(var->getType());
        if (!ptype->isPointerTy()) ptype = static_cast<PointerType*>(var->getType()->getPointerElementType());
        assignData = ConstantPointerNull::get(ptype);
    }

    std::string nameAddition;
    llvm::raw_string_ostream nameAdditionStream(nameAddition);
    if (var->getType()->isPointerTy()) {
        if (var->getType()->getPointerElementType()->isPointerTy()) {
            if (var->getType()->getPointerElementType()->getPointerElementType()->isFunctionTy()) {
                auto func = var->getType()->getPointerElementType()->getPointerElementType();
                auto n = func->getFunctionNumParams();
                for (int i = 0; i < n; i++){
                    func->getFunctionParamType(i)->print(nameAdditionStream);
                }
                cgcontext.isFuncPointerAssignment = true;
                cgcontext.currentNameAddition = nameAddition;
                assignData = right->codegen(cgcontext);
                if (assignData == nullptr) {
                    llvm::errs() << "[ERROR] Codegen - no such function found when assigning to \"" << left->value << "\".\n";
                }
                cgcontext.isFuncPointerAssignment = false;
                cgcontext.currentNameAddition = "";
            }
        }
    }
    // what is that?
    if (assignData->getType() != var->getType()->getPointerElementType()) {
        assignData = mycast(assignData, var->getType()->getPointerElementType(), cgcontext);
    }

    if (var->getType()->isPointerTy()) {
        if (var->getType()->getPointerElementType()->isPointerTy()) {
            if (assignData->getType() == var->getType()->getPointerElementType()->getPointerElementType()) {
                var = cgcontext.builder->CreateLoad(var);
            }
        }
    }
    auto ret = cgcontext.builder->CreateStore(assignData, var);
    return ret;
}

llvm::Value *CastExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *ExprStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *ReturnStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    return cgcontext.builder->CreateRet(expr->codegen(cgcontext));
}
llvm::Value *OutputStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    Value *value = expr->codegen(cgcontext);
    std::vector<Value *> printArgs;
    Value *formatStr;
    if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() == 32) {
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%d");
    }
    else if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() == 8) {
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%c");
    }
    else if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() == 1) {
        value = cgcontext.builder->CreateIntCast(value, Type::getInt32Ty(*cgcontext.context), false);
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%d");
    }
    else if (value->getType()->isDoubleTy()) {
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%f");
    }
    else if (value->getType()->isFloatTy()) {
        value = cgcontext.builder->CreateFPExt(value, Type::getDoubleTy(*cgcontext.context));
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%f");
    }
    else if (value->getType()->isPointerTy()) {
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%s");
    }
    printArgs.push_back(formatStr);
    printArgs.push_back(value);
    llvm::Value* res = cgcontext.builder->CreateCall(cgcontext.mModule->getFunction("printf"), printArgs);
    return res;
}

llvm::Value *InputStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    Value *value = expr->codegen(cgcontext);
    std::vector<Value *> printArgs;
    Value *formatStr;
    if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() == 32) {
        value = getPointerOperand(value);
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%d");
    }
    else if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() == 8) {
        value = getPointerOperand(value);
        formatStr = cgcontext.builder->CreateGlobalStringPtr(" %c");
    }
    else if (value->getType()->isIntegerTy() && value->getType()->getIntegerBitWidth() == 1) {
        value = getPointerOperand(value);
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%d");
    }
    else if (value->getType()->isDoubleTy()) {
        value = getPointerOperand(value);
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%lf");
    }
    else if (value->getType()->isFloatTy()) {
        value = getPointerOperand(value);
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%f");
    }
    else if (value->getType()->isPointerTy()) {
        formatStr = cgcontext.builder->CreateGlobalStringPtr("%s");
    }
    printArgs.push_back(formatStr);
    printArgs.push_back(value);
    llvm::Value* res = cgcontext.builder->CreateCall(cgcontext.mModule->getFunction("scanf"), printArgs);
    return res;
}

llvm::Value *DeclarationNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *VarDecStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    Value* newVar = nullptr;
    Value* rightVal = nullptr;
    DIType* debugType = nullptr;
    if (isGlobal)
    {
        if (cgcontext.allocatedClasses.contains(type)) {
            cgcontext.mModule->getOrInsertGlobal(name->value, ptrToTypeOf(cgcontext, type));
            debugType = dbgPrToTypeOf(cgcontext, type);
        }
        else {
            cgcontext.mModule->getOrInsertGlobal(name->value, typeOf(cgcontext, type));
            debugType = dbgTypeOf(cgcontext, type);
        }
        //cgcontext.mModule->getOrInsertGlobal(name->value, typeOf(cgcontext, type));
        auto gVar = cgcontext.mModule->getNamedGlobal(name->value);
        if (isExtern) gVar->setLinkage(llvm::GlobalValue::ExternalLinkage);
        else gVar->setLinkage(GlobalValue::InternalLinkage);

        if (!isExtern)
        {
            if (expr != nullptr) {
                rightVal = expr->codegen(cgcontext);
                gVar->setInitializer(static_cast<Constant *>(rightVal));
            }
            else {
                if (cgcontext.allocatedClasses.contains(type)) {
                    rightVal = ConstantPointerNull::get(static_cast<PointerType *>(ptrToTypeOf(cgcontext, type)));
                }
                else {
                    rightVal = Constant::getNullValue(typeOf(cgcontext, type));
                }
                gVar->setInitializer(static_cast<Constant *>(rightVal));
            }
        }
        gVar->setAlignment(Align(8));
        cgcontext.globals()[name->value] = gVar;
        newVar = gVar;
        // get name of variable (after last dot) in modern c++
        std::string varName = name->value;
        auto pos = varName.find_last_of('.');
        if (pos != std::string::npos) {
            varName = varName.substr(pos + 1);
        }

        auto dbgInfo = cgcontext.debugBuilder->createGlobalVariableExpression(
                cgcontext.dbgInfo.compileUnit,
                varName,
                "",
                cgcontext.dbgInfo.compileUnit->getFile(),
                loc.line,
                debugType,
                isPrivate,
                gVar);
        gVar->addDebugInfo(dbgInfo);
        // we should call default constructor if global is pointer to struct
        if (cgcontext.allocatedClasses.contains(type))
        {
            FunctionType* funcType = FunctionType::get(Type::getVoidTy(*cgcontext.context), {}, false);
            Function *constructorFunc = Function::Create(funcType, Function::ExternalLinkage, Twine(name->value + "_constructor"), cgcontext.mModule);
            DIFile* unit = cgcontext.debugBuilder->createFile(cgcontext.dbgInfo.compileUnit->getFilename(), cgcontext.dbgInfo.compileUnit->getDirectory());
            llvm::DISubprogram *dbgConstructorFunc = cgcontext.debugBuilder->createFunction(
                    cgcontext.dbgInfo.compileUnit, name->value + "_constructor", name->value + "._DefaultConstructor_", unit, loc.line,
                    cgcontext.dbgInfo.CreateFunctionType({}), loc.line,
                    llvm::DISubprogram::FlagPrivate,
                    llvm::DISubprogram::SPFlagDefinition);

            cgcontext.dbgInfo.lexicalBlocks.push_back(dbgConstructorFunc);
            cgcontext.dbgInfo.emitLocation(cgcontext.builder.get());
            BasicBlock *bb = BasicBlock::Create(*cgcontext.context, name->value + "_constructorEntry", constructorFunc, nullptr);
            cgcontext.pushBlock(bb);
            cgcontext.builder->SetInsertPoint(bb);
            getOrCreateSanitizerCtorAndInitFunctions(
                    *cgcontext.mModule, cgcontext.moduleName + "_GLOBAL_" + name->value, name->value + "_constructor", {}, {},
                    // This callback is invoked when the functions are created the first
                    // time. Hook them into the global ctors list in that case:
                    [&](Function *Ctor, FunctionCallee) { appendToGlobalCtors(*cgcontext.mModule, Ctor, 65535); });

            llvm::Type* int32type = llvm::Type::getInt32Ty(*cgcontext.context);
            auto structType = typeOf(cgcontext, type);
            auto malloc = CallInst::CreateMalloc(cgcontext.currentBlock(), int32type, structType, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*cgcontext.context), cgcontext.mModule->getDataLayout().getTypeAllocSize(structType)), nullptr, cgcontext.mModule->getFunction("malloc"), "");
            cgcontext.builder->Insert(malloc);
            cgcontext.builder->CreateStore(malloc, newVar);
            auto structConstructorFunc = cgcontext.mModule->getFunction(type + "._DefaultConstructor_");
            if (constructorFunc != nullptr)
            {
                auto p = cgcontext.builder->CreateLoad(newVar);
                if (DEBUG) structConstructorFunc->print(llvm::errs());
                if (DEBUG) newVar->print(llvm::errs());

                // add debugloc to createcall
                cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
                cgcontext.builder->CreateCall(structConstructorFunc, {p});
            }

            cgcontext.builder->CreateRetVoid();
            cgcontext.popBlock();
            cgcontext.dbgInfo.lexicalBlocks.pop_back();
            constructorFunc->setSubprogram(dbgConstructorFunc);
        }
    }
    else
    {
        if (cgcontext.allocatedClasses.contains(type)) {
            newVar = cgcontext.builder->CreateAlloca(ptrToTypeOf(cgcontext, type), 0, nullptr, name->value);
            llvm::Type* int64type = llvm::Type::getInt32Ty(*cgcontext.context);
            auto structType = typeOf(cgcontext, type);
            // we should not malloc if we want to assign other pointer
            // TODO: recheck this
            if (expr == nullptr) {
                auto malloc = CallInst::CreateMalloc(cgcontext.currentBlock(), int64type, structType, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*cgcontext.context), cgcontext.mModule->getDataLayout().getTypeAllocSize(structType)), nullptr, cgcontext.mModule->getFunction("malloc"), "");
                cgcontext.builder->Insert(malloc);
                cgcontext.builder->CreateStore(malloc, newVar);
            }
            debugType = dbgPrToTypeOf(cgcontext, type);
        }
        else {
            newVar = cgcontext.builder->CreateAlloca(typeOf(cgcontext, type), 0, nullptr, name->value);
            debugType = dbgTypeOf(cgcontext, type);
        }
        cgcontext.locals()[name->value] = newVar;
        cgcontext.localsExprs()[name->value] = this;

        if (expr != nullptr) {
            rightVal = expr->codegen(cgcontext);
            //new StoreInst(rightVal, newVar, false, cgcontext.currentBlock());

            if (rightVal->getType() != newVar->getType()->getPointerElementType()) {
                rightVal = mycast(rightVal, newVar->getType()->getPointerElementType(), cgcontext);
            }
            auto newVarLoad = newVar;
            if (newVar->getType()->isPointerTy()) {
                if (newVar->getType()->getPointerElementType()->isPointerTy()) {
                    if (rightVal->getType() == newVar->getType()->getPointerElementType()->getPointerElementType()) {
                        newVarLoad = cgcontext.builder->CreateLoad(newVar);
                        cgcontext.builder->CreateStore(rightVal, newVarLoad);
                    }
                }
            }

            cgcontext.builder->CreateStore(rightVal, newVarLoad);
        }
        // call constructor only if we are not assigning pointer, for example:
        // variable-Type a := otherTypeObj; // in this case we should not call constructor after assigning
        if (cgcontext.allocatedClasses.contains(type) && expr == nullptr)
        {
            auto constructorFunc = cgcontext.mModule->getFunction(type + "._DefaultConstructor_");
            if (constructorFunc != nullptr)
            {
                auto p = cgcontext.builder->CreateLoad(newVar);
                if (DEBUG) newVar->print(llvm::errs());
                cgcontext.builder->CreateCall(constructorFunc, {p});
            }
        }

        DIFile* unit = cgcontext.debugBuilder->createFile(cgcontext.dbgInfo.compileUnit->getFilename(), cgcontext.dbgInfo.compileUnit->getDirectory());
        auto funcScope = cgcontext.dbgInfo.lexicalBlocks.back();
        auto dbg = cgcontext.debugBuilder->createAutoVariable(funcScope, name->value, unit, loc.line, debugType);
        cgcontext.debugBuilder->insertDeclare(newVar, dbg, cgcontext.debugBuilder->createExpression(), DILocation::get(funcScope->getContext(), loc.line, 0, funcScope), cgcontext.builder->GetInsertBlock());
    }

    return newVar;
}

llvm::Value *FuncParamDecStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

void generateMallocLoopsRecursive(CodeGenContext &cgcontext, int i, int indicesCount, Type* currentType, vector<ExprNode *>* sizes, std::vector<Value*> jvars, Value *currentArr, Value *var)
{
    if (i == indicesCount) return;
    Function *TheFunctionMain = cgcontext.builder->GetInsertBlock()->getParent();
    BasicBlock* whileCheckMain = BasicBlock::Create(*cgcontext.context, "", TheFunctionMain, 0);
    BasicBlock* whileIterMain = BasicBlock::Create(*cgcontext.context, "", TheFunctionMain, 0);
    BasicBlock* whileEndMain = BasicBlock::Create(*cgcontext.context, "", TheFunctionMain, 0);
    // Check condition satisfaction
    BranchInst::Create(whileCheckMain, cgcontext.currentBlock());
    cgcontext.pushBlock(whileCheckMain);
    cgcontext.builder->SetInsertPoint(whileCheckMain);
    // Whether break the loop
    auto jvar = cgcontext.builder->CreateLoad(jvars[i]);
    auto sz = sizes->at(i - 1)->codegen(cgcontext);
    auto cmpVal = cgcontext.builder->CreateICmp(ICmpInst::ICMP_SLT, jvar, sz, "j<sz");
    BranchInst::Create(whileIterMain, whileEndMain, cmpVal, cgcontext.currentBlock());
    cgcontext.popBlock();
    // Entering loop block
    cgcontext.pushBlock(whileIterMain);
    cgcontext.builder->SetInsertPoint(whileIterMain);

    // Statements go here
    currentType = currentType->getPointerElementType();
    jvar = cgcontext.builder->CreateLoad(jvars[i]);

    llvm::Type* int32type = llvm::Type::getInt32Ty(*cgcontext.context);
    auto allocSz = sizes->at(i)->codegen(cgcontext);

    auto currentElementSize = ConstantInt::get(int32type, cgcontext.dataLayout->getTypeAllocSize(currentType));
    auto allocSize = BinaryOperator::Create(Instruction::Mul, allocSz, currentElementSize, "", cgcontext.currentBlock());
    // GC_malloc
    currentArr = CallInst::CreateMalloc(cgcontext.currentBlock(), int32type, currentType, allocSize, nullptr, cgcontext.mModule->getFunction("malloc"), "");
    // malloc
    //auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int32type, type, allocSize, nullptr, nullptr, "");
    cgcontext.builder->Insert(currentArr);

    llvm::Value *arrLoad = cgcontext.builder->CreateLoad(var);
    llvm::Value *arrPtr = nullptr;

    for (int k = 1; k <= i; k++)
    {
        jvar = cgcontext.builder->CreateLoad(jvars[k]);

        auto sext = cgcontext.builder->CreateSExt(jvar, llvm::Type::getInt64Ty(*cgcontext.context));
        arrPtr = cgcontext.builder->CreateInBoundsGEP(arrLoad, sext);
        arrLoad = cgcontext.builder->CreateLoad(arrPtr);

    }

    cgcontext.builder->CreateStore(currentArr, arrPtr);

    if (i == indicesCount - 1)
    {
        jvar = cgcontext.builder->CreateLoad(jvars[i]);
        auto add = cgcontext.builder->CreateAdd(jvar, ConstantInt::get(int32type, 1));
        cgcontext.builder->CreateStore(add, jvars[i]);
    }
    else
    {
        cgcontext.builder->CreateStore(ConstantInt::get(int32type, 0), jvars[i + 1]);
    }

    //auto jvar1 = cgcontext.builder->CreateLoad(jvars[i - 1]);
    //auto formatStr = cgcontext.builder->CreateGlobalStringPtr("%d %d %d\n");
    //cgcontext.builder->CreateCall(cgcontext.mModule->getFunction("printf"), {formatStr, allocSz, jvar1, jvar});

    generateMallocLoopsRecursive(cgcontext, i + 1, indicesCount, currentType, sizes, jvars, currentArr, var);
    // Jump back to condition checking
    BranchInst::Create(whileCheckMain, cgcontext.currentBlock());

    cgcontext.popBlock();
    // Return END

    cgcontext.ret(whileEndMain);
    cgcontext.builder->SetInsertPoint(whileEndMain);
    if (i > 1) {
        auto endJvar = cgcontext.builder->CreateLoad(jvars[i - 1]);
        auto endAdd = cgcontext.builder->CreateAdd(endJvar, ConstantInt::get(int32type, 1));
        cgcontext.builder->CreateStore(endAdd, jvars[i - 1]);
    }
}

llvm::Value *ArrayDecStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    auto exprNode = dynamic_cast<ArrayExprNode*>(expr);
    auto arrExpr = exprNode;
    DIType *dbgType;
    if (isGlobal)
    {
        if (!exprNode->size->isConst) {
            llvm::errs() << "[ERROR] Codegen - global array " << name->value << " size should be constant.\n";
            exit(-1);
        }
    }
    auto size = exprNode->size->codegen(cgcontext);
    auto arraySize = size;
    auto sizes = getSizesOfArray(this);
    if (arrExpr->type == "array")
    {
        for (auto &slice : *arrExpr->values)
        {
            auto castedSlice = dynamic_cast<ArrayExprNode*>(slice);
            arrExpr = castedSlice;
        }
    }
    auto type = typeOf(cgcontext, arrExpr->type);
    dbgType = dbgPrToTypeOf(cgcontext, arrExpr->type);
    auto finalType = type;
    for (int i = 1; i < this->indicesCount; i++) {
        finalType = finalType->getPointerTo();
        dbgType = cgcontext.dbgInfo.createPointerType(dbgType, "array");
    }

    llvm::Type* int32type = llvm::Type::getInt32Ty(*cgcontext.context);
    Function *constructorFunc = nullptr;
    llvm::DISubprogram *dbgConstructorFunc = nullptr;
    if (isGlobal)
    {
        cgcontext.mModule->getOrInsertGlobal(name->value, finalType->getPointerTo());
        auto gVar = cgcontext.mModule->getNamedGlobal(name->value);
        if (isExtern) gVar->setLinkage(llvm::GlobalValue::ExternalLinkage);
        else gVar->setLinkage(GlobalValue::InternalLinkage);
        gVar->setInitializer(ConstantPointerNull::get(PointerType::get(finalType, 0)));
        gVar->setAlignment(Align(8));
        gVar->setDSOLocal(true);
        cgcontext.globals()[name->value] = gVar;

        FunctionType* funcType = FunctionType::get(Type::getVoidTy(*cgcontext.context), {}, false);
        constructorFunc = Function::Create(funcType, Function::ExternalLinkage, Twine(name->value + "_constructor"), cgcontext.mModule);
        DIFile* unit = cgcontext.debugBuilder->createFile(cgcontext.dbgInfo.compileUnit->getFilename(), cgcontext.dbgInfo.compileUnit->getDirectory());
        dbgConstructorFunc = cgcontext.debugBuilder->createFunction(
                cgcontext.dbgInfo.compileUnit, name->value + "_constructor", name->value + "._constructor_", unit, loc.line,
                cgcontext.dbgInfo.CreateFunctionType({}), loc.line,
                llvm::DISubprogram::FlagPrivate,
                llvm::DISubprogram::SPFlagDefinition);
        cgcontext.dbgInfo.lexicalBlocks.push_back(dbgConstructorFunc);
        cgcontext.dbgInfo.emitLocation(cgcontext.builder.get());
        BasicBlock *bb = BasicBlock::Create(*cgcontext.context, name->value + "_constructorEntry", constructorFunc, nullptr);
        cgcontext.pushBlock(bb);
        cgcontext.builder->SetInsertPoint(bb);

        getOrCreateSanitizerCtorAndInitFunctions(
                *cgcontext.mModule, cgcontext.moduleName + "_GLOBAL_" + name->value, name->value + "_constructor", {}, {},
                // This callback is invoked when the functions are created the first
                // time. Hook them into the global ctors list in that case:
                [&](Function *Ctor, FunctionCallee) { appendToGlobalCtors(*cgcontext.mModule, Ctor, 65535); });
    }
    else
    {
        auto lVar = cgcontext.builder->CreateAlloca(finalType->getPointerTo(), 0, nullptr, name->value);
        cgcontext.locals()[name->value] = lVar;
        cgcontext.localsExprs()[name->value] = this;
    }

    llvm::Value *var = nullptr;
    if (isGlobal)
        var = cgcontext.mModule->getNamedGlobal(name->value);
    else
        var = cgcontext.locals()[name->value];

    auto elementSize = ConstantInt::get(int32type, cgcontext.dataLayout->getTypeAllocSize(finalType));
    auto allocSize = BinaryOperator::Create(Instruction::Mul, elementSize, arraySize, "", cgcontext.currentBlock());
    // GC_malloc
    auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int32type, finalType, allocSize, nullptr, cgcontext.mModule->getFunction("malloc"), "");
    // malloc
    //auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int32type, type, allocSize, nullptr, nullptr, "");
    cgcontext.builder->Insert(arr);
    cgcontext.builder->CreateStore(arr, var);

    auto currentArr = arr;
    auto currentType = finalType;

    if (indicesCount > 1)
    {
        std::vector<llvm::Value*> jvars;
        jvars.reserve(indicesCount);
        for (int i = 0; i < indicesCount; i++) {
            jvars.push_back(cgcontext.builder->CreateAlloca(int32type, 0, nullptr, "j" + std::to_string(i)));
            cgcontext.builder->CreateStore(ConstantInt::get(int32type, 0), jvars[i]);
        }
        int i = 1;

        generateMallocLoopsRecursive(cgcontext, i, indicesCount, currentType, sizes, jvars, currentArr, var);
    }

    if (isGlobal)
    {
        auto gVar = cgcontext.mModule->getNamedGlobal(name->value);
        // get var name after last dot
        auto varName = name->value;
        auto pos = varName.find_last_of('.');
        if (pos != std::string::npos)
            varName = varName.substr(pos + 1);

        auto dbgInfo = cgcontext.debugBuilder->createGlobalVariableExpression(
                cgcontext.dbgInfo.compileUnit,
                varName,
                "",
                cgcontext.dbgInfo.compileUnit->getFile(),
                loc.line,
                dbgType,
                isPrivate,
                var);
        gVar->addDebugInfo(dbgInfo);
        cgcontext.builder->CreateRetVoid();
        cgcontext.popBlock();
        cgcontext.dbgInfo.lexicalBlocks.pop_back();
        constructorFunc->setSubprogram(dbgConstructorFunc);
        return var;
    }
    else
    {
        auto lVar = cgcontext.locals()[name->value];
        DIFile* unit = cgcontext.debugBuilder->createFile(cgcontext.dbgInfo.compileUnit->getFilename(), cgcontext.dbgInfo.compileUnit->getDirectory());
        auto funcScope = cgcontext.dbgInfo.lexicalBlocks.back();
        auto dbg = cgcontext.debugBuilder->createAutoVariable(funcScope, name->value, unit, loc.line, dbgType);
        cgcontext.debugBuilder->insertDeclare(lVar, dbg, cgcontext.debugBuilder->createExpression(), DILocation::get(funcScope->getContext(), loc.line, 0, funcScope), cgcontext.builder->GetInsertBlock());
        //cgcontext.builder->CreateStore(arr, lVar);
        return var;
    }
    //ArrayType *arrayType = ArrayType::get(type, reinterpret_cast<ConstantInt*>(arraySize)->getZExtValue());
    //AllocaInst *alloc = new AllocaInst(arrayType, value->value, cgcontext.currentBlock());
    // auto var = new AllocaInst(arrayType, 0, value->value, cgcontext.currentBlock());
    return nullptr;
}

llvm::Value *IndexExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), (ExprNode*)this);
    Value* var;
    Type *type = nullptr;
    // check local variables
    var = cgcontext.localsLookup(value);
    // check global variables if there is no local
    if (var == nullptr)
    {
        // check global variable declared in current module
        if (cgcontext.globals().find(cgcontext.moduleName + "." + value) == cgcontext.globals().end())
        {
            // check other global variables
            if (cgcontext.globals().find(value) == cgcontext.globals().end())
            {
                // check type fields
                auto typeVarName = value.substr(0, value.rfind('.'));
                auto typeVar = cgcontext.localsLookup(typeVarName);
                if (typeVar != nullptr)
                {
                    Type* typeOfTypeVar = nullptr;
                    // type variable exists, get type
                    // we should check is it pointer to pointer to struct or just pointer to struct
                    if (dotClass)
                    {
                        if (typeVar->getType()->getPointerElementType()->isStructTy()) {
                            typeOfTypeVar = typeVar->getType()->getPointerElementType();
                            auto elementPtr = cgcontext.builder->CreateStructGEP(typeOfTypeVar, typeVar, index);
                            var = elementPtr;
                        }
                        else if (typeVar->getType()->getPointerElementType()->getPointerElementType()->isStructTy()) {
                            typeOfTypeVar = typeVar->getType()->getPointerElementType()->getPointerElementType();
                            auto loadThis = cgcontext.builder->CreateLoad(typeVar);
                            cgcontext.currentTypeLoad = loadThis;
                            auto elementPtr = cgcontext.builder->CreateStructGEP(typeOfTypeVar, loadThis, index);
                            var = elementPtr;
                        }
                    }
                    else llvm::errs() << "[ERROR] Variable \"" << typeVarName << "\" is not a custom type.\n";
                }
                else llvm::errs() << "[ERROR] Undeclared Variable \"" << value << "\".\n";
            }
            else
                var = cgcontext.globals()[value];
        }
        else
        {
            var = cgcontext.globals()[cgcontext.moduleName + "." + value];
        }
    }

    auto loadArr = cgcontext.builder->CreateLoad(var);
    auto elementPtr = cgcontext.builder->CreateInBoundsGEP(loadArr, indexExpr->codegen(cgcontext));
    if (isPointer) return elementPtr;
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
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    if (dynamic_cast<ExternFuncDecStatementNode*>(this) != nullptr) {
        return dynamic_cast<ExternFuncDecStatementNode*>(this)->codegen(cgcontext);
    }
    std::vector<llvm::Type*> argTypes;
    std::vector<int> refParams;
    int i = 1;
    std::string nameAddition;
    llvm::raw_string_ostream nameAdditionStream(nameAddition);
    for (auto arg : *args)
    {
        Type* argType;
        if (arg->parameterType == ParameterType::Out || arg->parameterType == ParameterType::Var)
        {
            // not tested
            argType = getTypeFromExprNode(cgcontext, arg->type)->getPointerTo();
            argTypes.push_back(argType);
            refParams.push_back(i);
        }
        else
        {
            argType = getTypeFromExprNode(cgcontext, arg->type);
            argTypes.push_back(argType);
        }
        argType->print(nameAdditionStream);
        i++;
    }
    Type *retType = getTypeFromExprNode(cgcontext, type);
    DIType *dbgRetType = getDbgTypeFromExprNode(cgcontext, type);
    if (retType->isStructTy()) {
        retType = retType->getPointerTo();
        dbgRetType = getDbgTypeFromExprNode(cgcontext, type, ParameterType::Out);
    }
    std::string retTypeName;

    FunctionType* funcType = FunctionType::get(retType, argTypes, false);
    auto function = Function::Create(funcType, GlobalValue::ExternalLinkage, name->value + nameAddition, cgcontext.mModule);
    for (auto paramID : refParams)
    {
        // TODO Set Dereferenceable attribute for ref params
        //function->addAttribute(paramID, Attribute::Dereferenceable);
    }

    DIFile* unit = cgcontext.debugBuilder->createFile(cgcontext.dbgInfo.compileUnit->getFilename(), cgcontext.dbgInfo.compileUnit->getDirectory());
    llvm::DISubprogram *dbgFunc = cgcontext.debugBuilder->createFunction(
            cgcontext.dbgInfo.compileUnit, name->value, name->value, unit, loc.line,
            cgcontext.dbgInfo.CreateFunctionType({dbgRetType}), loc.line,
            llvm::DISubprogram::FlagPrivate,
            llvm::DISubprogram::SPFlagDefinition);

    cgcontext.dbgInfo.lexicalBlocks.push_back(dbgFunc);
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get());
    // TODO: debug

    if (block != nullptr) {
        BasicBlock *bb = BasicBlock::Create(*cgcontext.context, cgcontext.moduleName + retTypeName + name->value + "Entry",
                                            function, 0);
        cgcontext.pushBlock(bb);
        cgcontext.builder->SetInsertPoint(bb);
        cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), block);

        Function::arg_iterator argsValues = function->arg_begin();
        int i = 1;
        for (auto it = args->begin(); it != args->end(); it++, argsValues++) {
            if ((*it)->parameterType == ParameterType::Out || (*it)->parameterType == ParameterType::Var) {
                auto var = new AllocaInst(getTypeFromExprNode(cgcontext, (*it)->type)->getPointerTo(), 0, nullptr, (*it)->name->value, bb);
                cgcontext.locals()[(*it)->name->value] = var;
                cgcontext.localsExprs()[(*it)->name->value] = (*it);
                Value *argumentValue = &(*argsValues);
                argumentValue->setName(getParameterTypeName((*it)->parameterType) + (*it)->name->value);
                StoreInst *inst = new StoreInst(argumentValue, cgcontext.locals()[(*it)->name->value], false, bb);

            } else {
                auto var = new AllocaInst(getTypeFromExprNode(cgcontext, (*it)->type), 0, nullptr, (*it)->name->value, bb);
                cgcontext.locals()[(*it)->name->value] = var;
                cgcontext.localsExprs()[(*it)->name->value] = (*it);
                Value *argumentValue = &(*argsValues);
                argumentValue->setName(getParameterTypeName((*it)->parameterType) + (*it)->name->value);
                StoreInst *inst = new StoreInst(argumentValue, cgcontext.locals()[(*it)->name->value], false, bb);
            }
            DILocalVariable *D = cgcontext.debugBuilder->createParameterVariable(dbgFunc, (*it)->name->value, i, unit, loc.line, getDbgTypeFromExprNode(cgcontext, (*it)->type, (*it)->parameterType), true);
            cgcontext.debugBuilder->insertDeclare(cgcontext.locals()[(*it)->name->value], D, cgcontext.debugBuilder->createExpression(),
                                                  DILocation::get(dbgFunc->getContext(), loc.line, 0, dbgFunc),
                                                  cgcontext.builder->GetInsertBlock());
            i++;
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
        cgcontext.dbgInfo.lexicalBlocks.pop_back();
    }

    // function declarations can't have debug info, so...
    if (!function->isDeclaration())
        function->setSubprogram(dbgFunc);

    //cgcontext.builder->SetInsertPoint(cgcontext.currentBlock());
    return nullptr;
}

llvm::Value *FieldVarDecNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    auto elementPtr = cgcontext.builder->CreateStructGEP(cgcontext.allocatedClasses[cgcontext.moduleName + "." + typeName], cgcontext.currentTypeLoad, index);
    if (cgcontext.allocatedClasses.contains(type)) {
        llvm::Type* int64type = llvm::Type::getInt32Ty(*cgcontext.context);
        auto structType = typeOf(cgcontext, type);
        auto malloc = CallInst::CreateMalloc(cgcontext.currentBlock(), int64type, structType, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*cgcontext.context), cgcontext.mModule->getDataLayout().getTypeAllocSize(structType)), nullptr, cgcontext.mModule->getFunction("malloc"), "");
        cgcontext.builder->Insert(malloc);
        auto newVar = cgcontext.builder->CreateStore(malloc, elementPtr);
        auto constructorFunc = cgcontext.mModule->getFunction(type + "._DefaultConstructor_");
        if (constructorFunc != nullptr)
        {
            auto p = cgcontext.builder->CreateLoad(elementPtr);
            if (DEBUG) elementPtr->print(llvm::errs());
            cgcontext.builder->CreateCall(constructorFunc, {p});
        }

        return newVar;
    }
    auto assignData = expr->codegen(cgcontext);
    return cgcontext.builder->CreateStore(assignData, elementPtr);
}

llvm::Value *FieldArrayVarDecNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    auto elementPtr = cgcontext.builder->CreateStructGEP(cgcontext.allocatedClasses[cgcontext.moduleName + "." + typeName], cgcontext.currentTypeLoad, index);
    auto arrDec = this->var;
    auto exprNode = dynamic_cast<ArrayExprNode*>(arrDec->expr);
    auto arrExpr = exprNode;

    auto size = exprNode->size->codegen(cgcontext);
    auto arraySize = size;
    auto sizes = getSizesOfArray(this->var);
    if (arrExpr->type == "array")
    {
        for (auto &slice : *arrExpr->values)
        {
            auto castedSlice = dynamic_cast<ArrayExprNode*>(slice);
            arrExpr = castedSlice;
        }
    }
    auto type = typeOf(cgcontext, arrExpr->type);
    auto finalType = type;
    for (int i = 1; i < var->indicesCount; i++) {
        finalType = finalType->getPointerTo();
    }

    llvm::Type* int32type = llvm::Type::getInt32Ty(*cgcontext.context);

    cgcontext.locals()[name->value] = elementPtr;
    cgcontext.localsExprs()[name->value] = this;

    auto elementSize = ConstantInt::get(int32type, cgcontext.dataLayout->getTypeAllocSize(finalType));
    auto allocSize = BinaryOperator::Create(Instruction::Mul, elementSize, arraySize, "", cgcontext.currentBlock());
    // GC_malloc
    auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int32type, finalType, allocSize, nullptr, cgcontext.mModule->getFunction("malloc"), "");
    // malloc
    //auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int32type, type, allocSize, nullptr, nullptr, "");
    cgcontext.builder->Insert(arr);
    cgcontext.builder->CreateStore(arr, elementPtr);

    auto currentArr = arr;
    auto currentType = finalType;

    if (var->indicesCount > 1)
    {
        std::vector<llvm::Value*> jvars;
        jvars.reserve(var->indicesCount);
        for (int i = 0; i < var->indicesCount; i++) {
            jvars.push_back(cgcontext.builder->CreateAlloca(int32type, 0, nullptr, "j" + std::to_string(i)));
            cgcontext.builder->CreateStore(ConstantInt::get(int32type, 0), jvars[i]);
        }
        int i = 1;

        generateMallocLoopsRecursive(cgcontext, i, var->indicesCount, currentType, sizes, jvars, currentArr, elementPtr);
    }
    return elementPtr;
}

llvm::Value *MethodDecNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    auto funcDec = new FuncDecStatementNode(loc, type, name, isPrivate, isFunction, args, block);
    auto ret = funcDec->codegen(cgcontext);
    return ret;
}

llvm::Value *TypeDecStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    if (name->value == "Object") return nullptr;
    if (cgcontext.allocatedClasses[name->value] != nullptr) return nullptr;
    cgcontext.allocatedClasses[name->value] = StructType::create(*cgcontext.context, name->value);
    cgcontext.allocatedClasses[name->value]->setName(name->value);
    std::vector<Type*> dataTypes;
    std::vector<DIType*> dbgDataTypes;
    for (auto field : *fields)
    {
        Type* type;
        DIType *dbgType;
        if (dynamic_cast<FieldArrayVarDecNode*>(field))
        {
            auto exprNode = dynamic_cast<FieldArrayVarDecNode*>(field)->var;
            auto arrExpr = dynamic_cast<ArrayExprNode*>(exprNode->expr);
            if (arrExpr->type == "array")
            {
                for (auto &slice : *arrExpr->values)
                {
                    auto castedSlice = dynamic_cast<ArrayExprNode*>(slice);
                    arrExpr = castedSlice;
                }
            }
            type = ptrToTypeOf(cgcontext, arrExpr->type);
            dbgType = dbgPrToTypeOf(cgcontext, arrExpr->type);
            for (int i = 1; i < exprNode->indicesCount; i++) {
                type = type->getPointerTo();
                dbgType = cgcontext.dbgInfo.createPointerType(dbgType, "array");
            }
        }
        else
        {
            auto typeNode = dynamic_cast<FieldVarDecNode*>(field)->type;
            if (cgcontext.allocatedClasses.contains(typeNode)) {
                type = ptrToTypeOf(cgcontext, dynamic_cast<FieldVarDecNode*>(field)->type);
                dbgType = dbgPrToTypeOf(cgcontext, dynamic_cast<FieldVarDecNode*>(field)->type);
            }
            else {
                type = typeOf(cgcontext, dynamic_cast<FieldVarDecNode*>(field)->type);
                dbgType = dbgTypeOf(cgcontext, dynamic_cast<FieldVarDecNode*>(field)->type);
            }
        }
        dataTypes.push_back(type);
        dbgDataTypes.push_back(dbgType);
        // ... array, class, string
    }
    cgcontext.allocatedClasses[name->value]->setBody(dataTypes);

    DIFile* unit = cgcontext.debugBuilder->createFile(cgcontext.dbgInfo.compileUnit->getFilename(), cgcontext.dbgInfo.compileUnit->getDirectory());
    // converting vector of types to type that createClassType function requires to be "elements"
    std::vector<DIDerivedType*> membersDbg;
    int offset = 0;
    for (int i = 0; i < fields->size(); i++) {
        auto memberDbg = cgcontext.debugBuilder->createMemberType(cgcontext.dbgInfo.compileUnit, fields->at(i)->name->value, unit, fields->at(i)->loc.line, cgcontext.dataLayout->getTypeAllocSize(dataTypes[i]) * 8, 0, offset, DINode::DIFlags::FlagPublic, dbgDataTypes[i]);
        membersDbg.push_back(memberDbg);
        offset += cgcontext.dataLayout->getTypeAllocSize(dataTypes[i]) * 8;
    }
    ArrayRef<Metadata*> elements(reinterpret_cast<Metadata *const *>(membersDbg.data()), membersDbg.size());
    auto dbgDataTypesArray = cgcontext.debugBuilder->getOrCreateArray(elements);
    auto dbgClass = cgcontext.debugBuilder->createClassType(cgcontext.dbgInfo.compileUnit, name->value, unit, loc.line, offset, offset, 0, DIType::FlagZero, nullptr, dbgDataTypesArray);
    cgcontext.dbgInfo.dbgClasses[name->value] = dbgClass;

    // create default constructor
    if (!fields->empty())
    {
        for (auto &method : *methods)
        {
            if (method->name->value == name->value + "._DefaultConstructor_")
            {
                if (method != nullptr)
                {
                    std::vector<llvm::Type*> argTypes;
                    auto thisArg = ptrToTypeOf(cgcontext, name->value);
                    argTypes.push_back(thisArg);
                    auto dbgThisArg = dbgPrToTypeOf(cgcontext, name->value);
                    FunctionType* type = FunctionType::get(Type::getVoidTy(*cgcontext.context), argTypes, false);
                    if (method->block != nullptr)
                    {
                        Function *constructorFunc = Function::Create(type, Function::ExternalLinkage, Twine(name->value + "._DefaultConstructor_"), cgcontext.mModule);
                        llvm::DISubprogram *dbgFunc = cgcontext.debugBuilder->createFunction(
                                cgcontext.dbgInfo.compileUnit, name->value + "._DefaultConstructor_", name->value + "._DefaultConstructor_", unit, loc.line,
                                cgcontext.dbgInfo.CreateFunctionType({dbgThisArg}), loc.line,
                                llvm::DISubprogram::FlagPrivate,
                                llvm::DISubprogram::SPFlagDefinition);

                        cgcontext.dbgInfo.lexicalBlocks.push_back(dbgFunc);
                        cgcontext.dbgInfo.emitLocation(cgcontext.builder.get());
                        BasicBlock *bb = BasicBlock::Create(*cgcontext.context, name->value + "_DefaultConstructor_Entry", constructorFunc, nullptr);
                        cgcontext.pushBlock(bb);
                        cgcontext.builder->SetInsertPoint(bb);

                        auto ptrToType = ptrToTypeOf(cgcontext, name->value);
                        auto typeAlloc = cgcontext.builder->CreateAlloca(ptrToType);
                        auto storeThis = cgcontext.builder->CreateStore(constructorFunc->getArg(0), typeAlloc);
                        auto loadThis = cgcontext.builder->CreateLoad(ptrToType, typeAlloc);
                        cgcontext.currentTypeLoad = loadThis;
                        for (auto &statement : *method->block->statements)
                        {
                            statement->codegen(cgcontext);
                        }
                        cgcontext.builder->CreateRetVoid();
                        cgcontext.popBlock();
                        cgcontext.dbgInfo.lexicalBlocks.pop_back();
                        constructorFunc->setSubprogram(dbgFunc);
                    }
                    else
                    {
                        Function *constructorFunc = Function::Create(type, Function::ExternalLinkage, Twine(name->value + "._DefaultConstructor_"), cgcontext.mModule);
                    }
                }
            }
        }
    }
    // codegen other methods
    for (auto &method : *methods)
    {
        if (method->name->value != name->value + "._DefaultConstructor_")
        {
            method->codegen(cgcontext);
        }
    }
    return nullptr;
}

llvm::Value *ExternFuncDecStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    std::vector<llvm::Type*> argTypes;
    std::vector<int> refParams;
    int i = 1;
    //std::string nameAddition;
    //llvm::raw_string_ostream nameAdditionStream(nameAddition);
    for (auto arg : *args)
    {
        Type* argType;
        if (arg->parameterType == ParameterType::Out || arg->parameterType == ParameterType::Var)
        {
            // not tested
            argType = getTypeFromExprNode(cgcontext, arg->type)->getPointerTo();
            argTypes.push_back(argType);
            refParams.push_back(i);
        }
        else
        {
            argType = getTypeFromExprNode(cgcontext, arg->type);
            argTypes.push_back(argType);
        }
        //argType->print(nameAdditionStream);
        i++;
    }
    Type *retType = getTypeFromExprNode(cgcontext, type);
    if (retType->isStructTy()) {
        retType = retType->getPointerTo();
    }
    std::string retTypeName;

    FunctionType* funcType = FunctionType::get(retType, argTypes, false);
    auto function = Function::Create(funcType, GlobalValue::ExternalLinkage, name->value, cgcontext.mModule);
    function->setCallingConv(CallingConv::C);
    return function;
}

llvm::Value *ElseIfStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
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

    return ifEnd;
}

llvm::Value *IfStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    Function *TheFunction = cgcontext.builder->GetInsertBlock()->getParent();
    BasicBlock* ifTrue = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
    BasicBlock* ifFalse = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
    BasicBlock *ifEnd = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
    BranchInst::Create(ifTrue, ifFalse, condExpr->codegen(cgcontext), cgcontext.currentBlock());
    // Entering IF
    cgcontext.pushBlock(ifTrue);
    cgcontext.builder->SetInsertPoint(ifTrue);
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), trueBlock);
    bool isRetTrue = false;
    DIFile* unit = cgcontext.debugBuilder->createFile(cgcontext.dbgInfo.compileUnit->getFilename(), cgcontext.dbgInfo.compileUnit->getDirectory());
    if (trueBlock != nullptr)
    {
        // create lexical block for true block
        auto dbgLexicalBlock = cgcontext.debugBuilder->createLexicalBlock(cgcontext.dbgInfo.lexicalBlocks.back(), unit, trueBlock->loc.line, trueBlock->loc.col);
        cgcontext.dbgInfo.lexicalBlocks.push_back(dbgLexicalBlock);
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
        cgcontext.dbgInfo.lexicalBlocks.pop_back();
    }

    // JMP to END
    if (!isRetTrue) BranchInst::Create(ifEnd, cgcontext.currentBlock());
    cgcontext.popBlock();

    // Entering ELSE
    cgcontext.pushBlock(ifFalse);
    cgcontext.builder->SetInsertPoint(ifFalse);

    for (auto &elseif : *elseifNodes) {
        BasicBlock *elseIfTrue = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
        BasicBlock *elseIfFalse = BasicBlock::Create(*cgcontext.context, "", TheFunction, 0);
        BranchInst::Create(elseIfTrue, elseIfFalse, elseif->condExpr->codegen(cgcontext), cgcontext.currentBlock());
        cgcontext.pushBlock(elseIfTrue);
        cgcontext.builder->SetInsertPoint(elseIfTrue);

        cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), elseif);
        bool isRetElseIfTrue = false;
        if (elseif->trueBlock != nullptr)
        {
            auto dbgLexicalBlock = cgcontext.debugBuilder->createLexicalBlock(cgcontext.dbgInfo.lexicalBlocks.back(), unit, elseif->loc.line, elseif->loc.col);
            cgcontext.dbgInfo.lexicalBlocks.push_back(dbgLexicalBlock);
            for (auto statement : *elseif->trueBlock->statements)
            {
                //if (isRetElseIfTrue) llvm::errs() << "[WARN] Code after return statement in block is unreachable.\n";
                statement->codegen(cgcontext);
                if (dynamic_cast<ReturnStatementNode*>(statement) != nullptr)
                {
                    isRetElseIfTrue = true;
                    break;
                }
            }
            cgcontext.dbgInfo.lexicalBlocks.pop_back();
        }

        // JMP to END
        if (!isRetElseIfTrue) BranchInst::Create(ifEnd, cgcontext.currentBlock());

        cgcontext.popBlock();
        cgcontext.pushBlock(elseIfFalse);
        cgcontext.builder->SetInsertPoint(elseIfFalse);
        //cgcontext.popBlock();
    }
    bool isRetFalse = false;
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), falseBlock);
    if (falseBlock != nullptr)
    {
        auto dbgLexicalBlock = cgcontext.debugBuilder->createLexicalBlock(cgcontext.dbgInfo.lexicalBlocks.back(), unit, falseBlock->loc.line, falseBlock->loc.col);
        cgcontext.dbgInfo.lexicalBlocks.push_back(dbgLexicalBlock);
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
        cgcontext.dbgInfo.lexicalBlocks.pop_back();
    }
    // JMP to END
    if (!isRetFalse) BranchInst::Create(ifEnd, cgcontext.currentBlock());

    // TODO: fixes "Use still stuck around after Def is destroyed" error, probably that's wrong way to do it, but IR looks ok
    for (auto &elseif : *elseifNodes) {
        cgcontext.popBlock();
    }
    cgcontext.popBlock();

    // Return END
    cgcontext.ret(ifEnd);
    cgcontext.builder->SetInsertPoint(ifEnd);

    return ifEnd;
}

llvm::Value *ForStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *WhileStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
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
    DIFile* unit = cgcontext.debugBuilder->createFile(cgcontext.dbgInfo.compileUnit->getFilename(), cgcontext.dbgInfo.compileUnit->getDirectory());
    bool isRet = false;
    if (block != nullptr)
    {
        auto dbgLexicalBlock = cgcontext.debugBuilder->createLexicalBlock(cgcontext.dbgInfo.lexicalBlocks.back(), unit, block->loc.line, block->loc.col);
        cgcontext.dbgInfo.lexicalBlocks.push_back(dbgLexicalBlock);
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
        cgcontext.dbgInfo.lexicalBlocks.pop_back();
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

llvm::Value *FuncPointerStatementNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), this);
    std::vector<llvm::Type*> argTypes;
    std::vector<int> refParams;
    llvm::Value* rightVal = nullptr;
    int i = 1;
    for (auto arg : *args)
    {
        Type* argType;
        if (arg->parameterType == ParameterType::Out || arg->parameterType == ParameterType::Var)
        {
            // not tested
            argType = getTypeFromExprNode(cgcontext, arg->type)->getPointerTo();
            argTypes.push_back(argType);
            refParams.push_back(i);
        }
        else
        {
            argType = getTypeFromExprNode(cgcontext, arg->type);
            argTypes.push_back(argType);
        }
        i++;
    }
    Type *retType = getTypeFromExprNode(cgcontext, type);
    if (retType->isStructTy()) {
        retType = retType->getPointerTo();
    }
    std::string retTypeName;

    FunctionType* funcType = FunctionType::get(retType, argTypes, false);
    llvm::Value* var = nullptr;
    if (!isGlobal) {
        var = cgcontext.builder->CreateAlloca(funcType->getPointerTo(), nullptr);
        cgcontext.locals()[name->value] = var;
        cgcontext.localsExprs()[name->value] = this;
    }
    else {
        cgcontext.mModule->getOrInsertGlobal(name->value, funcType->getPointerTo());
        auto gVar = cgcontext.mModule->getNamedGlobal(name->value);
        if (isExtern) gVar->setLinkage(llvm::GlobalValue::ExternalLinkage);
        else gVar->setLinkage(GlobalValue::InternalLinkage);
        if (!isExtern) gVar->setInitializer(ConstantPointerNull::get(funcType->getPointerTo()));
        gVar->setAlignment(Align(8));
        cgcontext.globals()[name->value] = gVar;
        var = gVar;
    }
    if (expr != nullptr) {
        std::string nameAddition;
        llvm::raw_string_ostream nameAdditionStream(nameAddition);
        if (var->getType()->isPointerTy()) {
            if (var->getType()->getPointerElementType()->isPointerTy()) {
                if (var->getType()->getPointerElementType()->getPointerElementType()->isFunctionTy()) {
                    auto func = var->getType()->getPointerElementType()->getPointerElementType();
                    auto n = func->getFunctionNumParams();
                    for (int i = 0; i < n; i++){
                        func->getFunctionParamType(i)->print(nameAdditionStream);
                    }
                    cgcontext.isFuncPointerAssignment = true;
                    cgcontext.currentNameAddition = nameAddition;
                    rightVal = expr->codegen(cgcontext);
                    if (rightVal == nullptr) {
                        llvm::errs() << "[ERROR] Codegen - no such function found when assigning to \"" << name->value << "\".\n";
                    }
                    cgcontext.isFuncPointerAssignment = false;
                    cgcontext.currentNameAddition = "";
                }
            }
        }
        if (rightVal->getType() != var->getType()->getPointerElementType()) {
            rightVal = mycast(rightVal, var->getType()->getPointerElementType(), cgcontext);
        }
        if (isGlobal) {
            cgcontext.mModule->getNamedGlobal(name->value)->setInitializer(static_cast<Constant *>(rightVal));
        }
        else {
            cgcontext.builder->CreateStore(rightVal, var);
        }
    }
    //auto t = cgcontext.mModule->getFunction("GC_init")->getType();
    return var;
}

llvm::Value *ModuleStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *ImportStatementNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *IndexesExprNode::codegen(CodeGenContext &cgcontext) {
    cgcontext.dbgInfo.emitLocation(cgcontext.builder.get(), (ExprNode*)this);
    Value* var;
    Type *type = nullptr;
    // check local variables
    var = cgcontext.localsLookup(value);
    auto varExpr = cgcontext.localsExprsLookup(value);
    // check global variables if there is no local
    if (var == nullptr)
    {
        // check global variable declared in current module
        if (cgcontext.globals().find(cgcontext.moduleName + "." + value) == cgcontext.globals().end())
        {
            // check other global variables
            if (cgcontext.globals().find(value) == cgcontext.globals().end())
            {
                // check type fields
                auto typeVarName = value.substr(0, value.rfind('.'));
                auto typeVar = cgcontext.localsLookup(typeVarName);
                if (typeVar != nullptr)
                {
                    Type* typeOfTypeVar = nullptr;
                    // type variable exists, get type
                    // we should check is it pointer to pointer to struct or just pointer to struct
                    if (dotClass)
                    {
                        if (typeVar->getType()->getPointerElementType()->isStructTy()) {
                            typeOfTypeVar = typeVar->getType()->getPointerElementType();
                            auto elementPtr = cgcontext.builder->CreateStructGEP(typeOfTypeVar, typeVar, index);
                            var = elementPtr;
                        }
                        else if (typeVar->getType()->getPointerElementType()->getPointerElementType()->isStructTy()) {
                            typeOfTypeVar = typeVar->getType()->getPointerElementType()->getPointerElementType();
                            auto loadThis = cgcontext.builder->CreateLoad(typeVar);
                            cgcontext.currentTypeLoad = loadThis;
                            auto elementPtr = cgcontext.builder->CreateStructGEP(typeOfTypeVar, loadThis, index);
                            var = elementPtr;
                        }
                    }
                    else llvm::errs() << "[ERROR] Variable \"" << typeVarName << "\" is not a custom type.\n";
                }
                else llvm::errs() << "[ERROR] Undeclared Variable \"" << value << "\".\n";
            }
            else
                var = cgcontext.globals()[value];
        }
        else
        {
            var = cgcontext.globals()[cgcontext.moduleName + "." + value];
        }
    }

    auto loadArr = cgcontext.builder->CreateLoad(var);
    //varExpr = cgcontext.localsExprs()[value];

    // move it to parser
    /*auto arrExpr = dynamic_cast<ArrayDecStatementNode*>(varExpr);
    if (arrExpr == nullptr) {
        auto field = dynamic_cast<FieldArrayVarDecNode*>(varExpr);
        if (field != nullptr) arrExpr = field->var;
    }
    if (arrExpr == nullptr) {
        llvm::errs() << "[ERROR] Variable \"" << varExpr->name->value << "\" is not an array.\n";
    }
    if (arrExpr->indicesCount > indexes->size()) {
        llvm::errs() << "[ERROR] Too much indexes in expression. (\"" << varExpr->name->value << "\")n";
    }*/

    llvm::Value *elementPtr = nullptr;
    for (int i = 0; i < indexes->size(); i++) {
        auto index = indexes->at(i)->codegen(cgcontext);
        elementPtr = cgcontext.builder->CreateInBoundsGEP(loadArr, index);
        loadArr = cgcontext.builder->CreateLoad(elementPtr);
    }
    if (isPointer) return elementPtr;
    return cgcontext.builder->CreateLoad(elementPtr, false);
}
