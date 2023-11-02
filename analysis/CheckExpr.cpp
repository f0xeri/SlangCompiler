//
// Created by user on 03.11.2023.
//

#include "Check.hpp"

namespace Slangc::Check {
    auto checkExpr(const ExprPtrVariant &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool;

    auto checkExpr(const ArrayExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const BooleanExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const CharExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const FloatExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const FuncExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const IntExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const NilExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const OperatorExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        bool result = true;
        if (!checkExpr(stmt->left, context, errors)) {
            result = false;
        }
        if (!checkExpr(stmt->right, context, errors)) {
            result = false;
        }
        auto leftType = std::get<TypeExprPtr>(getExprType(stmt->left, context))->type;
        auto rightType = std::get<TypeExprPtr>(getExprType(stmt->right, context))->type;
        // TODO: check if conversion is possible
        if (leftType != rightType) {
            errors.emplace_back(std::string("Type mismatch: cannot apply operator '") + "' to '" + leftType + "' and '" + rightType + "'.", stmt->loc, false, false);
            result = false;
        }
        return result;
    }

    auto checkExpr(const RealExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const StringExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const UnaryOperatorExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const VarExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const IndexesExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const IndexExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const CallExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const AccessExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const TypeExprPtr &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return true;
    }

    auto checkExpr(const ExprPtrVariant &stmt, const Context &context, std::vector<ErrorMessage> &errors) -> bool {
        return std::visit([&](const auto &expr) {
            return checkExpr(expr, context, errors);
        }, stmt);
    }
}