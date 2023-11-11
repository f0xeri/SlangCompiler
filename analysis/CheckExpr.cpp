//
// Created by user on 03.11.2023.
//

#include "Check.hpp"

namespace Slangc::Check {
    bool checkExpr(const ExprPtrVariant &expr, Context &context, std::vector<ErrorMessage> &errors);

    bool checkExpr(const ArrayExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        auto type = expr->type;
        while (std::holds_alternative<ArrayExprPtr>(type)) {
            type = std::get<ArrayExprPtr>(type)->type;
        }
        return checkExpr(type, context, errors);
    }

    bool checkExpr(const BooleanExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const CharExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const FloatExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const FuncExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(expr->type, context, errors);
        for (const auto &param: expr->params) {
            result &= checkStmt(param, context, errors);
        }
        return result;
    }

    bool checkExpr(const IntExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const NilExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const OperatorExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (!checkExpr(expr->left, context, errors)) {
            result = false;
        }
        if (!checkExpr(expr->right, context, errors)) {
            result = false;
        }
        auto leftType = std::get<TypeExprPtr>(getExprType(expr->left, context))->type;
        auto rightType = std::get<TypeExprPtr>(getExprType(expr->right, context))->type;
        // TODO: check if conversion is possible
        if (leftType != rightType) {
            errors.emplace_back(std::string("Type mismatch: cannot apply operator '") + "' to '" + leftType + "' and '" + rightType + "'.", expr->loc, false, false);
            result = false;
        }
        return result;
    }

    bool checkExpr(const RealExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const StringExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const UnaryOperatorExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const VarExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (!context.lookup(expr->name)) {
            if (context.lookup(context.moduleName + "." + expr->name)) {
                expr->name = context.moduleName + "." + expr->name;
            }
            else {
                errors.emplace_back("Variable, type, or function with name '" + expr->name + "' does not exist.", expr->loc, false, false);
                result = false;
            }
        }
        return result;
    }

    bool checkExpr(const IndexesExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const IndexExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const CallExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(expr->name, context, errors);
        auto type = getExprType(expr, context);
        return result;
    }

    bool checkExpr(const AccessExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return true;
    }

    bool checkExpr(const TypeExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (!context.lookup(expr->type) && !Context::isBuiltInType(expr->type)) {
            errors.emplace_back("Type '" + expr->type + "' does not exist.", expr->loc, false, false);
            result = false;
        }
        return result;
    }

    bool checkExpr(const ExprPtrVariant &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return std::visit([&](const auto &expr) {
            return checkExpr(expr, context, errors);
        }, expr);
    }
}