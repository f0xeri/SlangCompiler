//
// Created by f0xeri on 24.01.2022.
//

#ifndef SLANGPARSER_SCOPE_HPP
#define SLANGPARSER_SCOPE_HPP

#include <algorithm>
#include <ranges>
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "AST.hpp"

class Scope
{
public:
    Scope *parent = nullptr;
    //std::unordered_map<std::string, DeclarationNode*> symbols;
    std::vector<std::pair<std::string, DeclarationNode*>> symbols;
    Scope() = default;
    Scope(Scope *parent) : parent(parent) {};
    void insert(DeclarationNode *declarationNode)
    {
        symbols.emplace_back(declarationNode->name->value, declarationNode);
    }

    bool contains(const std::string &name)
    {
        return std::ranges::find(symbols, name, &std::pair<std::string, DeclarationNode*>::first) != symbols.end();
    }

    DeclarationNode* get(const std::string &name)
    {
        return (*(std::ranges::find(symbols, name, &std::pair<std::string, DeclarationNode*>::first))).second;
    }

    DeclarationNode* lookup(const std::string &name)
    {
        Scope *s = this;
        while (s)
        {
            /*llvm::StringMap<DeclarationNode*>::iterator i = s->symbols.find(value->value);
            if (i != s->symbols.end()) return i->second;*/
            if (contains(name)) return get(name);
            s = s->parent;
        }
        return nullptr;
    }
};

#endif //SLANGPARSER_SCOPE_HPP
