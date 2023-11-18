//
// Created by user on 17.11.2023.
//

#include <vector>

#ifndef SLANGCREFACTORED_SYMBOLTABLE_HPP
#define SLANGCREFACTORED_SYMBOLTABLE_HPP

#endif //SLANGCREFACTORED_SYMBOLTABLE_HPP

#include <vector>
#include <optional>
#include "parser/ASTFwdDecl.hpp"
#include "common.hpp"

namespace Slangc {
    struct Symbol {
        std::string name;
        std::string moduleName;
        DeclPtrVariant declaration;
    };
    class SymbolTable {
    public:
        std::vector<Symbol> symbols;

        void insert(const std::string& name, const std::string& moduleName, const DeclPtrVariant &declarationNode) {
            symbols.emplace_back(name, moduleName, declarationNode);
        }

        auto lookup(std::string_view name) const -> const DeclPtrVariant* {
            for (const auto& symbol: symbols) {
                if (symbol.name == name) {
                    return &symbol.declaration;
                }
            }
            return nullptr;
        }

        auto lookupFunc(std::string_view name, const FuncExprPtr& expr, const Context& context) const -> const DeclPtrVariant* {
            for (const auto& symbol: symbols) {
                if (symbol.name == name) {
                    if (std::holds_alternative<FuncDecStatementPtr>(symbol.declaration)) {
                        auto func = std::get<FuncDecStatementPtr>(symbol.declaration);
                        if (compareFuncSignatures(func, expr, context)) {
                            return &symbol.declaration;
                        }
                    }
                }
            }
            return nullptr;
        }

        auto lookupType(const std::string& name) const -> std::optional<TypeDecStmtPtr> {
            for (const auto& symbol: symbols) {
                if (symbol.name == name) {
                    if (std::holds_alternative<TypeDecStmtPtr>(symbol.declaration)) {
                        return std::get<TypeDecStmtPtr>(symbol.declaration);
                    }
                }
            }
            return std::nullopt;
        }
    };
}
