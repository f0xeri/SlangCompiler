//
// Created by f0xeri on 23.01.2022.
//

#include "AST.hpp"

using namespace llvm;

static Value* mycast(Value* value, Type* type, CodeGenContext& context) {
    if (type == value->getType())
        return value;
    if (type == Type::getDoubleTy(getGlobalContext())) {
        if (value->getType() == Type::getInt64Ty(getGlobalContext()) || value->getType() == Type::getInt8Ty(getGlobalContext()))
            value = new SIToFPInst(value, type, "", context.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    }
    else if (type == Type::getInt64Ty(getGlobalContext())) {
        if (value->getType() == Type::getDoubleTy(getGlobalContext()))
            value = new FPToSIInst(value, type, "", context.currentBlock());
        else if (value->getType() == Type::getInt8Ty(getGlobalContext()))
            value = new SExtInst(value, type, "", context.currentBlock());
        else if (value->getType() == Type::getInt32Ty(getGlobalContext()))
            value = new ZExtInst(value, type, "", context.currentBlock());
        else if (value->getType() == Type::getInt8PtrTy(getGlobalContext()))
            value = new PtrToIntInst(value, type, "", context.currentBlock());
        else if (value->getType() == Type::getInt64PtrTy(getGlobalContext()))
            value = new PtrToIntInst(value, type, "", context.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    } else if (type == Type::getInt8Ty(getGlobalContext())) {
        if (value->getType() == Type::getDoubleTy(getGlobalContext()))
            value = new FPToSIInst(value, type, "", context.currentBlock());
        else if (value->getType() == Type::getInt64Ty(getGlobalContext()))
            value = new TruncInst(value, type, "", context.currentBlock());
        else
            llvm::errs() << "[ERROR] Cannot cast this value.\n";
    } else
        llvm::errs() << "[ERROR] Cannot cast this value.\n";
    return value;
}

llvm::Value *IntExprNode::codegen(CodeGenContext &cgconext) {
    return ConstantInt::get(Type::getInt64Ty(getGlobalContext()), value, true);
}

llvm::Value *RealExprNode::codegen(CodeGenContext &cgconext) {
    return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

llvm::Value *CharExprNode::codegen(CodeGenContext &cgconext) {
    return ConstantInt::get(Type::getInt8Ty(getGlobalContext()), value, true);
}

llvm::Value *StringExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *ArrayExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *BooleanExprNode::codegen(CodeGenContext &cgconext) {
    return ConstantInt::get(Type::getInt1Ty(getGlobalContext()), value, false);
}

llvm::Value *VariableExprNode::codegen(CodeGenContext &cgconext) {
    Value* val;
    Type *type = nullptr;
    val = cgconext.localsLookup(value);
    if (val == nullptr)
    {
        if(cgconext.globals().find(value) == cgconext.globals().end())
        {
            llvm::errs() << "[ERROR] Undeclared Variable \"" << value << "\".\n";
            return nullptr;
        }
        else
        {
            val = cgconext.globals()[value];
        }
    }
    type = val->getType()->getPointerElementType();
    return new LoadInst(type, val, "", false, cgconext.currentBlock());
}

llvm::Value *UnaryOperatorExprNode::codegen(CodeGenContext &cgconext) {
    return BinaryOperator::CreateNeg(right->codegen(cgconext), "", cgconext.currentBlock());
}

llvm::Value *OperatorExprNode::codegen(CodeGenContext &cgconext) {
    Value* leftVal = left->codegen(cgconext);
    Value* rightVal = right->codegen(cgconext);
    bool floatOp = false;
    if (leftVal->getType()->isDoubleTy() || rightVal->getType()->isDoubleTy()) {
        leftVal = mycast(leftVal, Type::getDoubleTy(getGlobalContext()), cgconext);
        rightVal = mycast(rightVal, Type::getDoubleTy(getGlobalContext()), cgconext);
        floatOp = true;
    } else if (leftVal->getType() == rightVal->getType()) {
    } else {
        leftVal = mycast(leftVal, Type::getInt64Ty(getGlobalContext()), cgconext);
        rightVal = mycast(rightVal, Type::getInt64Ty(getGlobalContext()), cgconext);
    }
    if (!leftVal || !rightVal)
        return nullptr;
    if (!floatOp)
    {
        switch (op) {
            case TokenType::Plus:
                return BinaryOperator::Create(Instruction::Add, leftVal, rightVal, "", cgconext.currentBlock());
            case TokenType::Minus:
                return BinaryOperator::Create(Instruction::Sub, leftVal, rightVal, "", cgconext.currentBlock());
            case TokenType::Multiplication:
                return BinaryOperator::Create(Instruction::Mul, leftVal, rightVal, "", cgconext.currentBlock());
            case TokenType::Division:
                return BinaryOperator::Create(Instruction::SDiv, leftVal, rightVal, "", cgconext.currentBlock());
            case TokenType::Equal:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::ICMP_EQ, leftVal, rightVal, "");
            case TokenType::NotEqual:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::ICMP_NE, leftVal, rightVal, "");
            case TokenType::Greater:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::ICMP_SGT, leftVal, rightVal, "");
            case TokenType::GreaterOrEqual:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::ICMP_SGE, leftVal, rightVal, "");
            case TokenType::Less:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::ICMP_SLT, leftVal, rightVal, "");
            case TokenType::LessOrEqual:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::ICMP_SLE, leftVal, rightVal, "");
            case TokenType::And:
                return BinaryOperator::Create(Instruction::And, leftVal, rightVal, "", cgconext.currentBlock());
            case TokenType::Or:
                return BinaryOperator::Create(Instruction::Or, leftVal, rightVal, "", cgconext.currentBlock());
        }
    }
    else
    {
        switch (op) {
            case TokenType::Plus:
                return BinaryOperator::Create(Instruction::FAdd, leftVal, rightVal, "", cgconext.currentBlock());
            case TokenType::Minus:
                return BinaryOperator::Create(Instruction::FSub, leftVal, rightVal, "", cgconext.currentBlock());
            case TokenType::Multiplication:
                return BinaryOperator::Create(Instruction::FMul, leftVal, rightVal, "", cgconext.currentBlock());
            case TokenType::Division:
                return BinaryOperator::Create(Instruction::FDiv, leftVal, rightVal, "", cgconext.currentBlock());
            case TokenType::Equal:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::FCMP_OEQ, leftVal, rightVal, "");
            case TokenType::NotEqual:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::FCMP_ONE, leftVal, rightVal, "");
            case TokenType::Greater:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::FCMP_OGT, leftVal, rightVal, "");
            case TokenType::GreaterOrEqual:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::FCMP_OGE, leftVal, rightVal, "");
            case TokenType::Less:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::FCMP_OLT, leftVal, rightVal, "");
            case TokenType::LessOrEqual:
                return new ICmpInst(*cgconext.currentBlock(), ICmpInst::FCMP_OLE, leftVal, rightVal, "");
        }
    }
    llvm::errs() << "[ERROR] Unknown or wrong operator.\n";
    return nullptr;
}

llvm::Value *ConditionalExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *CallExprNode::codegen(CodeGenContext &cgconext) {
    Function *function = cgconext.mModule->getFunction(name->value);
    if (function == nullptr)
        llvm::errs() << "[ERROR] Codegen - no such function \"" << name->value << "\".\n";
    std::vector<Value*> argsRef;
    for (auto it = args->begin(); it != args->end(); it++)
        argsRef.push_back((*it)->codegen(cgconext));
    CallInst *call = CallInst::Create(function, makeArrayRef(argsRef), "", cgconext.currentBlock());
    return call;
}

llvm::Value *BlockExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *AssignExprNode::codegen(CodeGenContext &cgconext) {
    Value* var;
    auto assignData = right->codegen(cgconext);
    var = cgconext.localsLookup(left->value);
    if (var == nullptr)
    {
        llvm::errs() << "[ERROR] Undeclared Variable \"" << left->value << "\".\n";
        return nullptr;
    }

    return new StoreInst(assignData, var, false, cgconext.currentBlock());
}

llvm::Value *FuncExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *CastExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *ExprStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *ReturnStatementNode::codegen(CodeGenContext &cgconext) {
    return cgconext.Builder.CreateRet(expr->codegen(cgconext));
}

llvm::Value *OutputStatementNode::codegen(CodeGenContext &cgconext) {
    Value *value = expr->codegen(cgconext);
    std::vector<Value *> printArgs;
    Value *formatStr = cgconext.Builder.CreateGlobalStringPtr("%s\n");
    if (value->getType()->isIntegerTy()) {
        formatStr = cgconext.Builder.CreateGlobalStringPtr("%d\n");
    }
    else if (value->getType()->isDoubleTy()) {
        formatStr = cgconext.Builder.CreateGlobalStringPtr("%f\n");
    }
    printArgs.push_back(formatStr);
    printArgs.push_back(value);
    cgconext.Builder.CreateCall(cgconext.mModule->getFunction("printf"), printArgs);
    return nullptr;
}

llvm::Value *DeclarationNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

static Type *typeOf(CodeGenContext &cgconext, const std::string &var) {
    Type *type = nullptr;
    if (var == "integer")
        type = Type::getInt64Ty(getGlobalContext());
    else if (var == "character")
        type = Type::getInt8Ty(getGlobalContext());
    else if (var == "real")
        type = Type::getDoubleTy(getGlobalContext());
    else if (var.empty())
        type = Type::getVoidTy(getGlobalContext());
    else
    {
        if (cgconext.allocatedClasses.contains(var))
            type = cgconext.allocatedClasses[var];
    }
    return type;
}

static Type *ptrToTypeOf(CodeGenContext &cgconext, const std::string &var) {
    Type *type = nullptr;
    if (var == "integer")
        type = Type::getInt64PtrTy(getGlobalContext());
    else if (var == "character")
        type = Type::getInt8PtrTy(getGlobalContext());
    else if (var == "real")
        type = Type::getDoublePtrTy(getGlobalContext());
    else if (var.empty())
        type = Type::getVoidTy(getGlobalContext());
    else
    {
        if (cgconext.allocatedClasses.contains(var))
            type = cgconext.allocatedClasses[var]->getPointerTo();
    }
    return type;
}

llvm::Value *VarDecStatementNode::codegen(CodeGenContext &cgconext) {
    Value* newVar = nullptr;
    Value* rightVal = nullptr;
    if (isGlobal)
    {
        cgconext.mModule->getOrInsertGlobal(name->value, typeOf(cgconext, type));
        auto gVar = cgconext.mModule->getNamedGlobal(name->value);
        gVar->setLinkage(GlobalValue::ExternalLinkage);
        if (expr != NULL)
        {
            rightVal = expr->codegen(cgconext);
            gVar->setInitializer(static_cast<Constant *>(rightVal));
        }
        gVar->setAlignment(Align(8));
        cgconext.globals()[name->value] = gVar;
    }
    else
    {
        newVar = cgconext.Builder.CreateAlloca(typeOf(cgconext, type), 0, nullptr, name->value);
        cgconext.locals()[name->value] = newVar;
        if (expr != NULL) {
            rightVal = expr->codegen(cgconext);
            //new StoreInst(rightVal, newVar, false, cgconext.currentBlock());
            cgconext.Builder.CreateStore(rightVal, newVar);
        }
    }

    return newVar;
}

llvm::Value *FuncParamDecStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *ArrayDecStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *IndexExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

std::string getParameterTypeName(ParameterType type)
{
    if (type == In) return "in";
    if (type == Out) return "out";
    if (type == Var) return "var";
    return "";
}

llvm::Value *FuncDecStatementNode::codegen(CodeGenContext &cgconext) {
    std::vector<llvm::Type*> argTypes;
    std::vector<int> refParams;
    int i = 1;
    for (auto arg : *args)
    {
        Type* argType = typeOf(cgconext, arg->type);
        if (arg->parameterType == ParameterType::Out || arg->parameterType == ParameterType::Var)
        {
            argTypes.push_back(ptrToTypeOf(cgconext, arg->type));
            refParams.push_back(i);
        }
        else
        {
            argTypes.push_back(typeOf(cgconext, arg->type));
        }
        i++;
    }
    FunctionType* funcType = FunctionType::get(typeOf(cgconext, type), argTypes, false);
    auto function = Function::Create(funcType, GlobalValue::ExternalLinkage, name->value, cgconext.mModule);
    for (auto paramID : refParams)
    {
        // TODO Set Dereferenceable attribute for ref params
        //function->addAttribute(paramID, Attribute::Dereferenceable);
    }
    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), type + name->value + "Entry", function, 0);
    cgconext.pushBlock(bb);
    cgconext.Builder.SetInsertPoint(bb);
    Function::arg_iterator argsValues = function->arg_begin();
    for (auto it = args->begin(); it != args->end(); it++, argsValues++) {
        if ((*it)->parameterType == ParameterType::Out || (*it)->parameterType == ParameterType::Var)
        {
            auto var = new AllocaInst(ptrToTypeOf(cgconext, (*it)->type), 0, nullptr, (*it)->name->value, bb);
            cgconext.locals()[(*it)->name->value] = var;
            Value *argumentValue = &(*argsValues);
            argumentValue->setName(getParameterTypeName((*it)->parameterType) + (*it)->name->value);
            StoreInst *inst = new StoreInst(argumentValue, cgconext.locals()[(*it)->name->value], false, bb);
        }
        else
        {
            auto var = new AllocaInst(typeOf(cgconext, (*it)->type), 0, nullptr, (*it)->name->value, bb);
            cgconext.locals()[(*it)->name->value] = var;
            Value *argumentValue = &(*argsValues);
            argumentValue->setName(getParameterTypeName((*it)->parameterType) + (*it)->name->value);
            StoreInst *inst = new StoreInst(argumentValue, cgconext.locals()[(*it)->name->value], false, bb);
        }
    }
    if (block != nullptr)
    {
        for (auto statement : *block->statements)
        {
            statement->codegen(cgconext);
        }
    }
    cgconext.popBlock();
    //cgconext.Builder.SetInsertPoint(cgconext.currentBlock());
    return nullptr;
}

llvm::Value *FieldDecNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *FieldVarDecNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *FieldArrayVarDecNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *MethodDecNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *TypeDecStatementNode::codegen(CodeGenContext &cgconext) {
    if (name->value == "Object") return nullptr;
    cgconext.allocatedClasses[name->value] = StructType::create(getGlobalContext(), name->value);
    cgconext.allocatedClasses[name->value]->setName(name->value);
    std::vector<Type*> dataTypes;
    for (auto field : *fields)
    {
        if (field->type == "integer") {
            dataTypes.push_back(Type::getInt64Ty(getGlobalContext()));
        }
        else if (field->type == "real") {
            dataTypes.push_back(Type::getDoubleTy(getGlobalContext()));
        }
        else if (field->type == "character")
        {
            dataTypes.push_back(Type::getInt8Ty(getGlobalContext()));
        }
        // ... array, class, string
    }
    cgconext.allocatedClasses[name->value]->setBody(dataTypes);
    return nullptr;
}

llvm::Value *ExternFuncDecStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *ElseIfStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *IfStatementNode::codegen(CodeGenContext &cgconext) {
    Function *TheFunction = cgconext.Builder.GetInsertBlock()->getParent();
    BasicBlock* ifTrue = BasicBlock::Create(getGlobalContext(), "", TheFunction, 0);
    BasicBlock* ifFalse = BasicBlock::Create(getGlobalContext(), "", TheFunction, 0);
    BasicBlock* ifEnd = BasicBlock::Create(getGlobalContext(), "", TheFunction, 0);
    BranchInst::Create(ifTrue, ifFalse, condExpr->codegen(cgconext), cgconext.currentBlock());
    // Entering IF
    cgconext.pushBlock(ifTrue);
    cgconext.Builder.SetInsertPoint(ifTrue);
    if (trueBlock != nullptr)
    {
        for (auto statement : *trueBlock->statements)
        {
            statement->codegen(cgconext);
        }
    }
    // JMP to END
    BranchInst::Create(ifEnd, cgconext.currentBlock());
    cgconext.popBlock();
    // Entering ELSE
    cgconext.pushBlock(ifFalse);
    cgconext.Builder.SetInsertPoint(ifFalse);
    if (falseBlock != nullptr)
    {
        for (auto statement : *falseBlock->statements)
        {
            statement->codegen(cgconext);
        }
    }
    // JMP to END
    BranchInst::Create(ifEnd, cgconext.currentBlock());
    cgconext.popBlock();
    // Return END
    cgconext.ret(ifEnd);
    cgconext.Builder.SetInsertPoint(ifEnd);
    return ifEnd;
}

llvm::Value *ForStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *WhileStatementNode::codegen(CodeGenContext &cgconext) {
    Function *TheFunction = cgconext.Builder.GetInsertBlock()->getParent();
    BasicBlock* whileIter = BasicBlock::Create(getGlobalContext(), "", TheFunction, 0);
    BasicBlock* whileEnd = BasicBlock::Create(getGlobalContext(), "", TheFunction, 0);
    BasicBlock* whileCheck = BasicBlock::Create(getGlobalContext(), "", TheFunction, 0);
    // Check condition satisfaction
    BranchInst::Create(whileCheck, cgconext.currentBlock());
    cgconext.pushBlock(whileCheck);
    cgconext.Builder.SetInsertPoint(whileCheck);
    // Whether break the loop
    BranchInst::Create(whileIter, whileEnd, whileExpr->codegen(cgconext), cgconext.currentBlock());
    cgconext.popBlock();
    // Entering loop block
    cgconext.pushBlock(whileIter);
    cgconext.Builder.SetInsertPoint(whileIter);
    if (block != nullptr)
    {
        for (auto statement : *block->statements)
        {
            statement->codegen(cgconext);
        }
    }
    // Jump back to condition checking
    BranchInst::Create(whileCheck, cgconext.currentBlock());
    cgconext.popBlock();
    // Return END
    cgconext.ret(whileEnd);
    cgconext.Builder.SetInsertPoint(whileEnd);
    return whileEnd;
}

llvm::Value *ModuleStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *ImportStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}
