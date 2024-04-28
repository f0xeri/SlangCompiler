//
// Created by user on 05.11.2023.
//

#include <iostream>
#include "Check.hpp"

namespace Slangc::Check {
    bool checkDecl(const DeclPtrVariant &decl, Context &context);

    bool checkDecl(const ArrayDecStatementPtr &decl, Context &context) {
        return checkStmt(decl, context);
    }

    /*bool checkDecl(const ExternFuncDecStmtPtr &decl, Context &context) {
        return checkStmt(decl, context);
    }*/

    bool checkDecl(const FieldArrayVarDecPtr &decl, Context &context) {
        bool result = true;
        if (!checkExpr(decl->expr, context)) result = false;
        auto type = context.symbolTable.lookupType(decl->typeName.type);
        for (int i = 0; i < decl->index; ++i) {
            if (getDeclName(type.value()->fields[i]) == decl->name) {
                SLANGC_LOG(context.filename, "Field with name '" + decl->name + "' already exists.", decl->loc, LogLevel::Error, false);
                result = false;
            }
        }
        context.insert(decl->name, decl);

        if (decl->assignExpr.has_value())
            result &= checkExpr(decl->assignExpr.value(), context);

        if (decl->assignExpr.has_value() && result) {
            auto leftType = getDeclType(decl, context).value();
            if (auto nilExpr = std::get_if<NilExprPtr>(&decl->assignExpr.value())) {
                nilExpr->get()->type = leftType;
            }
            if (!checkExpr(decl->assignExpr.value(), context)) {
                return false;
            }
            auto exprType = typeToString(getExprType(decl->assignExpr.value(), context).value());
            auto arrType = typeToString(decl->getType(context).value());
            if (exprType != arrType) {
                SLANGC_LOG(context.filename, "Type mismatch: cannot assign '" + exprType + "' to '" + arrType + "'.", decl->loc, LogLevel::Error, false);
                result = false;
            }
        }

        return result;
    }

    bool checkDecl(const FieldVarDecPtr &decl, Context &context) {
        bool result = true;
        if (!context.symbolTable.lookupType(decl->typeExpr.type) && !Context::isBuiltInType(decl->typeExpr.type)) {
            if (!context.symbolTable.lookupType(context.moduleName + "." + decl->typeExpr.type)) {
                SLANGC_LOG(context.filename, "Type '" + decl->typeExpr.type + "' does not exist.", decl->loc, LogLevel::Error, false);
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
            if (getDeclName(type.value()->fields[i]) == decl->name) {
                SLANGC_LOG(context.filename, "Field with name '" + decl->name + "' already exists.", decl->loc, LogLevel::Error, false);
                result = false;
            }
        }
        context.insert(decl->name, decl);
        if (decl->expr.has_value() && result) {
            auto leftType = getDeclType(decl, context).value();
            if (auto nilExpr = std::get_if<NilExprPtr>(&decl->expr.value())) {
                nilExpr->get()->type = leftType;
            }
            if (!checkExpr(decl->expr.value(), context)) {
                return false;
            }
            // TODO: check why result is not TypeExprPtr
            // auto exprType = std::get<TypeExprPtr>(getExprType(decl->expr.value(), context).value())->type;
            auto exprType = typeToString(getExprType(decl->expr.value(), context).value());
            if (exprType != decl->typeExpr.type) {
                if (!Context::isCastable(exprType, decl->typeExpr.type, context)) {
                    SLANGC_LOG(context.filename, "Type mismatch: cannot assign '" + exprType + "' to '" + decl->typeExpr.type + "'.", decl->loc, LogLevel::Error, false);
                    result = false;
                }
                else {
                    SLANGC_LOG(context.filename, "Implicit conversion from '" + exprType + "' to '" + decl->typeExpr.type + "'.", decl->loc, LogLevel::Warn, false);
                }
            }
        }
        return result;
    }

    bool checkDecl(const FieldFuncPointerStmtPtr &decl, Context &context) {
        bool result = true;
        auto type = context.symbolTable.lookupType(decl->typeName.type);
        for (int i = 0; i < decl->index; ++i) {
            if (getDeclName(type.value()->fields[i]) == decl->name) {
                SLANGC_LOG(context.filename, "Field with name '" + decl->name + "' already exists.", decl->loc, LogLevel::Error, false);
                result = false;
            }
        }
        result &= checkExpr(decl->expr, context);
        context.insert(decl->name, decl);
        if (decl->assignExpr.has_value()) {
            result &= checkExpr(decl->assignExpr.value(), context);
            if (result) {
                auto leftType = decl->expr;

                if (auto nilExpr = std::get_if<NilExprPtr>(&decl->assignExpr.value())) {
                    nilExpr->get()->type = leftType;
                    return result;
                }
                auto rightType = getExprType(decl->assignExpr.value(), context).value();

                // searching for overloaded function
                if (auto varExpr = std::get_if<VarExprPtr>(&decl->assignExpr.value())) {
                    if (auto func = context.symbolTable.lookupFunc(varExpr->get()->name, decl->expr, context)) {
                        rightType = std::get<FuncDecStatementPtr>(*func)->expr;
                    }
                }
                if (!compareFuncSignatures(leftType, std::get<FuncExprPtr>(rightType), context)) {
                    SLANGC_LOG(context.filename, "Type mismatch: no matching function found, cannot assign '" + typeToString(rightType) + "' to '" + typeToString(leftType) + "'.", decl->loc, LogLevel::Error, false);
                    result = false;
                }
            }
        }
        return result;
    }

    bool checkDecl(const FuncDecStatementPtr &decl, Context &context) {
        bool result = true;
        result = checkExpr(decl->expr->type, context);
        if (result) {
            for (const auto &param: decl->expr->params) {
                result &= checkDecl(param, context);
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
                SLANGC_LOG(context.filename, errorMsg, decl->loc, LogLevel::Error, false);
                result = false;
            }
            context.insert(decl->name, decl);
            context.enterScope(decl->isExtern ? decl->name : decl->name + "." + getMangledFuncName(decl->expr));
            for (const auto &param: decl->expr->params) {
                context.insert(param->name, param);
            }
            if (decl->block.has_value()) {
                result &= checkBlockStmt(decl->block.value(), context);
                for (auto &type : context.currFuncReturnTypes) {
                    if (!compareTypes(type.first, decl->expr->type, context)) {
                        if (!Context::isCastable(typeToString(type.first), typeToString(decl->expr->type), context)) {
                            SLANGC_LOG(context.filename, "Function '" + decl->name + "' returns incorrect type '" + typeToString(type.first) + "' instead of '" +
                                                typeToString(decl->expr->type) + "'.", type.second, LogLevel::Error, false);
                            result = false;
                        }
                        else {
                            SLANGC_LOG(context.filename, "Implicit conversion from '" + typeToString(type.first) + "' to '" + typeToString(decl->expr->type) + "'.", decl->loc, LogLevel::Error, false);
                        }
                    }
                }
                context.currFuncReturnTypes.clear();
            }
            context.exitScope();
        }

        return result;
    }

    bool checkDecl(const FuncParamDecStmtPtr &decl, Context &context) {
        bool result = true;
        result &= checkExpr(decl->type, context);
        return result;
    }

    bool checkDecl(const FuncPointerStmtPtr &decl, Context &context) {
        return checkStmt(decl, context);
    }

    bool checkDecl(const MethodDecPtr &decl, Context &context) {
        bool result = true;
        result = checkExpr(decl->expr->type, context);
        if (result) {
            for (const auto &param: decl->expr->params) {
                result &= checkDecl(param, context);
            }
        }
        auto thisType = context.symbolTable.lookupType(context.currType);
        for (const auto& field : thisType.value()->fields) {
            auto str = context.currType + "." + std::string(getDeclName(field));
            if (str == decl->name) {
                SLANGC_LOG(context.filename, "Type '" + context.currType + "' already contains definition for '" + decl->name + "'.", decl->loc, LogLevel::Error, false);
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
                SLANGC_LOG(context.filename, errorMsg, decl->loc, LogLevel::Error, false);
                result = false;
            }
            context.insert(decl->name, decl);
            context.enterScope(decl->name + "." + getMangledFuncName(decl->expr));
            for (const auto &param: decl->expr->params) {
                context.insert(param->name, param);
            }
            result &= checkBlockStmt(decl->block.value(), context);
            for (auto &type : context.currFuncReturnTypes) {
                auto retTypeStr = typeToString(type.first);
                auto declTypeStr = typeToString(decl->expr->type);
                if (retTypeStr != declTypeStr) {
                    if (!Context::isCastable(retTypeStr, declTypeStr, context)) {
                        SLANGC_LOG(context.filename, "Function '" + decl->name + "' returns incorrect type '" + retTypeStr + "' instead of '" + declTypeStr + "'.", type.second, LogLevel::Error, false);
                        result = false;
                    }
                    else {
                        SLANGC_LOG(context.filename, "Implicit conversion from '" + retTypeStr + "' to '" + declTypeStr + "'.", decl->loc, LogLevel::Warn, false);
                    }
                }
            }
            context.currFuncReturnTypes.clear();
            context.exitScope();
        }
        return result;
    }

    bool checkDecl(const TypeDecStmtPtr &decl, Context &context) {
        bool result = true;
        if (context.lookup(decl->name)) {
            SLANGC_LOG(context.filename, "Variable, type, or function with name '" + decl->name + "' already exists.", decl->loc, LogLevel::Error, false);
            result = false;
        }
        if (decl->parentTypeName.has_value()) {
            if (!context.symbolTable.lookupType(decl->parentTypeName.value())) {
                if (!context.symbolTable.lookupType(context.moduleName + "." + decl->parentTypeName.value())) {
                    SLANGC_LOG(context.filename, "Parent type with name '" + decl->parentTypeName.value() + "' does not exist.", decl->loc, LogLevel::Error, false);
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
            result &= checkDecl(field, context);
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
            checkExpr(method->expr->type, context);
        }
        for (const auto &method: decl->methods) {
            result &= checkDecl(method, context);
        }
        context.exitScope();
        context.currType = "";
        return result;
    }

    bool checkDecl(const VarDecStmtPtr &decl, Context &context) {
        return checkStmt(decl, context);
    }

    bool checkDecl(const ModuleDeclPtr &decl, Context &context) {
        bool result = true;
        for (auto &stmt : decl->block->statements) {
            result &= checkStmt(stmt, context);
        }
        return result;
    }

    bool checkDecl(const DeclPtrVariant &expr, Context &context) {
        return std::visit([&](const auto &expr) {
            return checkDecl(expr, context);
        }, expr);
    }
}