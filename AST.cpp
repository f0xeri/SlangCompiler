//
// Created by f0xeri on 23.01.2022.
//

#include "AST.hpp"

using namespace llvm;


static Type *typeOf(CodeGenContext &cgcontext, const std::string &var) {
    Type *type = nullptr;
    if (var == "integer")
        type = Type::getInt64Ty(*cgcontext.context);
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
        type = Type::getInt64PtrTy(*cgcontext.context);
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

// array size is not calculating here!!!
Type* getTypeFromExprNode(CodeGenContext &cgcontext, ExprNode* node)
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
        if (arrExpr->type == "array")
        {
            for (auto &slice : *arrExpr->values)
            {
                auto castedSlice = dynamic_cast<ArrayExprNode*>(slice);
                //auto sliceSize = castedSlice->size->codegen(cgcontext);
                //auto newArraySize = BinaryOperator::Create(Instruction::Mul, sliceSize, arraySize, "", cgcontext.currentBlock());
                //arraySize = newArraySize;
                arrExpr = castedSlice;
            }
        }
        retType = ptrToTypeOf(cgcontext, arrExpr->type);
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
    return retType;
}

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

llvm::Value *FloatExprNode::codegen(CodeGenContext &cgcontext) {
    return ConstantFP::get(Type::getFloatTy(*cgcontext.context), value);
}

llvm::Value *CharExprNode::codegen(CodeGenContext &cgcontext) {
    return ConstantInt::get(Type::getInt8Ty(*cgcontext.context), value, true);
}

llvm::Value *StringExprNode::codegen(CodeGenContext &cgcontext) {
    return cgcontext.builder->CreateGlobalStringPtr(value);
}

llvm::Value *NilExprNode::codegen(CodeGenContext &cgcontext) {
    Type *retType = getTypeFromExprNode(cgcontext, type);
    auto nil = ConstantPointerNull::get(retType->getPointerTo());
    return nil;
}

llvm::Value *ArrayExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *FuncExprNode::codegen(CodeGenContext &cgcontext) {
    return nullptr;
}

llvm::Value *BooleanExprNode::codegen(CodeGenContext &cgcontext) {
    return ConstantInt::get(Type::getInt1Ty(*cgcontext.context), value, false);
}

llvm::Value *VariableExprNode::codegen(CodeGenContext &cgcontext) {
    Value *val = nullptr;
    Type *type = nullptr;
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
                    llvm::errs() << "[ERROR] Codegen - no such function \"" << value << "\".\n";
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
    if (val == nullptr) return nullptr;
    if (type == nullptr)
        type = val->getType()->getPointerElementType();
    if (val->getType()->getPointerElementType()->isStructTy()) return val;
    return new LoadInst(type, val, "", false, cgcontext.currentBlock());
}

llvm::Value *UnaryOperatorExprNode::codegen(CodeGenContext &cgcontext) {
    auto val = right->codegen(cgcontext);
    if (val->getType()->isIntegerTy())
        return BinaryOperator::CreateNeg(right->codegen(cgcontext), "", cgcontext.currentBlock());
    else if (val->getType()->isFloatingPointTy())
        return BinaryOperator::CreateFSub(ConstantFP::get(val->getType(), 0), val, "", cgcontext.currentBlock());
    return nullptr;
}

llvm::Value *OperatorExprNode::codegen(CodeGenContext &cgcontext) {
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
    std::vector<Value*> argsRef;
    std::string nameAddition;
    llvm::raw_string_ostream nameAdditionStream(nameAddition);
    auto foundFunc = cgcontext.lookupFuncs(name->value);
    auto funcDecl = dynamic_cast<FuncDecStatementNode*>(foundFunc);
    auto externFuncDecl = dynamic_cast<ExternFuncDecStatementNode*>(foundFunc);
    auto methodDecl = dynamic_cast<MethodDecNode*>(foundFunc);
    int i = 0;
    for (auto it = args->begin(); it != args->end(); it++, i++)
    {
        // TODO: CreateLoad if args is out/var (it should fix this.method() calls too
        auto var = (*it)->codegen(cgcontext);
        auto loadValue = var;
        if (dynamic_cast<NilExprNode*>(*it) == nullptr && !var->getType()->isPointerTy())
        {
            if (funcDecl != nullptr) {
                if (i < funcDecl->args->size()) {
                    if (funcDecl->args->at(i)->parameterType == ParameterType::Out || funcDecl->args->at(i)->parameterType == ParameterType::Var) {
                        loadValue = getPointerOperand(loadValue);
                    }
                }
            }
            if (externFuncDecl != nullptr) {
                if (i < externFuncDecl->args->size()) {
                    if (externFuncDecl->args->at(i)->parameterType == ParameterType::Out || externFuncDecl->args->at(i)->parameterType == ParameterType::Var) {
                        loadValue = getPointerOperand(loadValue);

                    }
                }
            }
            if (methodDecl != nullptr) {
                if (i < methodDecl->args->size()) {
                    if (methodDecl->args->at(i)->parameterType == ParameterType::Out || methodDecl->args->at(i)->parameterType == ParameterType::Var) {
                        loadValue = getPointerOperand(loadValue);
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

llvm::Value *AssignExprNode::codegen(CodeGenContext &cgcontext) {
    auto assignData = right->codegen(cgcontext);
    Value* var;
    Type *type = nullptr;
    // check local variables
    var = cgcontext.localsLookup(left->value);
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
                cgcontext.isFuncPointerAssignment = false;
                cgcontext.currentNameAddition = "";
            }
        }
    }
    return new StoreInst(assignData, var, false, cgcontext.currentBlock());
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
        if (cgcontext.allocatedClasses.contains(type)) {
            newVar = cgcontext.builder->CreateAlloca(ptrToTypeOf(cgcontext, type), 0, nullptr, name->value);
            llvm::Type* int64type = llvm::Type::getInt64Ty(*cgcontext.context);
            auto structType = typeOf(cgcontext, type);
            auto malloc = CallInst::CreateMalloc(cgcontext.currentBlock(), int64type, structType, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*cgcontext.context), cgcontext.mModule->getDataLayout().getTypeAllocSize(structType)), nullptr, cgcontext.mModule->getFunction("malloc"), "");
            cgcontext.builder->Insert(malloc);
            cgcontext.builder->CreateStore(malloc, newVar);
        }
        else {
            newVar = cgcontext.builder->CreateAlloca(typeOf(cgcontext, type), 0, nullptr, name->value);
        }
        cgcontext.locals()[name->value] = newVar;
        if (expr != NULL) {
            rightVal = expr->codegen(cgcontext);
            //new StoreInst(rightVal, newVar, false, cgcontext.currentBlock());
            cgcontext.builder->CreateStore(rightVal, newVar);
        }
        if (cgcontext.allocatedClasses.contains(type))
        {
            auto constructorFunc = cgcontext.mModule->getFunction(type + "._DefaultConstructor_");
            if (constructorFunc != nullptr)
            {
                auto p = cgcontext.builder->CreateLoad(newVar);
                newVar->print(llvm::errs());
                cgcontext.builder->CreateCall(constructorFunc, {p});
            }
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
    auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int64type, type, allocSize, nullptr, cgcontext.mModule->getFunction("malloc"), "");
    // malloc
    //auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int64type, type, allocSize, nullptr, nullptr, "");
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
    if (retType->isStructTy()) {
        retType = retType->getPointerTo();
    }
    std::string retTypeName;

    FunctionType* funcType = FunctionType::get(retType, argTypes, false);
    auto function = Function::Create(funcType, GlobalValue::ExternalLinkage, name->value + nameAddition, cgcontext.mModule);
    for (auto paramID : refParams)
    {
        // TODO Set Dereferenceable attribute for ref params
        //function->addAttribute(paramID, Attribute::Dereferenceable);
    }
    if (block != nullptr) {
        BasicBlock *bb = BasicBlock::Create(*cgcontext.context, cgcontext.moduleName + retTypeName + name->value + "Entry",
                                            function, 0);
        cgcontext.pushBlock(bb);
        cgcontext.builder->SetInsertPoint(bb);
        Function::arg_iterator argsValues = function->arg_begin();
        for (auto it = args->begin(); it != args->end(); it++, argsValues++) {
            if ((*it)->parameterType == ParameterType::Out || (*it)->parameterType == ParameterType::Var) {
                auto var = new AllocaInst(getTypeFromExprNode(cgcontext, (*it)->type)->getPointerTo(), 0, nullptr, (*it)->name->value, bb);
                cgcontext.locals()[(*it)->name->value] = var;
                Value *argumentValue = &(*argsValues);
                argumentValue->setName(getParameterTypeName((*it)->parameterType) + (*it)->name->value);
                StoreInst *inst = new StoreInst(argumentValue, cgcontext.locals()[(*it)->name->value], false, bb);
            } else {
                auto var = new AllocaInst(getTypeFromExprNode(cgcontext, (*it)->type), 0, nullptr, (*it)->name->value, bb);
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
    auto elementPtr = cgcontext.builder->CreateStructGEP(cgcontext.allocatedClasses[cgcontext.moduleName + "." + typeName], cgcontext.currentTypeLoad, index);
    auto assignData = expr->codegen(cgcontext);
    return cgcontext.builder->CreateStore(assignData, elementPtr);
}

llvm::Value *FieldArrayVarDecNode::codegen(CodeGenContext &cgcontext) {
    auto elementPtr = cgcontext.builder->CreateStructGEP(cgcontext.allocatedClasses[cgcontext.moduleName + "." + typeName], cgcontext.currentTypeLoad, index);

    auto arrDec = this->var;
    auto exprNode = dynamic_cast<ArrayExprNode*>(arrDec->expr);
    auto arrExpr = exprNode;

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

    cgcontext.locals()[name->value] = elementPtr;

    auto elementSize = ConstantInt::get(int64type, cgcontext.dataLayout->getTypeAllocSize(type));
    auto allocSize = BinaryOperator::Create(Instruction::Mul, elementSize, arraySize, "", cgcontext.currentBlock());
    // GC_malloc
    auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int64type, type, allocSize, nullptr, cgcontext.mModule->getFunction("malloc"), "");
    // malloc
    //auto arr = CallInst::CreateMalloc(cgcontext.currentBlock(), int64type, type, allocSize, nullptr, nullptr, "");
    cgcontext.builder->Insert(arr);

    cgcontext.builder->CreateStore(arr, elementPtr);
    return elementPtr;
}

llvm::Value *MethodDecNode::codegen(CodeGenContext &cgcontext) {
    auto funcDec = new FuncDecStatementNode(type, name, isPrivate, isFunction, args, block);
    auto ret = funcDec->codegen(cgcontext);
    return ret;
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
            type = typeOf(cgcontext, dynamic_cast<FieldVarDecNode*>(field)->type);
        dataTypes.push_back(type);
        // ... array, class, string
    }
    cgcontext.allocatedClasses[name->value]->setBody(dataTypes);

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
                    FunctionType* type = FunctionType::get(Type::getVoidTy(*cgcontext.context), argTypes, false);
                    if (method->block != nullptr)
                    {
                        Function *constructorFunc = Function::Create(type, Function::ExternalLinkage, Twine(name->value + "._DefaultConstructor_"), cgcontext.mModule);
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

llvm::Value *FuncPointerStatementNode::codegen(CodeGenContext &cgcontext) {
    std::vector<llvm::Type*> argTypes;
    std::vector<int> refParams;
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
    }
    else {
        cgcontext.mModule->getOrInsertGlobal(name->value, funcType->getPointerTo());
        auto gVar = cgcontext.mModule->getNamedGlobal(name->value);
        gVar->setLinkage(GlobalValue::ExternalLinkage);
        gVar->setInitializer(ConstantPointerNull::get(funcType->getPointerTo()));
        gVar->setAlignment(Align(8));
        cgcontext.globals()[name->value] = gVar;
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
    return nullptr;
}