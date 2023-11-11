//
// Created by f0xeri on 26.05.2023.
//

#ifndef SLANGCREFACTORED_CONTEXT_HPP
#define SLANGCREFACTORED_CONTEXT_HPP

#include <memory>
#include <map>
#include <optional>
#include "parser/Scope.hpp"
#include "common.hpp"

namespace Slangc {
    class Context {
        std::shared_ptr<Scope> currScope = std::make_unique<Scope>();
    public:
        std::map<std::string, TypeDecStmtPtr> types;
        std::map<std::string, VarDecStmtPtr> global_vars;
        std::vector<FuncDecStatementPtr> funcs;
        std::map<std::string, ExternFuncDecStmtPtr> extern_funcs;

        std::vector<std::pair<ExprPtrVariant, SourceLoc>> currFuncReturnTypes;
        std::string moduleName;
        auto enterScope() -> void {
            currScope = std::make_unique<Scope>(std::move(currScope));
        }

        auto exitScope() -> void {
            currScope = currScope->parent;
        }

        auto insert(const std::string& declName, const DeclPtrVariant &declarationNode) -> void {
            currScope->insert(declName, declarationNode);
        }

        auto remove(const std::string& declName) -> void {
            currScope->remove(declName);
        }

        auto lookup(std::string_view name) const -> const DeclPtrVariant* {
            return currScope->lookup(name);
        }

        auto lookupFunc(std::string_view name, FuncExprPtr expr) const -> const DeclPtrVariant* {
            return currScope->lookupFunc(name, expr);
        }

        auto getVarDeclType(std::string_view name) const -> ExprPtrVariant {
            auto* node = currScope->lookup(name);
        }

        static bool isBuiltInType(std::string_view name) {
            return name == "integer" || name == "float" || name == "real" || name == "boolean" || name == "string" || name == "character" || name == "void";
        }
    };
}

#endif //SLANGCREFACTORED_CONTEXT_HPP
