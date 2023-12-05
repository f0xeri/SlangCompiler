//
// Created by user on 05.11.2023.
//

#include "Check.hpp"

namespace Slangc::Check {
    bool checkDecl(const DeclPtrVariant &decl, Context &context, std::vector<ErrorMessage> &errors);

    bool checkDecl(const ArrayDecStatementPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        return checkStmt(decl, context, errors);
    }

    /*bool checkDecl(const ExternFuncDecStmtPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        return checkStmt(decl, context, errors);
    }*/

    bool checkDecl(const FieldArrayVarDecPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (!checkExpr(decl->expr, context, errors)) result = false;
        auto type = context.symbolTable.lookupType(decl->typeName.type);
        for (int i = 0; i < decl->index; ++i) {
            if (getDeclarationName(type.value()->fields[i]) == decl->name) {
                errors.emplace_back(context.filename, "Field with name '" + decl->name + "' already exists.", decl->loc, false, false);
                result = false;
            }
        }
        context.insert(decl->name, decl);
        return result;
    }

    bool checkDecl(const FieldVarDecPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (!context.symbolTable.lookupType(decl->typeExpr.type) && !Context::isBuiltInType(decl->typeExpr.type)) {
            if (!context.symbolTable.lookupType(context.moduleName + "." + decl->typeExpr.type)) {
                errors.emplace_back(context.filename, "Type '" + decl->typeExpr.type + "' does not exist.", decl->loc, false, false);
                result = false;
            } else {
                // update type name
                decl->typeExpr.type = context.moduleName + "." + decl->typeExpr.type;
            }
        }
        auto type = context.symbolTable.lookupType(decl->typeName.type);
        // check if field with the same name already exists
        // check only previous field names [0; currentFieldIndex)
        for (int i = 0; i < decl->index; ++i) {
            if (getDeclarationName(type.value()->fields[i]) == decl->name) {
                errors.emplace_back(context.filename, "Field with name '" + decl->name + "' already exists.", decl->loc, false, false);
                result = false;
            }
        }
        context.insert(decl->name, decl);
        if (decl->expr.has_value() && result) {
            if (!checkExpr(decl->expr.value(), context, errors)) {
                return false;
            }
            auto exprType = std::get<TypeExprPtr>(getExprType(decl->expr.value(), context, errors).value())->type;
            if (exprType != decl->typeExpr.type) {
                if (!Context::isCastable(exprType, decl->typeExpr.type, context)) {
                    errors.emplace_back(context.filename, "Type mismatch: cannot assign '" + exprType + "' to '" + decl->typeExpr.type + "'.", decl->loc, false, false);
                    result = false;
                }
                else {
                    errors.emplace_back(context.filename, "Implicit conversion from '" + exprType + "' to '" + decl->typeExpr.type + "'.", decl->loc, true, false);
                }
            }
        }
        return result;
    }

    bool checkDecl(const FieldFuncPointerStmtPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        auto type = context.symbolTable.lookupType(decl->typeName.type);
        for (int i = 0; i < decl->index; ++i) {
            if (getDeclarationName(type.value()->fields[i]) == decl->name) {
                errors.emplace_back(context.filename, "Field with name '" + decl->name + "' already exists.", decl->loc, false, false);
                result = false;
            }
        }
        result &= checkExpr(decl->expr, context, errors);
        context.insert(decl->name, decl);
        if (decl->assignExpr.has_value()) {
            result &= checkExpr(decl->assignExpr.value(), context, errors);
            if (result) {
                auto leftType = decl->expr;
                auto rightType = getExprType(decl->assignExpr.value(), context, errors).value();
                // searching for overloaded function
                if (auto varExpr = std::get_if<VarExprPtr>(&decl->assignExpr.value())) {
                    if (auto func = context.symbolTable.lookupFunc(varExpr->get()->name, decl->expr, context)) {
                        rightType = std::get<FuncDecStatementPtr>(*func)->expr;
                    }
                }
                if (!compareFuncSignatures(leftType, std::get<FuncExprPtr>(rightType), context)) {
                    errors.emplace_back(context.filename, "Type mismatch: no matching function found, cannot assign '" + typeToString(rightType) + "' to '" + typeToString(leftType) + "'.", decl->loc, false, false);
                    result = false;
                }
            }
        }
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
            auto foundFunc = context.lookupFuncInScope(decl->name, decl->expr, context, false);
            if (foundFunc) {
                auto func = std::get<FuncDecStatementPtr>(*foundFunc);
                auto errorMsg = std::string("Function '") + decl->name + "' with the same signature already exists.";
                if (!compareTypes(func->expr->type, decl->expr->type, context)) {
                    errorMsg += " Functions that differ only in their return type cannot be overloaded.";
                }
                errors.emplace_back(context.filename, errorMsg, decl->loc, false, false);
                result = false;
            }
            context.insert(decl->name, decl);
            context.enterScope(decl->name + "." + getMangledFuncName(decl->expr));
            for (const auto &param: decl->expr->params) {
                context.insert(param->name, param);
            }
            if (decl->block.has_value()) {
                result &= checkBlockStmt(decl->block.value(), context, errors);
                for (auto &type : context.currFuncReturnTypes) {
                    if (!compareTypes(type.first, decl->expr->type, context)) {
                        if (!Context::isCastable(typeToString(type.first), typeToString(decl->expr->type), context)) {
                            errors.emplace_back(context.filename, "Function '" + decl->name + "' returns incorrect type '" + typeToString(type.first) + "' instead of '" +
                                                typeToString(decl->expr->type) + "'.", type.second, false, false);
                            result = false;
                        }
                        else {
                            errors.emplace_back(context.filename, "Implicit conversion from '" + typeToString(type.first) + "' to '" + typeToString(decl->expr->type) + "'.", decl->loc, true, false);
                        }
                    }
                }
                context.currFuncReturnTypes.clear();
            }
            context.exitScope();
        }

        return result;
    }

    bool checkDecl(const FuncParamDecStmtPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result &= checkExpr(decl->type, context, errors);
        return result;
    }

    bool checkDecl(const FuncPointerStmtPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        return checkStmt(decl, context, errors);
    }

    bool checkDecl(const MethodDecPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(decl->expr->type, context, errors);
        if (result) {
            for (const auto &param: decl->expr->params) {
                result &= checkDecl(param, context, errors);
            }
        }
        auto thisType = context.symbolTable.lookupType(context.currType);
        for (const auto& field : thisType.value()->fields) {
            auto str = context.currType + "." + std::string(getDeclarationName(field));
            if (str == decl->name) {
                errors.emplace_back(context.filename, "Type '" + context.currType + "' already contains definition for '" + decl->name + "'.", decl->loc, false, false);
                result = false;
            }
        }
        if (result) {
            auto foundFunc = context.symbolTable.lookupFunc(decl->name, decl->expr, context);
            if (foundFunc) {
                auto func = std::get<FuncDecStatementPtr>(*foundFunc);
                auto errorMsg = std::string("Function '") + decl->name + "' with the same signature already exists.";
                if (!compareTypes(func->expr->type, decl->expr->type, context)) {
                    errorMsg += " Functions that differ only in their return type cannot be overloaded.";
                }
                errors.emplace_back(context.filename, errorMsg, decl->loc, false, false);
                result = false;
            }
            context.insert(decl->name, decl);
            context.enterScope(decl->name + "." + getMangledFuncName(decl->expr));
            for (const auto &param: decl->expr->params) {
                context.insert(param->name, param);
            }
            result &= checkBlockStmt(decl->block.value(), context, errors);
            for (auto &type : context.currFuncReturnTypes) {
                auto retTypeStr = typeToString(type.first);
                auto declTypeStr = typeToString(decl->expr->type);
                if (retTypeStr != declTypeStr) {
                    if (!Context::isCastable(retTypeStr, declTypeStr, context)) {
                        errors.emplace_back(context.filename, "Function '" + decl->name + "' returns incorrect type '" + retTypeStr + "' instead of '" + declTypeStr + "'.", type.second, false, false);
                        result = false;
                    }
                    else {
                        errors.emplace_back(context.filename, "Implicit conversion from '" + retTypeStr + "' to '" + declTypeStr + "'.", decl->loc, true, false);
                    }
                }
            }
            context.currFuncReturnTypes.clear();
            context.exitScope();
        }
        return result;
    }

    bool checkDecl(const TypeDecStmtPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (context.lookup(decl->name)) {
            errors.emplace_back(context.filename, "Variable, type, or function with name '" + decl->name + "' already exists.", decl->loc, false, false);
            result = false;
        }
        if (decl->parentTypeName.has_value()) {
            if (!context.symbolTable.lookupType(decl->parentTypeName.value())) {
                if (!context.symbolTable.lookupType(context.moduleName + "." + decl->parentTypeName.value())) {
                    errors.emplace_back(context.filename, "Parent type with name '" + decl->parentTypeName.value() + "' does not exist.", decl->loc, false, false);
                    result = false;
                } else {
                    // update type name
                    decl->parentTypeName = context.moduleName + "." + decl->parentTypeName.value();
                }
            }
        }

        context.insert(decl->name, decl);
        context.currType = decl->name;
        context.enterScope(decl->name);
        /*if (decl->vtableRequired) {
            decl->fields.insert(decl->fields.begin(), createDecl<FieldVarDecNode>(decl->loc, "", decl->name, true, 0, "_vtable", std::nullopt));
        }*/
        for (const auto &field: decl->fields) {
            result &= checkDecl(field, context, errors);
            if (decl->vtableRequired && decl->parentTypeName == "Object") {
                if (auto fieldVar = std::get_if<FieldVarDecPtr>(&field)) fieldVar->get()->index++;
                else if (auto fieldArrayVar = std::get_if<FieldArrayVarDecPtr>(&field)) fieldArrayVar->get()->index++;
                else if (auto fieldFuncPointer = std::get_if<FieldFuncPointerStmtPtr>(&field)) fieldFuncPointer->get()->index++;
            }
        }
        // we don't need field names in scope after checking them
        context.clearCurrentScope();
        // check return type of method to make sure it has mangled name
        // we should do it before fully checking methods
        // TODO: probably we can find better solution without two passes
        // example:
        //    module sample
        //        public method getA(A a)(): A      // checked, method returns "sample.A" type
        //            return a.getAp();             // error, returns "A" type, not "sample.A"
        //        end getA;
        //
        //        private method getAp(A a)(): A    // not checked yet, method returns "A" type
        //            return a;
        //        end getAp;
        // check of return type fixes this possible problem
        for (const auto &method: decl->methods) {
            checkExpr(method->expr->type, context, errors);
        }
        for (const auto &method: decl->methods) {
            result &= checkDecl(method, context, errors);
        }
        context.exitScope();
        context.currType = "";
        return result;
    }

    bool checkDecl(const VarDecStmtPtr &decl, Context &context, std::vector<ErrorMessage> &errors) {
        return checkStmt(decl, context, errors);
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