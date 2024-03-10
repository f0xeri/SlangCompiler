//
// Created by f0xeri on 04.05.2023.
//

#ifndef SLANGCREFACTORED_TOKENTYPE_HPP
#define SLANGCREFACTORED_TOKENTYPE_HPP

#include <cstdint>
#include <variant>
#include "common.hpp"

namespace Slangc {
    enum class TokenType : std::int8_t {
        Import,
        Module,
        Start,
        End,
        Class,
        Inherits,
        Base,
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

        Neg,
        Plus,
        Minus,
        Division,
        Multiplication,
        Remainder,

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
        Integer32,
        Float,
        Real,
        Boolean,
        String,
        Nil,

        EndOfFile,
        Delete,
        Extern,
        Virtual,
        New,
    };

    struct Token {
        TokenType type;
        std::string value;
        SourceLoc location;
    };
}

#endif //SLANGCREFACTORED_TOKENTYPE_HPP
