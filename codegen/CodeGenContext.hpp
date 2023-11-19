//
// Created by f0xeri on 05.05.2023.
//

#ifndef SLANGCREFACTORED_CODEGENCONTEXT_HPP
#define SLANGCREFACTORED_CODEGENCONTEXT_HPP

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/MathExtras.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Pass.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/IR/DIBuilder.h>

#include "check/Context.hpp"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetSelect.h"
#include "parser/ASTFwdDecl.hpp"

namespace Slangc {
    using namespace llvm;
    struct CodeGenBlock {
        BasicBlock* block;
        std::map<std::string, Value*> locals;
        std::map<std::string, DeclPtrVariant> localsDecls;
        explicit CodeGenBlock(BasicBlock* block) : block(block) {}
    };

    class CodeGenContext {
    public:
        std::unique_ptr<LLVMContext> llvmContext;
        std::unique_ptr<Module> module;
        std::unique_ptr<IRBuilder<>> builder;
        std::unique_ptr<DIBuilder> debugBuilder;
        std::vector<CodeGenBlock*> blocks;
        Context& context;

        std::map<std::string, llvm::StructType *> allocatedClasses;

        CodeGenContext(Context &context, bool isMainModule) : context(context) {
            llvmContext = std::make_unique<LLVMContext>();
            module = std::make_unique<Module>(context.moduleName, *llvmContext);
            module->setSourceFileName(context.filename);
            builder = std::make_unique<IRBuilder<>>(*llvmContext);
            debugBuilder = std::make_unique<DIBuilder>(*module);
        }
        ~CodeGenContext() = default;

        auto startMainFunc() -> llvm::Function* {
            auto mainFuncType = llvm::FunctionType::get(Type::getInt32Ty(*llvmContext), false);
            auto mainFunc = llvm::Function::Create(mainFuncType, Function::ExternalLinkage, "main", module.get());
            auto mainBlock = BasicBlock::Create(*llvmContext, "entry", mainFunc);
            builder->SetInsertPoint(mainBlock);
            pushBlock(mainBlock);
            return mainFunc;
        }

        void endMainFunc() {
            builder->CreateRet(ConstantInt::get(Type::getInt32Ty(*llvmContext), 0));
            popBlock();
        }

        void pushBlock(BasicBlock *block) {
            blocks.push_back(new CodeGenBlock(block));
        }

        void popBlock() {
            auto top = blocks.back();
            blocks.pop_back();
            delete top;
        }

        void ret(std::unique_ptr<CodeGenBlock> block) {
            builder->CreateRetVoid();
            popBlock();
        }
    };
    Type* typeOf(const std::string& type, CodeGenContext& context);
}

#endif //SLANGCREFACTORED_CODEGENCONTEXT_HPP
