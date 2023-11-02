//
// Created by user on 02.11.2023.
//

#include "Check.hpp"

namespace Slangc::Check {
    auto checkStmt(const StmtPtrVariant &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool;
    auto checkBlockStmt(const BlockStmtPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        bool result = true;
        for (const auto &statement: stmt->statements) {
            result &= checkStmt(statement, context, errors);
        }
        return result;
    }

    auto checkStmt(const VarDecStatementPtr &stmt, Context context, std::vector<ErrorMessage> &errors) -> bool {
        bool result = true;
        auto type = context.lookup(stmt->typeExpr.type);
        if (!type && !Context::isBuiltInType(stmt->typeExpr.type)) {
            errors.emplace_back("Type '" + stmt->typeExpr.type + "' does not exist.", stmt->loc, false, false);
            result = false;
        }
        if (context.lookup(stmt->name)) {
            errors.emplace_back("Variable with name '" + stmt->name + "' already exists.", stmt->loc, false, false);
            result = false;
        }
        context.insert(stmt->name, stmt);
        if (stmt->expr.has_value() && result) {
            if (!checkExpr(stmt->expr.value(), context, errors)) {
                return false;
            }
            auto exprType = std::get<TypeExprPtr>(getExprType(stmt->expr.value(), context))->type;
            // TODO: check if conversion is possible
            if (stmt->typeExpr.type != exprType) {
                errors.emplace_back("Type mismatch: cannot assign '" + exprType + "' to '" + stmt->typeExpr.type + "'.", stmt->loc, false, false);
                result = false;
            }
        }
        return result;
    }

    auto checkStmt(const ArrayDecStatementPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const FuncPointerStatementPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const IfStatementPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const WhileStatementPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const OutputStatementPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const InputStatementPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const AssignExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const ReturnStatementPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const CallExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const DeleteStmtPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkStmt(const StmtPtrVariant &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return std::visit([&](const auto &stmt) {
            return checkStmt(stmt, context, errors);
        }, stmt);
    }
};