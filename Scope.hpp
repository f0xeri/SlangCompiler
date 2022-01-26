//
// Created by f0xeri on 24.01.2022.
//

#ifndef SLANGPARSER_SCOPE_HPP
#define SLANGPARSER_SCOPE_HPP

#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "AST.hpp"

class Scope
{
public:
    Scope *parent = nullptr;
    std::map<std::string, DeclarationNode*> symbols;
    Scope() = default;
    Scope(Scope *parent) : parent(parent) {};
    bool insert(DeclarationNode *declarationNode)
    {
        return symbols.insert({declarationNode->name->value, declarationNode}).second;
    }

    DeclarationNode* lookup(VariableExprNode *name)
    {
        Scope *s = this;
        while (s)
        {
            /*llvm::StringMap<DeclarationNode*>::iterator i = s->symbols.find(name->value);
            if (i != s->symbols.end()) return i->second;*/
            if (symbols.contains(name->value)) return symbols[name->value];
            s = s->parent;
        }
        return nullptr;
    }
};

#endif //SLANGPARSER_SCOPE_HPP
