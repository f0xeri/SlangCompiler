//
// Created by f0xeri on 26.05.2023.
//

#include "Context.hpp"

namespace Slangc {
    bool Context::isBuiltInType(std::string_view name) {
        return builtInTypes.contains(name);
    }

    bool Context::isCastToLeft(const std::string& left, const std::string& right, const Context& context) {
        if (!isBuiltInType(left) || !isBuiltInType(right)) {
            if (isParentType(right, left, context))
                return true;
            else return false;
        }
        static std::map<BuiltInType, int> typeStrengthMap = {
                {BuiltInType::Real, 6},
                {BuiltInType::Float, 5},
                {BuiltInType::Int, 4},
                {BuiltInType::Char, 3},
                {BuiltInType::Bool, 2},
                {BuiltInType::Void, 1}
        };
        auto getTypeStrength = [](std::string_view type) { return type == "nil" ? 1 : typeStrengthMap[builtInTypes.at(type)]; };
        if (getTypeStrength(left) > getTypeStrength(right)) {
            return true;
        }
        return false;
    }

    bool Context::isCastable(const std::string& from, const std::string& to, const Context& context) {
        if (from == "nil" || to == "nil")
            return true;
        if (from == getBuiltInTypeName(BuiltInType::Void) || to == getBuiltInTypeName(BuiltInType::Void))
            return true;
        if (!Context::isBuiltInType(from) || !Context::isBuiltInType(to)) {
            if (isParentType(to, from, context) ||
                isParentType(from, to, context)) {
                return true;
            }
            else return false;
        }
        if (isBuiltInNonVoid(from) && isBuiltInNonVoid(to))
            return true;
        return false;
    }

    std::string Context::operatorToString(TokenType tokenType) {
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

    bool Context::isPrivateAccessible(const std::string& parent, const std::string& child, Context& context) {
        if (isParentType(child, parent, context)) {
            return true;
        }
        else return false;
    }
}