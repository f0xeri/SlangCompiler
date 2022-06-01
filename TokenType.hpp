//
// Created by Yaroslav on 03.10.2021.
//

#ifndef SLANGPARSER_TOKENTYPE_HPP
#define SLANGPARSER_TOKENTYPE_HPP

enum class TokenType
{
    Import,
    Module,
    Start,
    End,
    Class,
    Inherits,
    Base,
    Abstract,
    Override,
    Field,
    Method,
    Function,
    Procedure,
    Call,
    Return,

    While,
    Repeat,
    Do,
    If,
    Then,
    Else,
    Elseif,
    Input,
    Output,
    Variable,
    Assign,
    Let,

    Plus,
    Minus,
    Division,
    Multiplication,

    LParen,
    RParen,
    LBracket,
    RBracket,
    Colon,
    Semicolon,
    Comma,
    Dot,
    Comment,

    And,
    Or,
    Equal,
    NotEqual,
    Greater,
    Less,
    GreaterOrEqual,
    LessOrEqual,

    VisibilityType,
    Identifier,
    Integer,
    Real,
    Boolean,
    String,
    Nil,

    EndOfFile,
    Delete,
};

struct Token
{
    TokenType type;
    std::string data{};
};

#endif //SLANGPARSER_TOKENTYPE_HPP
