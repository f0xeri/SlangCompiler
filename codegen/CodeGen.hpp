//
// Created by f0xeri on 05.05.2023.
//

#ifndef SLANGCREFACTORED_CODEGEN_HPP
#define SLANGCREFACTORED_CODEGEN_HPP

#include "parser/AST.hpp"

namespace Slangc {

    class CodeGen {
    public:
        CodeGen(ModuleDeclPtr moduleAST) : moduleAST(std::move(moduleAST)) {}
        ModuleDeclPtr moduleAST;
        auto process() -> void {
            for (auto &stmt : moduleAST->block->statements) {
                processNode(stmt);
            }
        }
    private:
        CodeGenContext context;

        auto processNode(const auto &node) -> void {
            auto call = [this](auto& expr) { return expr.get()->codegen(context); };
            std::visit(call, node);
        }
    };

} // Slangc

#endif //SLANGCREFACTORED_CODEGEN_HPP
