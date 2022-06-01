//
// Created by Yaroslav on 03.10.2021.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include "Lexer.hpp"

std::map<std::string, TokenType> Lexer::tokensMap = {{"import",    TokenType::Import},
                                                     {"module",    TokenType::Module},
                                                     {"start",     TokenType::Start},
                                                     {"end",       TokenType::End},
                                                     {"class",     TokenType::Class},
                                                     {"inherits",  TokenType::Inherits},
                                                     {"base",      TokenType::Base},
                                                     {"abstract",  TokenType::Abstract},
                                                     {"override",  TokenType::Override},
                                                     {"field",     TokenType::Field},
                                                     {"method",    TokenType::Method},
                                                     {"function",  TokenType::Function},
                                                     {"procedure", TokenType::Procedure},
                                                     {"call",      TokenType::Call},
                                                     {"return",    TokenType::Return},
                                                     {"delete",    TokenType::Delete},

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

                                                    /*{"integer", TokenType::Integer},
                                                    {"real", TokenType::Real},
                                                    {"character", TokenType::Character},
                                                    {"boolean", TokenType::Boolean},
                                                    {"string", TokenType::String},*/
                                                     {"public",    TokenType::VisibilityType},
                                                     {"private",   TokenType::VisibilityType},
                                                     {"nil",       TokenType::Nil},
                                                     {"false",     TokenType::Boolean},
                                                     {"true",      TokenType::Boolean}
};

Lexer::Lexer(const std::string &path) {


    std::ifstream file(path);
    if (!file.is_open()) {
        //LOG(ERROR) << "Failed to open " + path;
        exit(1);
    } else {
        std::stringstream buffer;
        buffer << file.rdbuf();
        sourceCode = buffer.str();
    }
}

Lexer::~Lexer() {

}

void Lexer::tokenize() {
    while (pos < sourceCode.size()) {
        char symbol = sourceCode[pos];
        if (isalpha(symbol)) {
            tokenizeWord();
        } else if (isdigit(symbol)) {
            tokenizeNumber();
        } else if (symbol == '\n') {
            if (DEBUG) std::cout << "\n";
        } else if (isdigit(symbol)) {
            tokenizeNumber();
        } else if (symbol == ' ') {
            // nothing
        } else {
            tokenizeChar();
        }
        pos++;
    }
    tokens.push_back({TokenType::EndOfFile, ""});
    if (DEBUG) std::cout << "\n\n";
}

void Lexer::tokenizeNumber() {
    char symbol = sourceCode[pos];
    std::string str;
    bool real = false;
    while (isdigit(symbol) || symbol == '.') {
        if (symbol == '.') {
            if (!real) real = true;
            else {
                //LOG(ERROR) << "Real number can contain only 1 point";
                exit(1);
            }
        }
        str += symbol;
        symbol = sourceCode[++pos];
    }
    if (isalpha(symbol)) {
        //LOG(ERROR) << "Number can contain only digits";
        exit(1);
    }
    pos--;
    if (DEBUG) std::cout << "NUMBER(" << str << ") ";
    tokens.push_back({real ? TokenType::Real : TokenType::Integer, str});
}

void Lexer::tokenizeString() {
    char symbol = sourceCode[++pos];
    std::string str;
    while (symbol != '"') {
        str += symbol;
        symbol = sourceCode[++pos];
    }

    if (DEBUG) std::cout << "STRING(" << str << ") ";
    tokens.push_back({TokenType::String, str});
}

void Lexer::tokenizeWord() {
    unsigned int tpos = pos + 1;
    std::string word;
    word += sourceCode[pos];
    while (isalpha(sourceCode[tpos]) || isdigit(sourceCode[tpos])) {
        word += sourceCode[tpos];
        tpos++;
    }

    pos = tpos - 1;
    if (tokensMap.contains(word)) {
        if (tokensMap.find(word)->second == TokenType::VisibilityType) {
            if (DEBUG) std::cout << "VISIBILITYTYPE(" << word << ") ";
        } else {
            if (DEBUG) std::cout << "WORD(" << word << ") ";
        }

        tokens.push_back({tokensMap.find(word)->second, word});
    } else {
        if (DEBUG) std::cout << "ID" << "(" << word << ") ";
        tokens.push_back({TokenType::Identifier, word});
    }

}

void Lexer::tokenizeChar() {
    std::string word;
    word += sourceCode[pos];
    if (sourceCode[pos] == '"') {
        tokenizeString();
        return;
    }
    else if (sourceCode[pos + 1] == '=' || sourceCode[pos] == '|' && sourceCode[pos + 1] == '|' ||
             sourceCode[pos] == '&' && sourceCode[pos] == '&') {
        word += sourceCode[pos + 1];
        pos++;
    }

    if (tokensMap.contains(word)) {
        tokens.push_back({tokensMap.find(word)->second});
        if (DEBUG) std::cout << "(" << word << ") ";
    }
}
