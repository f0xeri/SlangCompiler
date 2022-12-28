//
// Created by Yaroslav on 03.10.2021.
//

#ifndef SLANGPARSER_LEXER_HPP
#define SLANGPARSER_LEXER_HPP

#include <vector>
#include <string>
#include <map>
#include "TokenType.hpp"
#include "Logger.hpp"

class Lexer {
public:
    static std::map<std::string, TokenType> tokensMap;
    static std::string getTokenName(TokenType type)
    {
        auto result = std::find_if(
                tokensMap.begin(),
                tokensMap.end(),
                [type](const auto& mo) {return mo.second == type; });

        if(result != tokensMap.end())
            return result->first;
        else
            if (type == TokenType::Identifier)
                return "identifier";
            else if (type == TokenType::String)
                return "string";
            else
                return "";
    }

    std::vector<Token> tokens;
    std::string sourceCode;
    uint64_t pos = 0;
    uint64_t currentStringNumber = 1;
    uint64_t currentSymbolNumber = 1;
    Lexer(const std::string &path);
    ~Lexer();
    void tokenize();
    void tokenizeNumber();
    void tokenizeString();
    void tokenizeWord();
    void tokenizeChar();
};


#endif //SLANGPARSER_LEXER_HPP
