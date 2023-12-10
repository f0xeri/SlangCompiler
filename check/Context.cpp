//
// Created by f0xeri on 26.05.2023.
//

#include "Context.hpp"

namespace Slangc {
    bool Context::isBuiltInType(std::string_view name) {
        return name == "integer" || name == "float" || name == "real" || name == "boolean" || name == "character" || name == "void";
    }

    bool Context::isCastToLeft(const std::string& left, const std::string& right, const Context& context) {
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

    bool Context::isCastable(const std::string& from, const std::string& to, const Context& context) {
        if (from == "nil" || to == "nil")
            return true;
        if (from == "void" || to == "void")
            return true;
        if (!Context::isBuiltInType(from) || !Context::isBuiltInType(to)) {
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