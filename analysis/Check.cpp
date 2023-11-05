//
// Created by user on 02.11.2023.
//

#include "Check.hpp"

namespace Slangc::Check {
    auto checkAST(const ModuleDeclPtr& moduleAST, Context &context, std::vector<ErrorMessage> &errors) -> bool {
        for (const auto &decl: context.funcs) {
            checkDecl(decl, context, errors);
        }
        checkBlockStmt(moduleAST->block, context, errors);
        return true;
    }
};
