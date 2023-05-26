//
// Created by f0xeri on 26.05.2023.
//

#ifndef SLANGCREFACTORED_BASICANALYSIS_HPP
#define SLANGCREFACTORED_BASICANALYSIS_HPP

#include <memory>
#include "parser/Scope.hpp"

namespace Slangc {
    class BasicAnalysis {
        std::shared_ptr<Scope> currScope = std::make_unique<Scope>();

    public:
        auto enterScope() -> void {
            currScope = std::make_unique<Scope>(std::move(currScope));
        }

        auto exitScope() -> void {
            currScope = currScope->parent;
        }

        auto insert(const std::string& declName, const DeclPtrVariant &declarationNode) -> void {
            currScope->insert(declName, declarationNode);
        }

        auto lookup(std::string_view name) const -> const DeclPtrVariant* {
            return currScope->lookup(name);
        }

        auto getVarDeclType(std::string_view name) const -> Type {
            auto* node = currScope->lookup(name);
        }
    };
}

#endif //SLANGCREFACTORED_BASICANALYSIS_HPP
