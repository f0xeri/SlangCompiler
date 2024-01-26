//
// Created by f0xeri on 25.01.2024.
//

#include "DebugInfoBuilder.hpp"
#include "CodeGenContext.hpp"
#include "parser/AST.hpp"

llvm::DIType *Slangc::DebugInfoBuilder::createType(Slangc::TypeDecStatementNode *type, Slangc::CodeGenContext &context,
                                                   std::vector<ErrorMessage> &errors) {
    auto file = compileUnit->getFile();
    std::vector<llvm::Metadata *> members;
    uint64_t index = 0;
    if (type->vtableRequired && type->parentTypeName == "Object") {
        // vtable type
        auto vtableType = debugBuilder->createSubroutineType(debugBuilder->getOrCreateTypeArray({typeCache["integer"]}));
        members.push_back(debugBuilder->createMemberType(
                compileUnit,
                "vptr_" + type->name,
                file,
                type->loc.line,
                dataLayout.getTypeAllocSizeInBits(builder.getPtrTy()),
                0,
                0,
                llvm::DINode::FlagArtificial,
                getPointerType(vtableType)));
    }
    else if (type->parentTypeName != "Object") {
        index++;
    }
    for (; index < type->fields.size(); index++) {
        auto loc = getDeclLoc(type->fields[index]);
        auto fieldType = getDeclType(type->fields[index], context.context, errors).value();
        auto fieldDbgType = getDebugType(fieldType, context);
        uint64_t offset = dataLayout.getStructLayout(context.allocatedClasses[type->name])->getElementOffsetInBits(
                getFieldIndex(getDeclName(type->fields[index]).data(), type->fields));
        members.push_back(debugBuilder->createMemberType(
                compileUnit,
                getDeclName(type->fields[index]),
                file,
                loc.line,
                dataLayout.getTypeAllocSizeInBits(
                        getIRType(fieldType, context)),
                0,
                offset,
                llvm::DINode::FlagZero,
                fieldDbgType));
    }
    auto dbgType = debugBuilder->createClassType(
            compileUnit,
            type->name,
            file,
            type->loc.line,
            dataLayout.getTypeAllocSizeInBits(
                    context.allocatedClasses[type->name]),
            0,
            0,
            llvm::DINode::FlagZero,
            typeCache[type->parentTypeName->data()],
            debugBuilder->getOrCreateArray(members));
    if (type->vtableRequired) dbgType->replaceVTableHolder(dbgType);
    typeCache[type->name] = dbgType;
    if (type->parentTypeName != "Object") {
        auto parentDbgType = typeCache[type->parentTypeName->data()];
        members.push_back(debugBuilder->createInheritance(
                typeCache[type->name], parentDbgType, 0, 0, llvm::DINode::FlagPublic));
    }
    debugBuilder->replaceArrays(dbgType, debugBuilder->getOrCreateArray(members));
    return dbgType;
}

llvm::DIType *Slangc::DebugInfoBuilder::getPointerType(llvm::DIType *type) {
    return debugBuilder->createPointerType(type, dataLayout.getPointerSizeInBits());
}

llvm::DIType *Slangc::DebugInfoBuilder::getArrayType(Slangc::ArrayExprPtr arrayExpr, std::string_view finalType) {
    auto type = arrayExpr->type;
    auto dbgType = typeCache[finalType.data()];
    while (auto arrayType = std::get_if<ArrayExprPtr>(&type)) {
        type = arrayType->get()->type;
        dbgType = getPointerType(dbgType);
    }
    return dbgType;
}

llvm::DISubroutineType *
Slangc::DebugInfoBuilder::getFunctionType(Slangc::FuncExprPtr func, Slangc::CodeGenContext &context) {
    std::vector<llvm::Metadata *> paramTypes;
    paramTypes.push_back(getDebugType(func->type, context));
    for (auto &param: func->params) {
        paramTypes.push_back(getDebugType(param->type, context));
    }
    return debugBuilder->createSubroutineType(debugBuilder->getOrCreateTypeArray(paramTypes));
}

Slangc::DebugInfoBuilder::DebugInfoBuilder(const llvm::DataLayout &dataLayout, std::string_view filename,
                                           std::unique_ptr<llvm::DIBuilder> diBuilder, llvm::IRBuilder<> &builder)
        : dataLayout(dataLayout),
          debugBuilder(std::move(diBuilder)), builder(builder) {
    std::filesystem::path path = filename;
    compileUnit = debugBuilder->createCompileUnit(
            llvm::dwarf::DW_LANG_C,
            debugBuilder->createFile(path.filename().string(), path.parent_path().string()),
            "Slangc",
            false,
            "",
            0);

    typeCache["integer"] = debugBuilder->createBasicType("integer", 32, llvm::dwarf::DW_ATE_signed);
    typeCache["float"] = debugBuilder->createBasicType("float", 32, llvm::dwarf::DW_ATE_float);
    typeCache["real"] = debugBuilder->createBasicType("real", 64, llvm::dwarf::DW_ATE_float);
    typeCache["boolean"] = debugBuilder->createBasicType("boolean", 1, llvm::dwarf::DW_ATE_boolean);
    typeCache["character"] = debugBuilder->createBasicType("character", 8, llvm::dwarf::DW_ATE_signed_char);
    typeCache["void"] = debugBuilder->createBasicType("void", 0, llvm::dwarf::DW_ATE_signed);
}

void Slangc::DebugInfoBuilder::emitLocation() {
    builder.SetCurrentDebugLocation(DebugLoc());
}

void Slangc::DebugInfoBuilder::emitLocation(Slangc::SourceLoc loc) {
    DIScope *scope;
    lexicalBlocks.empty() ? scope = compileUnit : scope = lexicalBlocks.back();
    builder.SetCurrentDebugLocation(DILocation::get(scope->getContext(), loc.line, loc.column, scope));
}

llvm::DISubprogram *
Slangc::DebugInfoBuilder::createFunction(FuncDecStatementNode *func, Slangc::CodeGenContext &context) {
    auto file = compileUnit->getFile();
    auto debugFunc = debugBuilder->createFunction(
            compileUnit,
            func->name,
            func->name + "." + getMangledFuncName(func->expr),
            file,
            func->loc.line,
            getFunctionType(func->expr, context),
            func->loc.line,
            func->isPrivate ? llvm::DINode::FlagPrivate : llvm::DINode::FlagPublic,
            func->isExtern ? llvm::DISubprogram::SPFlagDefinition : llvm::DISubprogram::SPFlagZero);
    return debugFunc;
}

llvm::DISubprogram *Slangc::DebugInfoBuilder::createMainFunction(Slangc::CodeGenContext &context) {
    auto file = compileUnit->getFile();
    auto debugFunc = debugBuilder->createFunction(
            compileUnit,
            "main",
            "main",
            file,
            0,
            debugBuilder->createSubroutineType(debugBuilder->getOrCreateTypeArray({typeCache["integer"]})),
            0,
            llvm::DINode::FlagPublic,
            llvm::DISubprogram::SPFlagDefinition);
    return debugFunc;
}

void
Slangc::DebugInfoBuilder::createLocalVar(std::string_view name, llvm::DIType *type, llvm::Value *value, SourceLoc loc) {
    auto file = compileUnit->getFile();
    auto scope = lexicalBlocks.back();
    auto dbgVar = debugBuilder->createAutoVariable(scope, name, file, loc.line, type);
    debugBuilder->insertDeclare(
            value,
            dbgVar,
            debugBuilder->createExpression(),
            DILocation::get(scope->getContext(), loc.line, loc.column, scope),
            builder.GetInsertBlock());
}

llvm::DIGlobalVariableExpression *
Slangc::DebugInfoBuilder::createGlobalVar(std::string_view name, llvm::DIType *type, llvm::Value *value, SourceLoc loc,
                                          bool isPrivate) {
    auto file = compileUnit->getFile();
    return debugBuilder->createGlobalVariableExpression(
            compileUnit,
            name,
            name,
            file,
            loc.line,
            type,
            true,
            value);
}

void Slangc::DebugInfoBuilder::finalize() {
    debugBuilder->finalize();
}
