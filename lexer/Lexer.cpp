//
// Created by f0xeri on 04.05.2023.
//

#include <iostream>
#include "Lexer.hpp"

namespace Slangc {
    const std::unordered_map<std::string_view, TokenType> Lexer::tokensMap = {
            {"import",    TokenType::Import},
            {"module",    TokenType::Module},
            {"start",     TokenType::Start},
            {"end",       TokenType::End},
            {"class",     TokenType::Class},
            {"inherits",  TokenType::Inherits},
            {"base",      TokenType::Base},
            {"field",     TokenType::Field},
            {"method",    TokenType::Method},
            {"function",  TokenType::Function},
            {"procedure", TokenType::Procedure},
            {"call",      TokenType::Call},
            {"return",    TokenType::Return},
            {"delete",    TokenType::Delete},
            {"extern",    TokenType::Extern},

            {"while",     TokenType::While},
            {"repeat",    TokenType::Repeat},
            {"do",        TokenType::Do},
            {"if",        TokenType::If},
            {"then",      TokenType::Then},
            {"else",      TokenType::Else},
            {"elseif",    TokenType::Elseif},
            {"input",     TokenType::Input},
            {"output",    TokenType::Output},
            {"variable",  TokenType::Variable},
            {":=",        TokenType::Assign},
            {"let",       TokenType::Let},

            {"+",         TokenType::Plus},
            {"-",         TokenType::Minus},
            {"/",         TokenType::Division},
            {"*",         TokenType::Multiplication},
            {"%",         TokenType::Remainder},

            {"(",         TokenType::LParen},
            {")",         TokenType::RParen},
            {"[",         TokenType::LBracket},
            {"]",         TokenType::RBracket},
            {":",         TokenType::Colon},
            {";",         TokenType::Semicolon},
            {",",         TokenType::Comma},
            {".",         TokenType::Dot},
            {"//",        TokenType::Comment},

            {"&&",        TokenType::And},
            {"||",        TokenType::Or},
            {"==",        TokenType::Equal},
            {"!=",        TokenType::NotEqual},
            {">",         TokenType::Greater},
            {"<",         TokenType::Less},
            {">=",        TokenType::GreaterOrEqual},
            {"<=",        TokenType::LessOrEqual},
            {"!",         TokenType::Neg},

            {"public",    TokenType::VisibilityType},
            {"private",   TokenType::VisibilityType},
            {"nil",       TokenType::Nil},
            {"false",     TokenType::Boolean},
            {"true",      TokenType::Boolean}
    };

    void Lexer::tokenize() {
        auto sourceText = sourceBuffer->getText();
        while (skipWhitespaces(sourceText)) {
            bool result = lexSymbol(sourceText);
            if (!result) {
                result = lexWordOrIdentifier(sourceText);
            }
            if (!result) {
                result = lexNumber(sourceText);
            }
            if (!result) {
                result = lexString(sourceText);
            }
            if (!result) {
                errors.emplace_back("Unknown token", SourceLoc{currentLine, currentColumn}, false, false);
                break;
            }
        }
        addToken(TokenType::EndOfFile, currentColumn);
    }

    bool Lexer::skipWhitespaces(std::string_view &sourceText) {
        auto whitespaceStart = sourceText.begin();

        while (!sourceText.empty()) {
            // skip comments
            if (sourceText.starts_with("//")) {
                while (!sourceText.empty() && sourceText.front() != '\n') {
                    ++currentColumn;
                    sourceText.remove_prefix(1);
                }
            }
            if (sourceText.empty()) {
                break;
            }

            switch (sourceText.front()) {
                case ' ':
                case '\t':
                    ++currentColumn;
                    sourceText.remove_prefix(1);
                    continue;
                case '\n':
                    ++currentLine;
                    currentColumn = 0;
                    sourceText.remove_prefix(1);
                    continue;
                default:
                    return true;
            }
        }
        if (sourceText.empty()) {
            return false;
        }
        return false;
    }

    bool Lexer::lexSymbol(std::string_view &sourceText) {
        std::string_view symbol = sourceText.substr(0, 1);
        auto symbolCol = currentColumn;

        if (sourceText.size() > 1) {
            if (sourceText.at(1) == '=' || sourceText.starts_with("||") || sourceText.starts_with("&&")) {
                symbol = sourceText.substr(0, 2);
            }
        }
        if (tokensMap.contains(symbol)) {
            addToken(tokensMap.at(symbol), symbol, symbolCol);
            sourceText.remove_prefix(symbol.size());
            currentColumn += symbol.size();
            return true;
        }
        return false;
    }

    bool Lexer::lexWordOrIdentifier(std::string_view &sourceText) {
        if (!std::isalpha(sourceText.front()) && sourceText.front() != '_') {
            return false;
        }
        auto identifierValue = takeWhile(sourceText, [](char c) { return std::isalnum(c) || c == '_'; });
        if (identifierValue.empty()) {
            return false;
        }
        auto identifierCol = currentColumn;
        currentColumn += identifierValue.size();
        sourceText.remove_prefix(identifierValue.size());

        if (tokensMap.contains(identifierValue)) {
            addToken(tokensMap.at(identifierValue), identifierValue, identifierCol);
        } else {
            addToken(TokenType::Identifier, identifierValue, identifierCol);
        }

        return true;
    }

    bool Lexer::lexNumber(std::string_view &sourceText) {
        bool isInt = true;
        bool isFloat = false;
        int i = 1;
        auto numberColumn = currentColumn;
        if (!std::isdigit(sourceText.front())) {
            return false;
        }
        for (; i < sourceText.size(); ++i) {
            if (std::isdigit(sourceText.at(i))) {
                continue;
            }
            if (sourceText.at(i) == '.' && isInt) {
                isInt = false;
                ++i;
                continue;
            }

            if (sourceText.at(i) == 'f' && !isFloat) {
                isFloat = true;
                ++i;
                break;
            }
            break;
        }
        auto numberValue = sourceText.substr(0, i);
        currentColumn += numberValue.size();
        sourceText.remove_prefix(numberValue.size());
        if (numberValue.empty()) {
            return false;
        }
        if (isInt) {
            addToken(TokenType::Integer, numberValue, numberColumn);
        } else if (isFloat) {
            addToken(TokenType::Float, numberValue, numberColumn);
        } else if (!isInt && !isFloat) {
            addToken(TokenType::Real, numberValue, numberColumn);
        }
        return true;
    }

    bool Lexer::lexString(std::string_view &sourceText) {
        if (sourceText.front() != '"') {
            return false;
        }
        auto stringColumn = currentColumn;
        auto stringValueView = takeWhile(sourceText.substr(1), [=](char c) { return c != '"'; });

        if (!stringValueView.empty()) {
            if (stringValueView.back() == sourceText.back()) {
                errors.emplace_back("Unclosed string", SourceLoc{currentLine, stringColumn}, false, false);
                return false;
            }
        }

        currentColumn += stringValueView.size() + 2;
        sourceText.remove_prefix(stringValueView.size() + 2);

        auto stringValue = std::string(stringValueView);

        bool sequenceError = false;
        for (int i = 0; i < stringValue.size(); ++i) {
            if (stringValue.at(i) == '\\') {
                if (i + 1 < stringValue.size()) {
                    switch (stringValue.at(i + 1)) {
                        case 'n':
                            stringValue.replace(i, 2, "\n");
                            break;
                        case 't':
                            stringValue.replace(i, 2, "\t");
                            break;
                        case 'r':
                            stringValue.replace(i, 2, "\r");
                            break;
                        case '\\':
                        case '0':
                            stringValue.replace(i, 2, "\0");
                            break;
                        default:
                            errors.emplace_back("Unknown escape sequence", SourceLoc{currentLine, stringColumn}, false,
                                                false);
                            sequenceError = true;
                            break;
                    }
                }
            }
        }

        addToken(TokenType::String, std::move(stringValue), stringColumn);
        return !sequenceError;
    }
}
