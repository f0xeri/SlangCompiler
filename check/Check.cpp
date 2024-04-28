//
// Created by user on 02.11.2023.
//

#include "Check.hpp"

namespace Slangc::Check {
    auto checkAST(const ModuleDeclPtr& moduleAST, Context &context) -> bool {
        // TODO: check imports
        // TODO: we should check all symbols, not funcs and types separately
        for (auto &decl: context.symbolTable.symbols) {
            if (decl.isImported)
                context.insert(decl.name, decl.declaration);
            else
                checkDecl(decl.declaration, context);
        }
        checkBlockStmt(moduleAST->block, context);
        return true;
    }
};
