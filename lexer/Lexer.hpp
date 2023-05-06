//
// Created by f0xeri on 04.05.2023.
//

#ifndef SLANGCREFACTORED_LEXER_HPP
#define SLANGCREFACTORED_LEXER_HPP

#include <unordered_map>
#include <map>
#include <memory>
#include <utility>
#include "TokenType.hpp"
#include "../common.hpp"
#include "../source/SourceBuffer.hpp"
#include "source_location"

namespace Slangc {
    class Lexer {
        std::vector<ErrorMessage> &errors;
        std::unique_ptr<SourceBuffer> sourceBuffer;

        uint64_t currentLine = 1;
        uint64_t currentColumn = 1;

        auto skipWhitespaces(std::string_view &sourceText) -> bool;
        auto lexSymbol(std::string_view &sourceText) -> bool;
        auto lexWordOrIdentifier(std::string_view &sourceText) -> bool;
        auto lexNumber(std::string_view &sourceText) -> bool;
        auto lexString(std::string_view &sourceText) -> bool;

        void addToken(TokenType type, std::string_view value, uint64_t column) {
            tokens.emplace_back(Token{type, std::string(value), SourceLoc{currentLine, column}});
        }

        void addToken(TokenType type, std::string value, uint64_t column) {
            tokens.emplace_back(Token{type, std::move(value), SourceLoc{currentLine, column}});
        }

        auto getErrors() -> std::vector<ErrorMessage> {
            return errors;
        }

    public:
       std::vector<Token> tokens;

        explicit Lexer(SourceBuffer &&sourceBuffer, std::vector<ErrorMessage> &errors) : errors(errors), sourceBuffer(std::make_unique<SourceBuffer>(sourceBuffer)) {}
        void tokenize();

        static const std::unordered_map<std::string_view, TokenType> tokensMap;
        static std::string_view getTokenName(TokenType type) {
            auto result = std::find_if(
                    tokensMap.begin(),
                    tokensMap.end(),
                    [type](const auto& mo) {return mo.second == type; });
            return result != tokensMap.end() ? result->first : "";
        }

        static std::string_view getTokenName(const Token& token) {
            switch (token.type) {
                case TokenType::Identifier:
                case TokenType::Integer:
                case TokenType::Float:
                case TokenType::Real:
                case TokenType::String:
                case TokenType::Boolean:
                    return token.value;
                case TokenType::EndOfFile:
                    return "EOF";
                default:
                    return getTokenName(token.type);
            }
        }

        void printTokens() {
            for (auto& token : tokens) {
                std::cout << "" << getTokenName(token) << " ";
            }
            std::cout << "\n";
        }
    };
}


#endif //SLANGCREFACTORED_LEXER_HPP
