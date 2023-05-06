//
// Created by f0xeri on 06.05.2023.
//

#ifndef SLANGCREFACTORED_SCOPE_HPP
#define SLANGCREFACTORED_SCOPE_HPP

#include <memory>
#include <utility>
#include <string>
#include "AST.hpp"

namespace Slangc {
    class Scope {
    public:
        std::shared_ptr<Scope> parent;

        Scope() = default;
        explicit Scope(std::shared_ptr<Scope> parent) : parent(std::move(parent)) {}

        std::vector<std::pair<std::string_view, AST::DeclarationNode>> symbols;

        void insert(AST::DeclarationNode declarationNode) {
            symbols.emplace_back(getDeclarationName(declarationNode), std::move(declarationNode));
        }

        bool contains(std::string_view name) {
            return std::ranges::find(symbols, name, &std::pair<std::string_view, AST::DeclarationNode>::first) != symbols.end();
        }

        auto get(std::string_view name) -> const AST::DeclarationNode* {
            auto it = std::ranges::find(symbols, name, &std::pair<std::string_view, AST::DeclarationNode>::first);
            return (it == symbols.end()) ? nullptr : &(it->second);
        }

        auto lookup(std::string_view name) -> const AST::DeclarationNode* {
            Scope *s = this;
            while (s) {
                if (auto* node = s->get(name)) {
                    return node;
                }
                s = s->parent.get();
            }
            return nullptr;
        }

    };
}

#endif //SLANGCREFACTORED_SCOPE_HPP
