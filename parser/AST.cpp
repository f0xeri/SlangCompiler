//
// Created by f0xeri on 07.05.2023.
//

#include <utility>
#include "common.hpp"
#include "AST.hpp"
#include "ASTFwdDecl.hpp"

namespace Slangc {
    // checks signatures WITHOUT return type
    bool compareFuncSignatures(const FuncExprPtr &func1, const FuncExprPtr &func2, const Context &context, bool checkReturnTypes, bool checkCast) {
        if (func1->params.size() != func2->params.size()) return false;
        if (checkReturnTypes) {
            if (!compareTypes(func1->type, func2->type, context)) return false;  // TODO: should we check cast here?
        }
        for (int i = 0; i < func1->params.size(); i++) {
            if (!compareTypes(func1->params[i]->type, func2->params[i]->type, context, checkCast)) {
                if (checkCast) {
                    // we can cast anything to void* (out void)
                    if (std::holds_alternative<TypeExprPtr>(func1->params[i]->type)) {
                        if (std::get<TypeExprPtr>(func1->params[i]->type)->type == "void" && func1->params[i]->parameterType == Out) {
                            return true;
                        }
                    }
                }
                return false;
            }
            // if parameterType is None, it means that it is not specified and we can cast
            if (func1->params[i]->parameterType != None && func2->params[i]->parameterType != None) {
                if (func1->params[i]->parameterType != func2->params[i]->parameterType) return false;
            }
            // if true, we're in call expr trying to pass func pointer to func
            // if func1 is out or var param and func2 is func (not func pointer), we can proceed
            if (func2->params[i]->parameterType == None && std::holds_alternative<FuncExprPtr>(func1->params[i]->type)) {
                if (func1->params[i]->parameterType == Out || func1->params[i]->parameterType == Var) {
                    if (!std::get<FuncExprPtr>(func2->params[i]->type)->isFunctionPtr) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool compareFuncSignatures(const FuncDecStatementPtr &func1, const FuncDecStatementPtr &func2, const Context &context, bool checkReturnTypes, bool checkCast) {
        return compareFuncSignatures(func1->expr, func2->expr, context,checkReturnTypes, checkCast);
    }

    bool compareFuncSignatures(const MethodDecPtr &func1, const MethodDecPtr &func2, const Context &context, bool checkReturnTypes, bool checkCast) {
        return compareFuncSignatures(func1->expr, func2->expr, context, checkReturnTypes, checkCast);
    }

    bool compareFuncSignatures(const FuncDecStatementPtr &func1, const FuncExprPtr &func2, const Context &context, bool checkReturnTypes, bool checkCast) {
        return compareFuncSignatures(func1->expr, func2, context, checkReturnTypes, checkCast);
    }

    bool compareTypes(const ExprPtrVariant &type1, const ExprPtrVariant &type2, const Context &context, bool checkCast, ParameterType parameterType1, ParameterType parameterType2) {
        bool result = false;
        if (auto type1Ptr = std::get_if<TypeExprPtr>(&type1)) {
            if (auto type2Ptr = std::get_if<TypeExprPtr>(&type2)) {
                result = type1Ptr->get()->type == type2Ptr->get()->type;
                if (!result && checkCast) {
                    result = Context::isCastable(type1Ptr->get()->type, type2Ptr->get()->type, context);
                }
                return result;
            }
        }
        if (auto type1Ptr = std::get_if<ArrayExprPtr>(&type1)) {
            if (auto type2Ptr = std::get_if<ArrayExprPtr>(&type2)) {
                return compareTypes(type1Ptr->get()->type, type2Ptr->get()->type, context);
            }
        }
        if (std::holds_alternative<FuncExprPtr>(type1)) {
            if (std::holds_alternative<FuncExprPtr>(type2)) {
                return compareFuncSignatures(std::get<FuncExprPtr>(type1), std::get<FuncExprPtr>(type2), context);
            }
        }
        if (checkCast) {
            if (auto type1ptr = std::get_if<NilExprPtr>(&type1)) {
                type1ptr->get()->type = type2;
                return true;
            }
            if (auto type2ptr = std::get_if<NilExprPtr>(&type2)) {
                type2ptr->get()->type = type1;
                return true;
            }
        }
        return false;
    }

    std::string parameterTypeToString(ParameterType parameterType) {
        switch (parameterType) {
            case In:
                return "in ";
            case Out:
                return "out ";
            case Var:
                return "var ";
            case None:
                return "";
        }
    }

    std::string typeToString(ExprPtrVariant type, ParameterType parameterType) {
        std::string result = parameterTypeToString(parameterType);
        if (auto typePtr = std::get_if<TypeExprPtr>(&type)) {
            return result + typePtr->get()->type;
        }
        if (auto typePtr = std::get_if<ArrayExprPtr>(&type)) {
            result += "array[] ";
            return result + typeToString(typePtr->get()->type);
        }
        if (std::holds_alternative<FuncExprPtr>(type)) {
            auto funcExpr = std::get<FuncExprPtr>(type);
            result += "function(";
            for (int i = 0; i < funcExpr->params.size(); i++) {
                result += typeToString(funcExpr->params[i]->type, funcExpr->params[i]->parameterType);
                if (i != funcExpr->params.size() - 1) result += ", ";
            }
            result += "): " + typeToString(funcExpr->type);
            return result;
        }
        if (std::holds_alternative<NilExprPtr>(type)) {
            auto nilExpr = std::get<NilExprPtr>(type);
            if (nilExpr->type) {
                return result + typeToString(nilExpr->type.value());
            }
            else return result + "nil";
        }
        return "unknown";
    }

    std::string typeToString(TypeExprNode &type, ParameterType parameterType = ParameterType::None) {
        return parameterTypeToString(parameterType) + type.type;
    }

    bool isParentType(const std::string &parentTypeName, const std::string &childTypeName, const Context &analysis) {
        if (parentTypeName == childTypeName) return true;
        if (auto parentType = analysis.symbolTable.lookupType(parentTypeName)) {
            if (auto childType = analysis.symbolTable.lookupType(childTypeName)) {
                if (childType.value()->parentTypeName.has_value()) {
                    return isParentType(parentTypeName, childType.value()->parentTypeName.value(), analysis);
                }
            }
        }
        return false;
    }

    // if there is no func with equal signature, choose the best available overload using implicit casts
    auto selectBestOverload(const std::string &name, FuncExprPtr &func, bool useParamType, bool checkReturnType, Context &analysis) -> std::optional<DeclPtrVariant> {
        std::optional<DeclPtrVariant> bestOverload = std::nullopt;
        auto bestOverloadScore = 0;

        for (auto &&f : analysis.symbolTable.symbols | std::views::filter([](const auto &s) { return std::holds_alternative<FuncDecStatementPtr>(s.declaration); })) {
            auto funcDec = std::get<FuncDecStatementPtr>(f.declaration);
            if (funcDec->name == name) {
                if (compareFuncSignatures(funcDec, func, analysis, checkReturnType)) {
                    return funcDec;
                }
                if (funcDec->expr->params.size() == func->params.size()) {
                    auto score = 0;
                    for (int i = 0; i < funcDec->expr->params.size(); i++) {
                        auto type1 = typeToString(funcDec->expr->params[i]->type, useParamType ? funcDec->expr->params[i]->parameterType : ParameterType::None);
                        auto type2 = typeToString(func->params[i]->type, useParamType ? func->params[i]->parameterType : ParameterType::None);
                        if (type1 == type2) {
                            score += 2;
                        }
                        else {
                            if (Context::isCastable(type1, type2, analysis)) {
                                score += 1;
                            }
                            else {
                                score = 0;
                                break;
                            }
                        }
                    }
                    if (score > bestOverloadScore) {
                        bestOverloadScore = score;
                        bestOverload = funcDec;
                    }
                }
            }
        }
        return bestOverload;
    }

    auto selectBestOverload(const TypeDecStmtPtr &typeDecl, const std::string &methodName, const FuncExprPtr &func, bool useParamType, bool checkReturnType, Context &analysis) -> std::optional<DeclPtrVariant> {
        std::optional<DeclPtrVariant> bestOverload = std::nullopt;
        auto bestOverloadScore = 0;
        for (const auto &method: typeDecl->methods) {
            if (method->name == methodName) {
                if (compareFuncSignatures(method->expr, func, analysis, checkReturnType)) {
                    return method;
                }
                if (method->expr->params.size() == func->params.size()) {
                    auto score = 0;
                    for (int i = 0; i < method->expr->params.size(); i++) {
                        auto type1 = typeToString(method->expr->params[i]->type, useParamType ? method->expr->params[i]->parameterType : ParameterType::None);
                        auto type2 = typeToString(func->params[i]->type, useParamType ? func->params[i]->parameterType : ParameterType::None);
                        if (type1 == type2) {
                            score += 2;
                        }
                        else {
                            if (Context::isCastable(type1, type2, analysis)) {
                                score += 1;
                            }
                            else {
                                score = 0;
                                break;
                            }
                        }
                    }
                    if (score > bestOverloadScore) {
                        bestOverloadScore = score;
                        bestOverload = method;
                    }
                }
            }
        }
        return bestOverload;
    }
}