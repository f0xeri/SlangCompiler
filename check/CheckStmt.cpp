//
// Created by user on 02.11.2023.
//

#include <iostream>
#include "Check.hpp"

namespace Slangc::Check {
    bool checkStmt(const StmtPtrVariant &stmt, Context &context);
    bool checkBlockStmt(const BlockStmtPtr &stmt, Context &context) {
        bool result = true;
        for (const auto &statement: stmt->statements) {
            result &= checkStmt(statement, context);
        }
        return result;
    }

    bool checkStmt(const VarDecStmtPtr &stmt, Context &context) {
        bool result = true;
        if (!context.symbolTable.lookupType(stmt->typeExpr.type) && !Context::isBuiltInType(stmt->typeExpr.type)) {
            if (!context.symbolTable.lookupType(context.moduleName + "." + stmt->typeExpr.type)) {
                SLANGC_LOG(context.filename, "Type '" + stmt->typeExpr.type + "' does not exist.", stmt->loc, LogLevel::Error, false);
                result = false;
            } else {
                // update type name
                stmt->typeExpr.type = context.moduleName + "." + stmt->typeExpr.type;
            }
        }
        if (context.lookup(stmt->name)) {
            SLANGC_LOG(context.filename, "Variable with name '" + stmt->name + "' already exists.", stmt->loc, LogLevel::Error, false);
            result = false;
        }
        context.insert(stmt->name, stmt);
        if (stmt->expr.has_value() && result) {
            auto leftType = getDeclType(stmt, context).value();
            if (auto nilExpr = std::get_if<NilExprPtr>(&stmt->expr.value())) {
                nilExpr->get()->type = leftType;
            }
            if (!checkExpr(stmt->expr.value(), context)) {
                return false;
            }
            auto exprType = typeToString(getExprType(stmt->expr.value(), context).value());
            if (exprType != stmt->typeExpr.type) {
                bool isCastable = Context::isCastable(exprType, stmt->typeExpr.type, context);
                if (!isCastable || stmt->isGlobal) {
                    if (isCastable && stmt->isGlobal) {
                        SLANGC_LOG(context.filename, "Type mismatch: cannot convert '" + exprType + "' to '" + stmt->typeExpr.type + "' in global scope.", stmt->loc, LogLevel::Error, false);
                    }
                    else {
                        SLANGC_LOG(context.filename, "Type mismatch: cannot assign '" + exprType + "' to '" + stmt->typeExpr.type + "'.", stmt->loc, LogLevel::Error, false);
                    }
                    result = false;
                }
                else {
                    SLANGC_LOG(context.filename, "Implicit conversion from '" + exprType + "' to '" + stmt->typeExpr.type + "'.", stmt->loc, LogLevel::Warn, false);
                }
            }
            if (!isConstExpr(stmt->expr.value()) && stmt->isGlobal) {
                SLANGC_LOG(context.filename, "Global variable '" + stmt->name + "' can be initialized only with constant expression.", stmt->loc, LogLevel::Error, false);
                result = false;
            }
        }
        return result;
    }

    bool checkStmt(const ArrayDecStatementPtr &stmt, Context &context) {
        bool result = true;
        if (!checkExpr(stmt->expr, context)) result = false;
        if (context.lookup(stmt->name)) {
            SLANGC_LOG(context.filename, "Variable with name '" + stmt->name + "' already exists.", stmt->loc, LogLevel::Error, false);
            result = false;
        }
        context.insert(stmt->name, stmt);
        if (stmt->assignExpr.has_value())
            result &= checkExpr(stmt->assignExpr.value(), context);

        if (stmt->assignExpr.has_value() && result) {
            auto leftType = getDeclType(stmt, context).value();
            if (auto nilExpr = std::get_if<NilExprPtr>(&stmt->assignExpr.value())) {
                nilExpr->get()->type = leftType;
            }
            if (!checkExpr(stmt->assignExpr.value(), context)) {
                return false;
            }
            auto exprType = typeToString(getExprType(stmt->assignExpr.value(), context).value());
            auto arrType = typeToString(stmt->getType(context).value());
            if (exprType != arrType) {
                SLANGC_LOG(context.filename, "Type mismatch: cannot assign '" + exprType + "' to '" + arrType + "'.", stmt->loc, LogLevel::Error, false);
                result = false;
            }
            if (!isConstExpr(stmt->assignExpr.value()) && stmt->isGlobal) {
                SLANGC_LOG(context.filename, "Global variable '" + stmt->name + "' can be initialized only with constant expression.", stmt->loc, LogLevel::Error, false);
                result = false;
            }
        }
        return result;
    }

    bool checkStmt(const FuncPointerStmtPtr &stmt, Context &context) {
        bool result = true;
        if (context.lookup(stmt->name)) {
            SLANGC_LOG(context.filename, "Variable with name '" + stmt->name + "' already exists.", stmt->loc, LogLevel::Error, false);
            result = false;
        }
        result &= checkExpr(stmt->expr, context);
        context.insert(stmt->name, stmt);
        if (stmt->assignExpr.has_value()) {
            result &= checkExpr(stmt->assignExpr.value(), context);
            if (result) {
                auto leftType = stmt->expr;

                if (auto nilExpr = std::get_if<NilExprPtr>(&stmt->assignExpr.value())) {
                    nilExpr->get()->type = leftType;
                    return result;
                }
                auto rightType = getExprType(stmt->assignExpr.value(), context).value();

                // searching for overloaded function
                if (auto varExpr = std::get_if<VarExprPtr>(&stmt->assignExpr.value())) {
                    if (auto func = context.symbolTable.lookupFunc(varExpr->get()->name, stmt->expr, context)) {
                        rightType = std::get<FuncDecStatementPtr>(*func)->expr;
                    }
                }
                if (!compareFuncSignatures(leftType, std::get<FuncExprPtr>(rightType), context)) {
                    SLANGC_LOG(context.filename, "Type mismatch: no matching function found, cannot assign '" + typeToString(rightType) + "' to '" + typeToString(leftType) + "'.", stmt->loc, LogLevel::Error, false);
                    result = false;
                }
            }
        }
        return result;
    }

    bool checkStmt(const IfStatementPtr &stmt, Context &context) {
        bool result = true;
        result &= checkExpr(stmt->condition, context);
        if (result) {
            auto condType = typeToString(getExprType(stmt->condition, context).value());
            if (condType != getBuiltInTypeName(BuiltInType::Bool)) {
                if (!Context::isCastable(condType, getBuiltInTypeName(BuiltInType::Bool), context)) {
                    SLANGC_LOG(context.filename, "Type mismatch: cannot use '" + condType + "' as condition.", stmt->loc, LogLevel::Error, false);
                    result = false;
                }
                else {
                    SLANGC_LOG(context.filename, "Implicit conversion from '" + condType + "' to 'boolean'.", stmt->loc, LogLevel::Warn, false);
                }
            }
        }
        context.enterScope("if#" + std::to_string(stmt->loc.line) + ":" + std::to_string(stmt->loc.column));
        result &= checkBlockStmt(stmt->trueBlock, context);
        context.exitScope();
        for (const auto &elseIf: stmt->elseIfNodes) {
            result &= checkStmt(elseIf, context);
        }
        if (stmt->falseBlock.has_value()) {
            context.enterScope("else#" + std::to_string(stmt->loc.line) + ":" + std::to_string(stmt->loc.column));
            result &= checkBlockStmt(stmt->falseBlock.value(), context);
            context.exitScope();
        }
        return result;
    }

    bool checkStmt(const ElseIfStatementPtr &stmt, Context &context) {
        bool result = true;
        result &= checkExpr(stmt->condition, context);
        if (result) {
            auto condType = typeToString(getExprType(stmt->condition, context).value());
            if (condType != getBuiltInTypeName(BuiltInType::Bool)) {
                if (!Context::isCastable(condType, getBuiltInTypeName(BuiltInType::Bool), context)) {
                    SLANGC_LOG(context.filename, "Type mismatch: cannot use '" + condType + "' as condition.", stmt->loc, LogLevel::Error, false);
                    result = false;
                }
                else {
                    SLANGC_LOG(context.filename, "Implicit conversion from '" + condType + "' to 'boolean'.", stmt->loc, LogLevel::Warn, false);
                }
            }
        }
        context.enterScope("else-if#" + std::to_string(stmt->loc.line) + ":" + std::to_string(stmt->loc.column));
        result &= checkBlockStmt(stmt->trueBlock, context);
        context.exitScope();
        return result;
    }

    bool checkStmt(const WhileStatementPtr &stmt, Context &context) {
        bool result = true;
        result &= checkExpr(stmt->condition, context);
        if (result) {
            auto condType = typeToString(getExprType(stmt->condition, context).value());
            if (condType != getBuiltInTypeName(BuiltInType::Bool)) {
                if (!Context::isCastable(condType, getBuiltInTypeName(BuiltInType::Bool), context)) {
                    SLANGC_LOG(context.filename, "Type mismatch: cannot use '" + condType + "' as condition.", stmt->loc, LogLevel::Error, false);
                    result = false;
                }
                else {
                    SLANGC_LOG(context.filename, "Implicit conversion from '" + condType + "' to 'boolean'.", stmt->loc, LogLevel::Warn, false);
                }
            }
        }
        context.enterScope("while#" + std::to_string(stmt->loc.line) + ":" + std::to_string(stmt->loc.column));
        result &= checkBlockStmt(stmt->block, context);
        context.exitScope();
        return result;
    }

    bool checkStmt(const OutputStatementPtr &stmt, Context &context) {
        bool result = true;
        result &= checkExpr(stmt->expr, context);
        return result;
    }

    bool checkStmt(const InputStatementPtr &stmt, Context &context) {
        bool result = true;
        result &= checkExpr(stmt->expr, context);
        if (result) {
            result &= Context::isBuiltInType(typeToString(getExprType(stmt->expr, context).value()));
        }
        return result;
    }

    bool checkStmt(const AssignExprPtr &stmt, Context &context) {
        bool result = true;
        result &= checkExpr(stmt->left, context);
        result &= checkExpr(stmt->right, context);
        if (result) {
            auto leftType = getExprType(stmt->left, context).value();
            auto rightType = getExprType(stmt->right, context).value();
            // check if left is func type
            if (auto left = std::get_if<FuncExprPtr>(&leftType)) {
                if (auto varExpr = std::get_if<VarExprPtr>(&stmt->right)) {
                    if (auto func = context.symbolTable.lookupFunc(varExpr->get()->name, *left, context)) {
                        rightType = std::get<FuncDecStatementPtr>(*func)->expr;
                    }
                }
            }
            // we don't need to compare function signatures here because we can just check their text representation
            auto leftTypeStr = typeToString(leftType);
            auto rightTypeStr = typeToString(rightType);
            if (auto nilExpr = std::get_if<NilExprPtr>(&stmt->right)) {
                nilExpr->get()->type = getExprType(stmt->left, context).value();
                rightTypeStr = typeToString(nilExpr->get()->type.value());
            }
            if (leftTypeStr != rightTypeStr) {
                if (!Context::isCastable(rightTypeStr, leftTypeStr, context)) {
                    SLANGC_LOG(context.filename, "Type mismatch: cannot assign '" + rightTypeStr + "' to '" + leftTypeStr + "'.", stmt->loc, LogLevel::Error, false);
                    result = false;
                }
                else {
                    SLANGC_LOG(context.filename, "Implicit conversion from '" + rightTypeStr + "' to '" + leftTypeStr + "'.", stmt->loc, LogLevel::Warn, false);
                }
            }
        }
        return result;
    }

    bool checkStmt(const ReturnStatementPtr &stmt, Context &context) {
        bool result = true;
        if (!(std::holds_alternative<TypeExprPtr>(stmt->expr) && std::get<TypeExprPtr>(stmt->expr)->type == getBuiltInTypeName(BuiltInType::Void))) {
            result = checkExpr(stmt->expr, context);
        }
        if (!result) return result;
        auto type = getExprType(stmt->expr, context).value();
        context.currFuncReturnTypes.emplace_back(type, stmt->loc);
        return result;
    }

    bool checkStmt(const CallExprPtr &stmt, Context &context) {
        bool result = true;
        result = checkExpr(stmt, context);
        return result;
    }

    bool checkStmt(const DeleteStmtPtr &stmt, Context &context) {
        bool result = true;
        result = checkExpr(stmt->expr, context);
        // check if expr is pointer
        if (result) {
            auto exprType = getExprType(stmt->expr, context).value();
            if (auto typeExpr = std::get_if<TypeExprPtr>(&exprType)) {
                if (Context::isBuiltInType(typeExpr->get()->type)) {
                    SLANGC_LOG(context.filename, "Cannot delete expression of type '" + typeExpr->get()->type + "'.", stmt->loc, LogLevel::Error, false);
                    result = false;
                }
            }
            else if (!std::holds_alternative<ArrayExprPtr>(exprType)) {
                SLANGC_LOG(context.filename, "Cannot delete expression of type '" + typeToString(exprType) + "'.", stmt->loc, LogLevel::Error, false);
                result = false;
            }
        }
        return result;
    }

    bool checkStmt(const FuncParamDecStmtPtr &stmt, Context &context) {
        bool result = true;
        result &= checkExpr(stmt->type, context);
        return result;
    }

    bool checkStmt(const StmtPtrVariant &stmt, Context &context) {
        return std::visit([&](auto &&stmt) {
            return checkStmt(stmt, context);
        }, stmt);
    }
};