//
// Created by f0xeri on 05.05.2023.
//

#ifndef SLANGCREFACTORED_CODEGEN_HPP
#define SLANGCREFACTORED_CODEGEN_HPP

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include "parser/AST.hpp"
#include "check/Context.hpp"
#include "CodeGenContext.hpp"

namespace Slangc {
    llvm::Value* processNode(const auto& node, CodeGenContext &codeGenContext) {
        auto call = [&](auto& expr) -> llvm::Value* { return expr.get()->codegen(codeGenContext); };
        return std::visit(call, node);
    }
    class CodeGen {
    public:
        CodeGen(Context& context, ModuleDeclPtr&& moduleAST, bool isMainModule, bool debug, bool gcEnabled) : context(context),
            moduleAST(std::move(moduleAST)), isMainModule(isMainModule),
            codeGenContext(context, debug, gcEnabled) {}

        void process() {

            for (auto& symbol: context.symbolTable.symbols /*| std::views::filter([&](const auto&s) { return s.moduleName == moduleAST->name; })*/) {
                if (symbol.isImported) codeGenContext.currentDeclImported = true;
                processNode(symbol.declaration, codeGenContext);
                codeGenContext.currentDeclImported = false;
            }

            if (isMainModule) {
                codeGenContext.startMainFunc();
                if (codeGenContext.debug) codeGenContext.debugBuilder->emitLocation(moduleAST->block->loc);
                for (auto&stmt: moduleAST->block->statements) {
                    processNode(stmt, codeGenContext);
                }
                // cleanupCurrentScope(codeGenContext);
                codeGenContext.endMainFunc();
            }
            llvm::verifyModule(*codeGenContext.module, &llvm::errs());
        }

        void dumpIRToFile(const std::string&filename) const {
            std::error_code EC;
            auto outFileStream = raw_fd_ostream(filename, EC, llvm::sys::fs::OF_None);
            codeGenContext.module->print(outFileStream, nullptr);
        }

        // generate object file
        void generateObjectFile(const std::string& filename) const {
            std::error_code EC;
            auto outFileStream = llvm::raw_fd_ostream(filename, EC, llvm::sys::fs::OF_None);
            llvm::legacy::PassManager pass;
            auto fileType = llvm::CodeGenFileType::ObjectFile;
            if (codeGenContext.targetMachine->addPassesToEmitFile(pass, outFileStream, nullptr, fileType)) {
                llvm::errs() << "TargetMachine can't emit a file of this type";
            }
            pass.run(*codeGenContext.module);
            outFileStream.flush();
        }

        llvm::Module* getModule() const {
            return codeGenContext.module.get();
        }


    private:
        ModuleDeclPtr moduleAST;
        Context context;
        CodeGenContext codeGenContext;
        bool isMainModule;

    };
} // Slangc

#endif //SLANGCREFACTORED_CODEGEN_HPP
