//
// Created by f0xeri on 06.05.2023.
//

#ifndef SLANGCREFACTORED_SCOPE_HPP
#define SLANGCREFACTORED_SCOPE_HPP

#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <optional>
#include <map>
#include <algorithm>
#include "ASTFwdDecl.hpp"
#include "log/Logger.hpp"

namespace Slangc {
    class Scope {
    public:
        std::shared_ptr<Scope> parent;
        std::map<std::string, std::shared_ptr<Scope>> children;
        using ChildrenIterator = std::map<std::string, std::shared_ptr<Scope>>::iterator;

        Scope() = default;
        explicit Scope(std::shared_ptr<Scope> parent) : parent(std::move(parent)) {}
        std::vector<std::pair<std::string, DeclPtrVariant>> symbols;

        void insert(const std::string& declName, const DeclPtrVariant& declarationNode) {
            symbols.emplace_back(declName, declarationNode);
        }

        void remove(const std::string& declName) {
            symbols.erase(std::ranges::find(symbols, declName, &std::pair<std::string, DeclPtrVariant>::first));
        }

        void clear() {
            symbols.clear();
        }

        bool contains(std::string_view name) {
            return std::ranges::find(symbols, name, &std::pair<std::string, DeclPtrVariant>::first) != symbols.end();
        }

        auto get(std::string_view name) -> const DeclPtrVariant* {
            auto it = std::ranges::find(symbols, name, &std::pair<std::string, DeclPtrVariant>::first);
            return (it == symbols.end()) ? nullptr : &(it->second);
        }

        auto getNamedChild(const std::string &name) -> std::optional<std::shared_ptr<Scope>> {
            return children[name];
        }

        auto lookup(std::string_view name) -> const DeclPtrVariant* {
            Scope *s = this;
            while (s) {
                if (auto* node = s->get(name)) {
                    return node;
                }
                s = s->parent.get();
            }
            return nullptr;
        }

        auto lookupFunc(std::string_view name, FuncExprPtr& expr, const Context &context, bool checkReturnTypes) -> const DeclPtrVariant* {
            Scope *s = this;
            while (s) {
                for (auto &decl : s->symbols) {
                    if (std::holds_alternative<FuncDecStatementPtr>(decl.second)) {
                        auto func = std::get<FuncDecStatementPtr>(decl.second);
                        if (decl.first == name) {
                            if (compareFuncSignatures(func, expr, context, checkReturnTypes)) {
                                return &decl.second;
                            }
                        }
                    }
                }
                s = s->parent.get();
            }
            return nullptr;
        }
    };
}

#endif //SLANGCREFACTORED_SCOPE_HPP
