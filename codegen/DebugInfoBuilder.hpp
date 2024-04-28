//
// Created by f0xeri on 25.01.2024.
//

#ifndef SLANGCREFACTORED_DEBUGINFOBUILDER_HPP
#define SLANGCREFACTORED_DEBUGINFOBUILDER_HPP

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DIBuilder.h>
#include <map>
#include <llvm/IR/IRBuilder.h>
#include "parser/ASTFwdDecl.hpp"
#include "common.hpp"

namespace Slangc {
    class CodeGenContext;
    class DebugInfoBuilder {
        llvm::DataLayout dataLayout;
        std::unique_ptr<llvm::DIBuilder> debugBuilder;
        llvm::DICompileUnit* compileUnit;
        llvm::IRBuilder<>& builder;
    public:
        std::vector<llvm::DIScope *> lexicalBlocks;
        std::map<std::string, llvm::DIType *> typeCache;
        explicit DebugInfoBuilder(const llvm::DataLayout& dataLayout, std::string_view filename, std::unique_ptr<llvm::DIBuilder> diBuilder, llvm::IRBuilder<>& builder);
        llvm::DIType *createType(TypeDecStatementNode* type, CodeGenContext &context);
        llvm::DISubprogram *createFunction(FuncDecStatementNode* func, CodeGenContext &context);
        llvm::DISubprogram *createMainFunction(CodeGenContext &context);
        void createLocalVar(std::string_view name, llvm::DIType* type, llvm::Value* value, SourceLoc loc);
        void createLocalFuncParam(std::string_view name, llvm::DIType* type, llvm::Value* value, SourceLoc loc, llvm::DISubprogram* dbgFunc, uint64_t argNo);
        llvm::DISubprogram *createDefaultConstructor(std::string_view name, llvm::DIType *type, SourceLoc loc);
        llvm::DISubprogram *createDefaultDestructor(std::string_view name, llvm::DIType *type, SourceLoc loc);
        llvm::DIGlobalVariableExpression* createGlobalVar(std::string_view name, llvm::DIType* type, SourceLoc loc, bool isPrivate);
        llvm::DIType *getPointerType(llvm::DIType *type);
        llvm::DIType *getArrayType(ArrayExprPtr arrayExpr, Slangc::CodeGenContext &context);
        llvm::DISubroutineType *getFunctionType(FuncExprPtr func, CodeGenContext &context);
        void finalize();

        void emitLocation();
        void emitLocation(SourceLoc loc);
    };
}


#endif //SLANGCREFACTORED_DEBUGINFOBUILDER_HPP
