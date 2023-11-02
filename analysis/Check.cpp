//
// Created by user on 02.11.2023.
//

#include "Check.hpp"

namespace Slangc::Check {
    auto checkAST(const ModuleDeclPtr& moduleAST, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        checkBlockStmt(moduleAST->block, context, errors);
        return true;
    }
};
