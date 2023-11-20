//
// Created by f0xeri on 05.05.2023.
//

#ifndef SLANGCREFACTORED_CODEGEN_HPP
#define SLANGCREFACTORED_CODEGEN_HPP

#include "parser/AST.hpp"
#include "check/Context.hpp"

namespace Slangc {
    Value* processNode(const auto& node, CodeGenContext &codeGenContext, std::vector<ErrorMessage>& errors) {
        auto call = [&](auto& expr) -> Value* { return expr.get()->codegen(codeGenContext, errors); };
        return std::visit(call, node);
    }
    class CodeGen {
    public:
        CodeGen(Context& context, ModuleDeclPtr&&moduleAST, bool isMainModule) : context(context),
            moduleAST(std::move(moduleAST)), isMainModule(isMainModule),
            codeGenContext(context, isMainModule) {}

        void process(std::vector<ErrorMessage>& errors) {

            for (auto& symbol: context.symbolTable.symbols | std::views::filter([&](const auto&s) { return s.moduleName == moduleAST->name; })) {
                processNode(symbol.declaration, codeGenContext, errors);
            }

            if (isMainModule) {
                codeGenContext.startMainFunc();
                for (auto&stmt: moduleAST->block->statements) {
                    processNode(stmt, codeGenContext, errors);
                }
                codeGenContext.endMainFunc();
            }
        }

        void dumpIRToFile(const std::string&filename) const {
            std::error_code EC;
            auto outFileStream = raw_fd_ostream(filename, EC, llvm::sys::fs::OF_None);
            codeGenContext.module->print(outFileStream, nullptr);
        }


    private:
        ModuleDeclPtr moduleAST;
        Context context;
        CodeGenContext codeGenContext;
        bool isMainModule;

    };
} // Slangc

#endif //SLANGCREFACTORED_CODEGEN_HPP
