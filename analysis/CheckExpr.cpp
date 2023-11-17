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
                if (!Context::isCastable(leftType, rightType, context)) {
                    errors.emplace_back(std::string("Type mismatch: cannot apply operator '") + "' to '" + leftType + "' and '" + rightType + "'.", expr->loc, false, false);
                    result = false;
                }
                else {
                    errors.emplace_back("Implicit conversion from '" + rightType + "' to '" + leftType + "'.", expr->loc, true, false);
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
                errors.emplace_back("Cannot apply indexing with [] to an expression of type '" + typeToString(type.value()) + "'.",expr->loc, false, false);
            else
                errors.emplace_back("Cannot apply indexing with [] to an expression of unknown type.", expr->loc, false,false);
            return false;
        }
        if (indexType.has_value()) {
            auto typeStr = typeToString(indexType.value());
            if (typeStr != "integer") {
                errors.emplace_back("Index must be of type 'integer, not '" + typeStr + "'.", expr->loc, false, false);
                result = false;
            }
        }
        return result;
    }

    bool checkExpr(const CallExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(expr->name, context, errors);
        for (const auto &arg: expr->args) {
            result &= checkExpr(arg, context, errors);
        }
        if (!result) return false;
        std::optional<DeclPtrVariant> func = std::nullopt;
        std::optional<IndexExprPtr> funcPtrsArrIndex = std::nullopt;
        bool overloaded = false;

        // create func expr based on arg types
        std::vector<FuncParamDecStmtPtr> params;
        params.reserve(expr->args.size());
        auto zeroLoc = SourceLoc{0, 0};
        for (const auto &arg: expr->args) {
            params.push_back(create<FuncParamDecStatementNode>(zeroLoc, "", ParameterType::None, getExprType(arg, context, errors).value()));
        }
        auto funcExpr = create<FuncExprNode>(zeroLoc, create<TypeExprNode>(zeroLoc, "void"), params);

        auto type = getExprType(expr->name, context, errors);
        // check is callable
        if (auto access = std::get_if<AccessExprPtr>(&expr->name)) {
            auto accessType = getExprType(access->get()->expr, context, errors);
            if (std::holds_alternative<TypeExprPtr>(accessType.value())) {
                auto typeDecl = context.symbolTable.lookupType(std::get<TypeExprPtr>(accessType.value())->type);
                if (typeDecl) {
                    funcExpr->params.insert(funcExpr->params.begin(), create<FuncParamDecStatementNode>(zeroLoc, "", ParameterType::Out, std::get<TypeExprPtr>(accessType.value())));
                    func = selectBestOverload(typeDecl.value(), typeDecl.value()->name + "." + access->get()->name, funcExpr, false, false, context);
                    overloaded = true;
                }
            }
        }
        else if (auto var = std::get_if<VarExprPtr>(&expr->name)) {
            func = selectBestOverload(var->get()->name, funcExpr, false, false, context);
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
                        if (compareFuncSignatures(std::get<FuncPointerStmtPtr>(*funcPointer)->expr, funcExpr, false))
                            func = *funcPointer;
                    }
                    else if (std::holds_alternative<FieldFuncPointerStmtPtr>(*funcPointer)) {
                        overloaded = true;
                        if (compareFuncSignatures(std::get<FieldFuncPointerStmtPtr>(*funcPointer)->expr, funcExpr, false))
                            func = *funcPointer;
                    }
                    else if (std::holds_alternative<FuncParamDecStmtPtr>(*funcPointer)) {
                        auto param = std::get<FuncParamDecStmtPtr>(*funcPointer);
                        if (std::holds_alternative<FuncExprPtr>(param->type)) {
                            overloaded = true;
                            if (compareFuncSignatures(std::get<FuncExprPtr>(param->type), funcExpr, false))
                                func = *funcPointer;
                        }
                    }
                }
            }
        }
        else if (auto index = std::get_if<IndexExprPtr>(&expr->name)) {
            auto indexType = getExprType(*index, context, errors);
            if (indexType.has_value() && std::holds_alternative<FuncExprPtr>(indexType.value())) {
                overloaded = true;
                if (compareFuncSignatures(std::get<FuncExprPtr>(indexType.value()), funcExpr, false)) {
                    funcPtrsArrIndex = *index;
                }
            }
        }
        if (!func && !funcPtrsArrIndex) {
            errors.emplace_back(overloaded ? "No matching function for call." : "Expression is not callable.", expr->loc, false, false);
            result = false;
        }

        return result;
    }

    bool checkExpr(const AccessExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        result = checkExpr(expr->expr, context, errors);
        if (!result) return result;
        auto exprType = getExprType(expr->expr, context, errors);
        if (!exprType.has_value()) {
            errors.emplace_back("Failed to get type of expression.", expr->loc, false, false);
            return false;
        }
        if (std::holds_alternative<TypeExprPtr>(exprType.value()) && !Context::isBuiltInType(std::get<TypeExprPtr>(exprType.value())->type)) {
            if (auto typeOpt = context.symbolTable.lookupType(std::get<TypeExprPtr>(exprType.value())->type)) {
                auto type = typeOpt.value();
                auto found = false;
                for (const auto &field: type->fields) {
                    if (auto fieldVar = std::get_if<FieldVarDecPtr>(&field)) {
                        if ((*fieldVar)->name == expr->name) {
                            if ((*fieldVar)->isPrivate && !Context::isPrivateAccessible(context.currType, type->name, context)) {
                                errors.emplace_back("Cannot access private field '" + expr->name + "' of type '" + type->name + "' from '" + context.currType + "'.", expr->loc, false, false);
                                result = false;
                            }
                            // found = true even if we got error, because we want to suppress final not found error
                            found = true;
                        }
                    } else if (const auto &fieldArrayVar = std::get_if<FieldArrayVarDecPtr>(&field)) {
                        if ((*fieldArrayVar)->name == expr->name) {
                            if ((*fieldArrayVar)->isPrivate && !Context::isPrivateAccessible(context.currType, type->name, context)) {
                                errors.emplace_back("Cannot access private field '" + expr->name + "' of type '" + type->name + "' from '" + context.currType + "'.", expr->loc, false, false);
                                result = false;
                            }
                            found = true;
                        }
                    } else if (auto fieldFuncPointer = std::get_if<FieldFuncPointerStmtPtr>(&field)) {
                        if ((*fieldFuncPointer)->name == expr->name) {
                            if ((*fieldFuncPointer)->isPrivate && !Context::isPrivateAccessible(context.currType, type->name, context)) {
                                errors.emplace_back("Cannot access private field '" + expr->name + "' of type '" + type->name + "' from '" + context.currType + "'.", expr->loc, false, false);
                                result = false;
                            }
                            found = true;
                        }
                    }

                    if (found) break;
                }
                if (!found) {
                    for (const auto &method: type->methods) {
                        if ((method->name == type->name + "." + expr->name)) {
                            if (method->isPrivate && !Context::isPrivateAccessible(context.currType, type->name, context)) {
                                errors.emplace_back("Cannot access private method '" + expr->name + "' of type '" + type->name + "' from '" + context.currType + "'.", expr->loc, false, false);
                                result = false;
                            }
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) {
                    errors.emplace_back("Type '" + std::get<TypeExprPtr>(exprType.value())->type + "' does not have field or method called '" + expr->name + "'.", expr->loc, false,false);
                    result = false;
                }
            }
        }
        else {
            errors.emplace_back("Type '" + typeToString(exprType.value()) + "' is not accessible.", expr->loc, false, false);
            result = false;
        }
        return result;
    }

    bool checkExpr(const TypeExprPtr &expr, Context &context, std::vector<ErrorMessage> &errors) {
        bool result = true;
        if (!context.symbolTable.lookupType(expr->type) && !Context::isBuiltInType(expr->type)) {
            if (!context.symbolTable.lookupType(context.moduleName + "." + expr->type)) {
                errors.emplace_back("Type '" + expr->type + "' does not exist.", expr->loc, false, false);
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