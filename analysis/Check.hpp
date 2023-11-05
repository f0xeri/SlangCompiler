//
// Created by user on 02.11.2023.
//

#ifndef SLANGCREFACTORED_CHECK_HPP
#define SLANGCREFACTORED_CHECK_HPP


#include "parser/AST.hpp"
#include "Context.hpp"

namespace Slangc::Check {
    auto checkAST(const ModuleDeclPtr& moduleAST, Context &context, std::vector<ErrorMessage> &errors) -> bool;
    auto checkBlockStmt(const BlockStmtPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) -> bool;
    auto checkExpr(const ExprPtrVariant &stmt, Context &context, std::vector<ErrorMessage> &errors) -> bool;
    auto checkStmt(const StmtPtrVariant &stmt, Context &context, std::vector<ErrorMessage> &errors) -> bool;
    auto checkDecl(const DeclPtrVariant &decl, Context &context, std::vector<ErrorMessage> &errors) -> bool;
};


#endif //SLANGCREFACTORED_CHECK_HPP
