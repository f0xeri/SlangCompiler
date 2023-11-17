//
// Created by user on 02.11.2023.
//

#include <iostream>
#include "Check.hpp"

namespace Slangc::Check {
    bool checkStmt(const StmtPtrVariant &stmt, Context &context, std::vector<ErrorMessage> &errors);
    bool checkBlockStmt(const BlockStmtPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        for (const auto &statement: stmt->statements) {
            result &= checkStmt(statement, context, errors);
        }
        return result;
    }

    bool checkStmt(const VarDecStmtPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (!context.symbolTable.lookupType(stmt->typeExpr.type) && !Context::isBuiltInType(stmt->typeExpr.type)) {
            if (!context.symbolTable.lookupType(context.moduleName + "." + stmt->typeExpr.type)) {
                errors.emplace_back("Type '" + stmt->typeExpr.type + "' does not exist.", stmt->loc, false, false);
                result = false;
            } else {
                // update type name
                stmt->typeExpr.type = context.moduleName + "." + stmt->typeExpr.type;
            }
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
            auto exprType = std::get<TypeExprPtr>(getExprType(stmt->expr.value(), context, errors).value())->type;
            if (exprType != stmt->typeExpr.type) {
                if (!Context::isCastable(exprType, stmt->typeExpr.type, context)) {
                    errors.emplace_back("Type mismatch: cannot assign '" + exprType + "' to '" + stmt->typeExpr.type + "'.", stmt->loc, false, false);
                    result = false;
                }
                else {
                    errors.emplace_back("Implicit conversion from '" + exprType + "' to '" + stmt->typeExpr.type + "'.", stmt->loc, true, false);
                }
            }
        }
        return result;
    }

    bool checkStmt(const ArrayDecStatementPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (!checkExpr(stmt->expr, context, errors)) result = false;
        if (context.lookup(stmt->name)) {
            errors.emplace_back("Variable with name '" + stmt->name + "' already exists.", stmt->loc, false, false);
            result = false;
        }
        context.insert(stmt->name, stmt);
        return result;
    }

    bool checkStmt(const FuncPointerStmtPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (context.lookup(stmt->name)) {
            errors.emplace_back("Variable with name '" + stmt->name + "' already exists.", stmt->loc, false, false);
            result = false;
        }
        result &= checkExpr(stmt->expr, context, errors);
        context.insert(stmt->name, stmt);
        if (stmt->assignExpr.has_value()) {
            result &= checkExpr(stmt->assignExpr.value(), context, errors);
            if (result) {
                auto leftType = stmt->expr;
                auto rightType = getExprType(stmt->assignExpr.value(), context, errors).value();
                // searching for overloaded function
                if (auto varExpr = std::get_if<VarExprPtr>(&stmt->assignExpr.value())) {
                    if (auto func = context.symbolTable.lookupFunc(varExpr->get()->name, stmt->expr)) {
                        rightType = std::get<FuncDecStatementPtr>(*func)->expr;
                    }
                }
                if (!compareFuncSignatures(leftType, std::get<FuncExprPtr>(rightType))) {
                    errors.emplace_back("Type mismatch: no matching function found, cannot assign '" + typeToString(rightType) + "' to '" + typeToString(leftType) + "'.", stmt->loc, false, false);
                    result = false;
                }
            }
        }
        return result;
    }

    bool checkStmt(const IfStatementPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result &= checkExpr(stmt->condition, context, errors);
        if (result) {
            auto condType = typeToString(getExprType(stmt->condition, context, errors).value());
            if (condType != "boolean") {
                if (!Context::isCastable(condType, "boolean", context)) {
                    errors.emplace_back("Type mismatch: cannot use '" + condType + "' as condition.", stmt->loc, false, false);
                    result = false;
                }
                else {
                    errors.emplace_back("Implicit conversion from '" + condType + "' to 'boolean'.", stmt->loc, true, false);
                }
            }
        }
        context.enterScope();
        result &= checkBlockStmt(stmt->trueBlock, context, errors);
        context.exitScope();
        for (const auto &elseIf: stmt->elseIfNodes) {
            result &= checkStmt(elseIf, context, errors);
        }
        if (stmt->falseBlock.has_value()) {
            context.enterScope();
            result &= checkBlockStmt(stmt->falseBlock.value(), context, errors);
            context.exitScope();
        }
        return result;
    }

    bool checkStmt(const ElseIfStatementPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result &= checkExpr(stmt->condition, context, errors);
        if (result) {
            auto condType = typeToString(getExprType(stmt->condition, context, errors).value());
            if (condType != "boolean") {
                if (!Context::isCastable(condType, "boolean", context)) {
                    errors.emplace_back("Type mismatch: cannot use '" + condType + "' as condition.", stmt->loc, false, false);
                    result = false;
                }
                else {
                    errors.emplace_back("Implicit conversion from '" + condType + "' to 'boolean'.", stmt->loc, true, false);
                }
            }
        }
        context.enterScope();
        result &= checkBlockStmt(stmt->trueBlock, context, errors);
        context.exitScope();
        return result;
    }

    bool checkStmt(const WhileStatementPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result &= checkExpr(stmt->condition, context, errors);
        if (result) {
            auto condType = typeToString(getExprType(stmt->condition, context, errors).value());
            if (condType != "boolean") {
                if (!Context::isCastable(condType, "boolean", context)) {
                    errors.emplace_back("Type mismatch: cannot use '" + condType + "' as condition.", stmt->loc, false, false);
                    result = false;
                }
                else {
                    errors.emplace_back("Implicit conversion from '" + condType + "' to 'boolean'.", stmt->loc, true, false);
                }
            }
        }
        context.enterScope();
        result &= checkBlockStmt(stmt->block, context, errors);
        context.exitScope();
        return result;
    }

    bool checkStmt(const OutputStatementPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result &= checkExpr(stmt->expr, context, errors);
        return result;
    }

    bool checkStmt(const InputStatementPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result &= checkExpr(stmt->expr, context, errors);
        if (result) {
            result &= Context::isBuiltInType(typeToString(getExprType(stmt->expr, context, errors).value()));
        }
        return result;
    }

    bool checkStmt(const AssignExprPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result &= checkExpr(stmt->left, context, errors);
        result &= checkExpr(stmt->right, context, errors);
        if (result) {
            auto leftType = getExprType(stmt->left, context, errors).value();
            auto rightType = getExprType(stmt->right, context, errors).value();
            // check if left is func type
            if (auto left = std::get_if<FuncExprPtr>(&leftType)) {
                if (auto varExpr = std::get_if<VarExprPtr>(&stmt->right)) {
                    if (auto func = context.symbolTable.lookupFunc(varExpr->get()->name, *left)) {
                        rightType = std::get<FuncDecStatementPtr>(*func)->expr;
                    }
                }
            }
            // we don't need to compare function signatures here because we can just check their text representation
            auto leftTypeStr = typeToString(leftType);
            auto rightTypeStr = typeToString(rightType);
            if (leftTypeStr != rightTypeStr) {
                if (!Context::isCastable(rightTypeStr, leftTypeStr, context)) {
                    errors.emplace_back("Type mismatch: cannot assign '" + rightTypeStr + "' to '" + leftTypeStr + "'.", stmt->loc, false, false);
                    result = false;
                }
                else {
                    errors.emplace_back("Implicit conversion from '" + rightTypeStr + "' to '" + leftTypeStr + "'.", stmt->loc, true, false);
                }
            }
        }
        return result;
    }

    bool checkStmt(const ReturnStatementPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        auto result = checkExpr(stmt->expr, context, errors);
        if (!result) return result;
        auto type = getExprType(stmt->expr, context, errors).value();
        context.currFuncReturnTypes.emplace_back(type, stmt->loc);
        return result;
    }

    bool checkStmt(const CallExprPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(stmt, context, errors);
        return result;
    }

    bool checkStmt(const DeleteStmtPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(stmt->expr, context, errors);
        // check if expr is pointer
        if (result) {
            auto exprType = getExprType(stmt->expr, context, errors).value();
            if (auto typeExpr = std::get_if<TypeExprPtr>(&exprType)) {
                if (Context::isBuiltInType(typeExpr->get()->type)) {
                    errors.emplace_back("Cannot delete expression of type '" + typeExpr->get()->type + "'.", stmt->loc, false, false);
                    result = false;
                }
            }
            else if (!std::holds_alternative<ArrayExprPtr>(exprType)) {
                errors.emplace_back("Cannot delete expression of type '" + typeToString(exprType) + "'.", stmt->loc, false, false);
                result = false;
            }
        }
        return result;
    }

    bool checkStmt(const FuncParamDecStmtPtr &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result &= checkExpr(stmt->type, context, errors);
        return result;
    }

    bool checkStmt(const StmtPtrVariant &stmt, Context &context, std::vector<ErrorMessage> &errors) {
        return std::visit([&](auto &&stmt) {
            return checkStmt(stmt, context, errors);
        }, stmt);
    }
};