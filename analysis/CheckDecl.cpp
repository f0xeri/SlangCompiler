//
// Created by user on 05.11.2023.
//

#include "Check.hpp"

namespace Slangc::Check {
    bool checkDecl(const DeclPtrVariant &decl, Context &context, std::vector<ErrorMessage> &errors);

    bool checkDecl(const ArrayDecStatementPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        return checkStmt(decl, context, errors);
    }

    bool checkDecl(const ExternFuncDecStatementPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        return checkStmt(decl, context, errors);
    }

    bool checkDecl(const FieldArrayVarDecPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        return result;
    }

    bool checkDecl(const FieldVarDecPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        return result;
    }

    bool checkDecl(const FieldFuncPointerStatementPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        /*if (context.lookup(decl->name)) {
            errors.emplace_back("Variable with name '" + decl->name + "' already exists.", decl->loc, false, false);
            result = false;
        }
        result &= checkExpr(decl->expr, context, errors);
        context.insert(decl->name, decl);
        if (decl->assignExpr.has_value()) {
            result &= checkExpr(decl->assignExpr.value(), context, errors);
            if (result) {
                auto leftType = decl->expr;
                auto rightType = getExprType(decl->assignExpr.value(), context);
                // searching for overloaded function
                if (auto varExpr = std::get_if<VarExprPtr>(&decl->assignExpr.value())) {
                    if (auto func = context.lookupFunc(varExpr->get()->name, decl->expr)) {
                        rightType = std::get<FuncDecStatementPtr>(*func)->expr;
                    }
                }
                if (!compareFuncSignatures(leftType, std::get<FuncExprPtr>(rightType))) {
                    errors.emplace_back("Type mismatch: cannot assign: function signatures do not match.", decl->loc, false, false);
                    result = false;
                }
            }
        }*/
        return result;
    }

    bool checkDecl(const FuncDecStatementPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(decl->expr->type, context, errors);
        if (result) {
            for (const auto &param: decl->expr->params) {
                result &= checkDecl(param, context, errors);
            }
        }
        if (result) {
            auto foundFunc = context.lookupFunc(decl->name, decl->expr);
            if (foundFunc) {
                auto func = std::get<FuncDecStatementPtr>(*foundFunc);
                auto errorMsg = std::string("Function '") + decl->name + "' with the same signature already exists.";
                if (!compareTypes(func->expr->type, decl->expr->type)) {
                    errorMsg += " Functions that differ only in their return type cannot be overloaded.";
                }
                errors.emplace_back(errorMsg, decl->loc, false, false);
                result = false;
            }
            context.insert(decl->name, decl);
            context.enterScope();
            for (const auto &param: decl->expr->params) {
                context.insert(param->name, param);
            }
            result &= checkBlockStmt(decl->block.value(), context, errors);
            for (auto &type : context.currFuncReturnTypes) {
                if (!compareTypes(type.first, decl->expr->type)) {
                    errors.emplace_back("Function '" + decl->name + "' returns incorrect type.", type.second, false, false);
                    result = false;
                }
            }
            context.currFuncReturnTypes.clear();

            context.exitScope();
        }

        return result;
    }

    bool checkDecl(const FuncParamDecStmtPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result &= checkExpr(decl->type, context, errors);
        return result;
    }

    bool checkDecl(const FuncPointerStatementPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        return result;
    }

    bool checkDecl(const MethodDecPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(decl->expr->type, context, errors);
        if (result) {
            for (const auto &param: decl->expr->params) {
                result &= checkDecl(param, context, errors);
            }
        }
        if (result) {
            auto foundFunc = context.lookupFunc(decl->name, decl->expr);
            if (foundFunc) {
                auto func = std::get<FuncDecStatementPtr>(*foundFunc);
                auto errorMsg = std::string("Function '") + decl->name + "' with the same signature already exists.";
                if (!compareTypes(func->expr->type, decl->expr->type)) {
                    errorMsg += " Functions that differ only in their return type cannot be overloaded.";
                }
                errors.emplace_back(errorMsg, decl->loc, false, false);
                result = false;
            }
            context.insert(decl->name, decl);
            context.enterScope();
            for (const auto &param: decl->expr->params) {
                context.insert(param->name, param);
            }
            result &= checkBlockStmt(decl->block.value(), context, errors);
            for (auto &type : context.currFuncReturnTypes) {
                if (!compareTypes(type.first, decl->expr->type)) {
                    errors.emplace_back("Function '" + decl->name + "' returns incorrect type.", type.second, false, false);
                    result = false;
                }
            }
            context.currFuncReturnTypes.clear();
            context.exitScope();
        }
        return result;
    }

    bool checkDecl(const TypeDecStatementPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (context.lookup(decl->name)) {
            errors.emplace_back("Variable, type, or function with name '" + decl->name + "' already exists.", decl->loc, false, false);
            result = false;
        }
        if (decl->parentTypeName.has_value()) {
            if (!context.lookup(decl->parentTypeName.value())) {
                errors.emplace_back("Parent type with name '" + decl->parentTypeName.value() + "' does not exist.", decl->loc, false, false);
                result = false;
            }
        }
        context.insert(decl->name, decl);
        context.enterScope();
        for (const auto &field: decl->fields) {
            result &= checkDecl(field, context, errors);
        }
        for (const auto &method: decl->methods) {
            result &= checkDecl(method, context, errors);
        }
        context.exitScope();
        return result;
    }

    bool checkDecl(const VarDecStatementPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        return result;
    }

    bool checkDecl(const ModuleDeclPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        for (auto &stmt : decl->block->statements) {
            result &= checkStmt(stmt, context, errors);
        }
        return result;
    }

    bool checkDecl(const DeclPtrVariant &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return std::visit([&](const auto &expr) {
            return checkDecl(expr, context, errors);
        }, expr);
    }
}