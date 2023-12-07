//
// Created by f0xeri on 26.05.2023.
//

#ifndef SLANGCREFACTORED_CONTEXT_HPP
#define SLANGCREFACTORED_CONTEXT_HPP

#include <memory>
#include <map>
#include <optional>
#include <filesystem>
#include "parser/Scope.hpp"
#include "SymbolTable.hpp"
#include "common.hpp"
#include "lexer/TokenType.hpp"

namespace Slangc {
    extern std::map<std::filesystem::path, std::shared_ptr<Context>> globalImports;
    class Context {
        std::shared_ptr<Scope> currScope = std::make_unique<Scope>();
    public:
        SymbolTable symbolTable;

        std::vector<std::pair<ExprPtrVariant, SourceLoc>> currFuncReturnTypes;
        std::string currType;
        std::string moduleName;
        std::string filename;
        std::vector<std::string> imports;
        auto enterScope(const std::string &name) -> void {
            if (currScope->children[name] == nullptr) {
                currScope->children[name] = std::make_shared<Scope>(currScope);
            }
            currScope = currScope->children.at(name);
        }

        // enterScope by iterator
        auto enterScope(const Scope::ChildrenIterator& iter) -> void {
            currScope = iter->second;
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

        auto clearCurrentScope() -> void {
            currScope->clear();
        }

        auto lookup(std::string_view name) const -> const DeclPtrVariant* {
            return currScope->lookup(name);
        }

        auto lookupFuncInScope(std::string_view name, FuncExprPtr& expr, const Context& context, bool checkReturnTypes) const -> const DeclPtrVariant* {
            return currScope->lookupFunc(name, expr, context, checkReturnTypes);
        }

        /*auto lookupType(const std::string& name) const -> std::optional<TypeDecStmtPtr> {
            if (types.contains(name)) {
                return types.at(name);
            }
            else if (types.contains(moduleName + "." + name)) {
                return types.at(moduleName + "." + name);
            }
            return std::nullopt;
        }*/

        static bool isBuiltInType(std::string_view name) {
            return name == "integer" || name == "float" || name == "real" || name == "boolean" || name == "character" || name == "void";
        }

        static bool isCastToLeft(const std::string& left, const std::string& right, const Context& context) {
            if (!isBuiltInType(left) || !isBuiltInType(right)) {
                if (isParentType(right, left, context))
                    return true;
                else return false;
            }
            static std::map<std::string, int> typeStrengthMap = {
                    {"real", 6},
                    {"float", 5},
                    {"integer", 4},
                    {"character", 3},
                    {"boolean", 2},
                    {"void", 1},
                    {"nil", 1}
            };
            if (typeStrengthMap[left] > typeStrengthMap[right]) {
                return true;
            }
            return false;
        }

        static bool isCastable(const std::string& from, const std::string& to, const Context& context) {
            if (from == "nil" || to == "nil")
                return true;
            if (from == "void" || to == "void")
                return true;
            if (!isBuiltInType(from) || !isBuiltInType(to)) {
                if (isParentType(to, from, context) ||
                    isParentType(from, to, context)) {
                    return true;
                }
                else return false;
            }
            if ((from == "integer" || from == "float" || from == "real" || from == "character" || from == "boolean") &&
                (  to == "integer" ||   to == "float" ||   to == "real" ||   to == "character" ||   to == "boolean"))
                return true;
            return false;
        }

        static std::string operatorToString(TokenType tokenType) {
            switch (tokenType) {
                case TokenType::Plus:
                    return "+";
                case TokenType::Minus:
                    return "-";
                case TokenType::Multiplication:
                    return "*";
                case TokenType::Division:
                    return "/";
                case TokenType::Remainder:
                    return "%";
                case TokenType::Less:
                    return "<";
                case TokenType::LessOrEqual:
                    return "<=";
                case TokenType::Greater:
                    return ">";
                case TokenType::GreaterOrEqual:
                    return ">=";
                case TokenType::Equal:
                    return "==";
                case TokenType::NotEqual:
                    return "!=";
                case TokenType::And:
                    return "&&";
                case TokenType::Or:
                    return "||";
                case TokenType::Neg:
                    return "!";
                default:
                    return "";
            }
        }

        static bool isPrivateAccessible(const std::string& parent, const std::string& child, Context& context) {
            if (isParentType(child, parent, context)) {
                return true;
            }
            else return false;
        }
    };
}

#endif //SLANGCREFACTORED_CONTEXT_HPP
