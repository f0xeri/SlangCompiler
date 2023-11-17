//
// Created by user on 02.11.2023.
//

#ifndef SLANGCREFACTORED_CHECK_HPP
#define SLANGCREFACTORED_CHECK_HPP


#include "parser/AST.hpp"
#include "Context.hpp"
#include <ranges>

namespace Slangc::Check {
    bool checkAST(const ModuleDeclPtr& moduleAST, Context &context, std::vector<ErrorMessage> &errors);
    bool checkBlockStmt(const BlockStmtPtr &stmt, Context &context, std::vector<ErrorMessage> &errors);
    bool checkExpr(const ExprPtrVariant &stmt, Context &context, std::vector<ErrorMessage> &errors);
    bool checkStmt(const StmtPtrVariant &stmt, Context &context, std::vector<ErrorMessage> &errors);
    bool checkDecl(const DeclPtrVariant &decl, Context &context, std::vector<ErrorMessage> &errors);
};


#endif //SLANGCREFACTORED_CHECK_HPP
