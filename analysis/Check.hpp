//
// Created by user on 02.11.2023.
//

#ifndef SLANGCREFACTORED_CHECK_HPP
#define SLANGCREFACTORED_CHECK_HPP


#include "parser/AST.hpp"
#include "Context.hpp"

namespace Slangc::Check {
    auto checkAST(const ModuleDeclPtr& moduleAST, const Context &context, std::vector<ErrorMessage> &errors) -> bool;
    auto checkBlockStmt(const BlockStmtPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool;
    auto checkExpr(const ExprPtrVariant &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool;
};


#endif //SLANGCREFACTORED_CHECK_HPP
