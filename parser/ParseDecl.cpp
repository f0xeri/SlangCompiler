//
// Created by f0xeri on 06.05.2023.
//

#include "Parser.hpp"

namespace Slangc {
    auto Parser::parseVarDecl(bool isGlobal) -> std::optional<DeclPtrVariant> {
        SourceLoc loc = token->location;
        consume(TokenType::Variable);
        consume(TokenType::Minus);
        expect(TokenType::Identifier);
        auto type = token->value;
        advance();
        std::string name;
        std::optional<ExprPtrVariant> value;
        if (oneOfDefaultTypes(type)) {
            name = consume(TokenType::Identifier).value;
        }
        else if (type == "array") {
            // ...
        }
        else if (type == "function" || type == "procedure") {
            // ...
        }
        else {
            // custom types...
        }
        if (expect(TokenType::Assign)) {
            advance();
            value = parseExpr();
        }
        consume(TokenType::Semicolon);
        auto result = createDecl<VarDecStatementNode>(loc, name, type, std::move(value));
        return result;
    }
} // Slangc