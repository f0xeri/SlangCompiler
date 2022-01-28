//
// Created by f0xeri on 23.01.2022.
//

#include "AST.hpp"

using namespace llvm;

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
    return nullptr;
}

llvm::Value *UnaryOperatorExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *OperatorExprNode::codegen(CodeGenContext &cgconext) {
    Value* leftVal = left->codegen(cgconext);
    Value* rightVal = right->codegen(cgconext);
    if (!leftVal || !rightVal)
        return nullptr;
    switch (op) {
        case Plus:
            return BinaryOperator::Create(Instruction::Add, leftVal, rightVal, "", cgconext.currentBlock());
            break;
        case Minus:
            return BinaryOperator::Create(Instruction::Sub, leftVal, rightVal, "", cgconext.currentBlock());
            break;
        case Multiply:
            return BinaryOperator::Create(Instruction::Mul, leftVal, rightVal, "", cgconext.currentBlock());
            break;
        case Divide:
            return BinaryOperator::Create(Instruction::SDiv, leftVal, rightVal, "", cgconext.currentBlock());
            break;
    }
}

llvm::Value *ConditionalExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *CallExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *BlockExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *AssignExprNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
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
    return nullptr;
}

llvm::Value *OutputStatementNode::codegen(CodeGenContext &cgconext) {
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
    /*else if (var->value == "void")
        type = Type::getVoidTy(TheContext);*/
    else
    {
        if (cgconext.allocatedClasses.contains(var))
            type = cgconext.allocatedClasses[var];
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
        gVar->setLinkage(GlobalValue::CommonLinkage);
        if (expr != NULL)
        {
            rightVal = expr->codegen(cgconext);
            gVar->setInitializer(static_cast<Constant *>(rightVal));
        }
        gVar->setAlignment(Align(8));
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

llvm::Value *FuncDecStatementNode::codegen(CodeGenContext &cgconext) {
    std::vector<llvm::Type*> argTypes;
    for (auto arg : *args)
    {
        argTypes.push_back(typeOf(cgconext, arg->type));
    }
    FunctionType* funcType = FunctionType::get(typeOf(cgconext, type), argTypes, false);
    auto function = Function::Create(funcType, GlobalValue::ExternalLinkage, name->value, cgconext.mModule);
    BasicBlock *bb = BasicBlock::Create(getGlobalContext(), type + name->value + "Entry", function, 0);
    cgconext.pushBlock(bb);
    //cgconext.Builder.SetInsertPoint(bb);
    Function::arg_iterator argsValues = function->arg_begin();
    for (auto it = args->begin(); it != args->end(); it++, argsValues++) {
        auto var = new AllocaInst(typeOf(cgconext, (*it)->type), 0, nullptr, (*it)->name->value, bb);
        cgconext.locals()[(*it)->name->value] = var;
        Value *argumentValue = &(*argsValues);
        argumentValue->setName((*it)->name->value.c_str());
        StoreInst *inst = new StoreInst(argumentValue, cgconext.locals()[(*it)->name->value], false, bb);
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
    return nullptr;
}

llvm::Value *ForStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *WhileStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *ModuleStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}

llvm::Value *ImportStatementNode::codegen(CodeGenContext &cgconext) {
    return nullptr;
}
