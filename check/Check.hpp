//
// Created by user on 02.11.2023.
//

#ifndef SLANGCREFACTORED_CHECK_HPP
#define SLANGCREFACTORED_CHECK_HPP


#include "parser/AST.hpp"
#include "Context.hpp"
#include <ranges>

namespace Slangc::Check {
    bool checkAST(const ModuleDeclPtr& moduleAST, Context &context);
    bool checkBlockStmt(const BlockStmtPtr &stmt, Context &context);
    bool checkExpr(const ExprPtrVariant &stmt, Context &context);
    bool checkStmt(const StmtPtrVariant &stmt, Context &context);
    bool checkDecl(const DeclPtrVariant &decl, Context &context);
};


#endif //SLANGCREFACTORED_CHECK_HPP
