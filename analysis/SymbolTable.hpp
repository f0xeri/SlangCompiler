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
    class SymbolTable {
    public:
        std::vector<std::pair<std::string, DeclPtrVariant>> symbols;

        void insert(const std::string& name, const DeclPtrVariant &declarationNode) {
            symbols.emplace_back(name, declarationNode);
        }

        auto lookup(std::string_view name) const -> const DeclPtrVariant* {
            for (const auto&[symbolName, symbol]: symbols) {
                if (symbolName == name) {
                    return &symbol;
                }
            }
            return nullptr;
        }

        auto lookupFunc(std::string_view name, const FuncExprPtr& expr) const -> const DeclPtrVariant* {
            for (const auto&[symbolName, symbol]: symbols) {
                if (symbolName == name) {
                    if (std::holds_alternative<FuncDecStatementPtr>(symbol)) {
                        auto func = std::get<FuncDecStatementPtr>(symbol);
                        if (compareFuncSignatures(func, expr)) {
                            return &symbol;
                        }
                    }
                }
            }
            return nullptr;
        }

        auto lookupType(const std::string& name) const -> std::optional<TypeDecStmtPtr> {
            for (const auto&[symbolName, symbol]: symbols) {
                if (symbolName == name) {
                    if (std::holds_alternative<TypeDecStmtPtr>(symbol)) {
                        return std::get<TypeDecStmtPtr>(symbol);
                    }
                }
            }
            return std::nullopt;
        }
    };
}
