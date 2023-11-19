//
// Created by user on 02.11.2023.
//

#include "Check.hpp"

namespace Slangc::Check {
    auto checkAST(const ModuleDeclPtr& moduleAST, Context &context, std::vector<ErrorMessage> &errors) -> bool {
        // TODO: check imports
        // TODO: we should check all symbols, not funcs and types separately
        for (auto &decl: context.symbolTable.symbols | std::views::filter([&](const auto& symbol) { return symbol.moduleName == moduleAST->name; })) {
            checkDecl(decl.declaration, context, errors);
        }
        checkBlockStmt(moduleAST->block, context, errors);
        return true;
    }
};
