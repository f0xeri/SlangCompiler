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
        if (result) {
            auto leftType = typeToString(getExprType(expr->left, context, errors).value());
            auto rightType = typeToString(getExprType(expr->right, context, errors).value());
            // TODO: check if conversion is possible
            if (leftType != rightType) {
                if (!Context::isCastable(rightType, leftType, context)) {
                    errors.emplace_back(context.filename, std::string("Type mismatch: cannot apply operator '") + "' to '" + leftType + "' and '" + rightType + "'.", expr->loc, false, false);
                    result = false;
                }
                else {
                    errors.emplace_back(context.filename, "Implicit conversion from '" + rightType + "' to '" + leftType + "'.", expr->loc, true, false);
                }
            }
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
            else if (std::ranges::find(imports, expr->name) == imports.end()) {
                errors.emplace_back(context.filename, "Variable, type, or function with name '" + expr->name + "' does not exist.", expr->loc, false, false);
                result = false;
            }
        }
        return result;
    }

    bool checkExpr(const IndexExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(expr->expr, context, errors);
        result &= checkExpr(expr->indexExpr, context, errors);
        if (!result) return false;
        auto exprType = getExprType(expr->expr, context, errors);
        auto indexType = getExprType(expr->indexExpr, context, errors);

        if (!exprType.has_value() || !std::holds_alternative<ArrayExprPtr>(exprType.value())) {
            // this code is needed simply to get final type of the array in order to display it in error message
            // two loops is too much for just an error, but IDK how to do it better
            auto ind = std::get_if<IndexExprPtr>(&expr->expr);
            std::optional<ExprPtrVariant> type = exprType;
            std::optional<ExprPtrVariant> typeOpt;
            while (ind) {
                typeOpt = getExprType(ind->get()->expr, context, errors);
                ind = std::get_if<IndexExprPtr>(&ind->get()->expr);
                if (typeOpt) type = typeOpt.value();
                else break;
            }
            while (type.has_value() && std::holds_alternative<ArrayExprPtr>(type.value())) {
                typeOpt = std::get<ArrayExprPtr>(type.value())->type;
                if (typeOpt) type = typeOpt.value();
                else break;
            }
            if (type.has_value())
                errors.emplace_back(context.filename, "Cannot apply indexing with [] to an expression of type '" + typeToString(type.value()) + "'.",expr->loc, false, false);
            else
                errors.emplace_back(context.filename, "Cannot apply indexing with [] to an expression of unknown type.", expr->loc, false,false);
            return false;
        }
        if (indexType.has_value()) {
            auto typeStr = typeToString(indexType.value());
            if (typeStr != "integer") {
                errors.emplace_back(context.filename, "Index must be of type 'integer, not '" + typeStr + "'.", expr->loc, false, false);
                result = false;
            }
        }
        return result;
    }

    bool checkExpr(const CallExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(expr->expr, context, errors);
        for (const auto &arg: expr->args) {
            result &= checkExpr(arg, context, errors);
        }
        if (!result) return false;
        std::optional<DeclPtrVariant> func = std::nullopt;
        std::optional<IndexExprPtr> funcPtrsArrIndex = std::nullopt;
        std::optional<CallExprPtr> funcCall = std::nullopt;
        bool overloaded = false;

        // create func expr based on arg types
        std::vector<FuncParamDecStmtPtr> params;
        params.reserve(expr->args.size());
        auto zeroLoc = SourceLoc{0, 0};
        for (const auto &arg: expr->args) {
            params.push_back(create<FuncParamDecStatementNode>(zeroLoc, "", None, getExprType(arg, context, errors).value()));
        }
        auto funcExpr = create<FuncExprNode>(zeroLoc, create<TypeExprNode>(zeroLoc, "void"), params);

        auto type = getExprType(expr->expr, context, errors);
        // check is callable
        if (auto access = std::get_if<AccessExprPtr>(&expr->expr)) {
            auto accessType = getExprType(access->get()->expr, context, errors);
            std::optional<TypeDecStmtPtr> typeDecl = std::nullopt;
            if (std::holds_alternative<TypeExprPtr>(accessType.value())) {
                typeDecl = context.symbolTable.lookupType(std::get<TypeExprPtr>(accessType.value())->type);
                funcExpr->params.insert(funcExpr->params.begin(), create<FuncParamDecStatementNode>(zeroLoc, "", Out, std::get<TypeExprPtr>(accessType.value())));
            }
            while (typeDecl.has_value()) {
                func = selectBestOverload(typeDecl.value(), typeDecl.value()->name + "." + access->get()->name, funcExpr, false, false, context);
                if (!func && typeDecl.value()->parentTypeName.has_value()) {
                    typeDecl = context.symbolTable.lookupType(typeDecl.value()->parentTypeName.value());
                }
                else {
                    overloaded = true;
                    if (func) {
                        expr->foundFunc = func;
                        expr->funcType = std::get<MethodDecPtr>(*func)->expr;
                    }
                    break;
                }
            }
        }
        else if (auto var = std::get_if<VarExprPtr>(&expr->expr)) {
            func = selectBestOverload(var->get()->name, funcExpr, false, false, context);
            expr->foundFunc = func;
            if (func) expr->funcType = std::get<FuncDecStatementPtr>(*func)->expr;
            overloaded = true;
            if (!func) {
                auto funcPointer = context.lookup(var->get()->name);
                if (!funcPointer) {
                    funcPointer = context.lookup(context.moduleName + "." + var->get()->name);
                    var->get()->name = context.moduleName + "." + var->get()->name;
                }
                if (funcPointer) {
                    if (std::holds_alternative<FuncPointerStmtPtr>(*funcPointer)) {
                        overloaded = true;
                        if (compareFuncSignatures(std::get<FuncPointerStmtPtr>(*funcPointer)->expr, funcExpr, context, false, true)) {
                            func = *funcPointer;
                            expr->funcType = std::get<FuncPointerStmtPtr>(*funcPointer)->expr;
                        }
                    }
                    else if (std::holds_alternative<FieldFuncPointerStmtPtr>(*funcPointer)) {
                        overloaded = true;
                        if (compareFuncSignatures(std::get<FieldFuncPointerStmtPtr>(*funcPointer)->expr, funcExpr, context, false, true)) {
                            func = *funcPointer;
                            expr->funcType = std::get<FieldFuncPointerStmtPtr>(*funcPointer)->expr;
                        }
                    }
                    else if (std::holds_alternative<FuncParamDecStmtPtr>(*funcPointer)) {
                        auto param = std::get<FuncParamDecStmtPtr>(*funcPointer);
                        if (std::holds_alternative<FuncExprPtr>(param->type)) {
                            overloaded = true;
                            if (compareFuncSignatures(std::get<FuncExprPtr>(param->type), funcExpr, context, false, true)) {
                                func = *funcPointer;
                                expr->funcType = std::get<FuncExprPtr>(param->type);
                            }
                        }
                    }
                }
            }
        }
        else if (auto index = std::get_if<IndexExprPtr>(&expr->expr)) {
            auto indexType = getExprType(*index, context, errors);
            if (indexType.has_value() && std::holds_alternative<FuncExprPtr>(indexType.value())) {
                overloaded = true;
                if (compareFuncSignatures(std::get<FuncExprPtr>(indexType.value()), funcExpr, context, false, true)) {
                    funcPtrsArrIndex = *index;
                    expr->funcType = std::get<FuncExprPtr>(indexType.value());
                }
            }
        }
        else if (auto call = std::get_if<CallExprPtr>(&expr->expr)) {
            auto callType = getExprType(*call, context, errors);
            if (callType.has_value() && std::holds_alternative<FuncExprPtr>(callType.value())) {
                overloaded = true;
                if (compareFuncSignatures(std::get<FuncExprPtr>(callType.value()), funcExpr, context, false, true)) {
                    funcCall = *call;
                    expr->funcType = std::get<FuncExprPtr>(callType.value());
                }
            }
        }
        if (!func && !funcPtrsArrIndex && !funcCall) {
            errors.emplace_back(context.filename, overloaded ? "No matching function for call." : "Expression is not callable.", expr->loc, false, false);
            result = false;
        }
        if (expr->funcType.has_value()) {
            for (size_t i = 0; i < expr->args.size(); ++i) {
                if (auto nilExpr = std::get_if<NilExprPtr>(&expr->args[i])) {
                    nilExpr->get()->type = expr->funcType->get()->params[i]->type;
                }
            }
        }
        return result;
    }

    bool checkExpr(const AccessExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(expr->expr, context, errors);
        if (!result) return result;
        auto exprType = getExprType(expr->expr, context, errors);
        std::optional<TypeDecStmtPtr> typeOpt = std::nullopt;
        if (!exprType.has_value()) {
            errors.emplace_back(context.filename, "Failed to get type of expression.", expr->loc, false, false);
            return false;
        }
        if (std::holds_alternative<TypeExprPtr>(exprType.value()) && !Context::isBuiltInType(std::get<TypeExprPtr>(exprType.value())->type)) {
            typeOpt = context.symbolTable.lookupType(std::get<TypeExprPtr>(exprType.value())->type);
        }
        else {
            errors.emplace_back(context.filename, "Type '" + typeToString(exprType.value()) + "' is not accessible.", expr->loc, false, false);
            result = false;
        }
        auto found = false;
        size_t index = 0;
        std::string accessedType;
        while (typeOpt.has_value()) {
            auto type = typeOpt.value();
            for (const auto &field: type->fields) {
                if (auto fieldVar = std::get_if<FieldVarDecPtr>(&field)) {
                    if ((*fieldVar)->name == expr->name) {
                        if ((*fieldVar)->isPrivate && !Context::isPrivateAccessible(context.currType, type->name, context)) {
                            errors.emplace_back(context.filename, "Cannot access private field '" + expr->name + "' of type '" + type->name + "' from '" + context.currType + "'.", expr->loc, false, false);
                            result = false;
                        }
                        // found = true even if we got error, because we want to suppress final not found error
                        found = true;
                        index = (*fieldVar)->index;
                        accessedType = (*fieldVar)->typeName.type;
                    }
                } else if (const auto &fieldArrayVar = std::get_if<FieldArrayVarDecPtr>(&field)) {
                    if ((*fieldArrayVar)->name == expr->name) {
                        if ((*fieldArrayVar)->isPrivate && !Context::isPrivateAccessible(context.currType, type->name, context)) {
                            errors.emplace_back(context.filename, "Cannot access private field '" + expr->name + "' of type '" + type->name + "' from '" + context.currType + "'.", expr->loc, false, false);
                            result = false;
                        }
                        found = true;
                        index = (*fieldArrayVar)->index;
                        accessedType = (*fieldArrayVar)->typeName.type;
                    }
                } else if (auto fieldFuncPointer = std::get_if<FieldFuncPointerStmtPtr>(&field)) {
                    if ((*fieldFuncPointer)->name == expr->name) {
                        if ((*fieldFuncPointer)->isPrivate && !Context::isPrivateAccessible(context.currType, type->name, context)) {
                            errors.emplace_back(context.filename, "Cannot access private field '" + expr->name + "' of type '" + type->name + "' from '" + context.currType + "'.", expr->loc, false, false);
                            result = false;
                        }
                        found = true;
                        index = (*fieldFuncPointer)->index;
                        accessedType = (*fieldFuncPointer)->typeName.type;
                    }
                }

                if (found) break;
            }
            if (!found) {
                for (const auto &method: type->methods) {
                    if ((method->name == type->name + "." + expr->name)) {
                        if (method->isPrivate && !Context::isPrivateAccessible(context.currType, type->name, context)) {
                            errors.emplace_back(context.filename, "Cannot access private method '" + expr->name + "' of type '" + type->name + "' from '" + context.currType + "'.", expr->loc, false, false);
                            result = false;
                        }
                        found = true;
                        break;
                    }
                }
            }
            if (!found && type->parentTypeName.has_value()) {
                typeOpt = context.symbolTable.lookupType(type->parentTypeName.value());
            }
            else break;
        }
        if (!found) {
            errors.emplace_back(context.filename, "Type '" + std::get<TypeExprPtr>(exprType.value())->type + "' does not have field or method called '" + expr->name + "'.", expr->loc, false,false);
            result = false;
        }
        expr->index = index;
        expr->accessedType = accessedType;
        return result;
    }

    bool checkExpr(const TypeExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (!context.symbolTable.lookupType(expr->type) && !Context::isBuiltInType(expr->type)) {
            if (!context.symbolTable.lookupType(context.moduleName + "." + expr->type)) {
                errors.emplace_back(context.filename, "Type '" + expr->type + "' does not exist.", expr->loc, false, false);
                result = false;
            } else {
                // update type name
                expr->type = context.moduleName + "." + expr->type;
            }
        }
        return result;
    }

    bool checkExpr(const ExprPtrVariant &expr, Context &context, std::vector<ErrorMessage> &errors) {
        return std::visit([&](const auto &expr) {
            return checkExpr(expr, context, errors);
        }, expr);
    }
}