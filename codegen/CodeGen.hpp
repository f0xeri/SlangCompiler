//
// Created by f0xeri on 05.05.2023.
//

#ifndef SLANGCREFACTORED_CODEGEN_HPP
#define SLANGCREFACTORED_CODEGEN_HPP

#include "parser/AST.hpp"

namespace Slangc {

    class CodeGen {
    public:
        CodeGen() = default;
    private:
        CodeGenContext context;


        auto processNode(const auto &node) -> bool {
            return std::visit(node.codegen, context);
        }
    };

} // Slangc

#endif //SLANGCREFACTORED_CODEGEN_HPP
