//
// Created by f0xeri on 06.05.2023.
//

#include "Parser.hpp"

namespace Slangc {
    auto Parser::parseVarStmt() -> std::optional<StmtPtrVariant> {
        SourceLoc loc = token->location;
        std::optional<StmtPtrVariant> result = std::nullopt;
        consume(TokenType::Variable);
        consume(TokenType::Minus);
        expect(TokenType::Identifier);
        auto type = token->value;
        advance();
        std::string name;
        std::optional<ExprPtrVariant> value;
        if (oneOfDefaultTypes(type)) {
            name = consume(TokenType::Identifier).value;
            if (match(TokenType::Assign)) {
                value = parseExpr();
            }
            result = createStmt<VarDecStatementNode>(loc, name, type, std::move(value));
        }
        else if (type == "array") {
            auto indicesCount = 1;
            consume(TokenType::LBracket);
            auto size = parseExpr().value();
            consume(TokenType::RBracket);
            auto arrayType = token->value;
            auto arrExpr = create<ArrayExprNode>(loc, std::vector<ExprPtrVariant>(), arrayType, std::move(size));
            while (token->value == "array") {
                advance();
                indicesCount++;
                consume(TokenType::LBracket);
                size = parseExpr().value();
                consume(TokenType::RBracket);

                if (token->type == TokenType::Identifier && token->value != "array") {
                    arrayType = token->value;
                    advance();
                }
                arrExpr->values.emplace_back(createExpr<ArrayExprNode>(loc, std::vector<ExprPtrVariant>(), arrayType, std::move(size)));
            }
            name = consume(TokenType::Identifier).value;
            if (match(TokenType::Assign)) {
                value = parseExpr();
            }
            result = createStmt<ArrayDecStatementNode>(loc, name, std::move(arrExpr), std::move(value), indicesCount);
        }
        else if (type == "function" || type == "procedure") {
            // ...
        }
        else {
            // custom types...
        }
        consume(TokenType::Semicolon);
        return result;
    }

    auto Parser::parseBlockStmt(const std::string& name) -> std::optional<BlockStmtPtr> {
        auto block = create<BlockStmtNode>(token->location, std::vector<StmtPtrVariant>());
        bool blockEnd = false;
        advance();
        while (!blockEnd) {
            if (token->type == TokenType::Variable) {
                auto varStmt = parseVarStmt();
                if (varStmt.has_value()) {
                    block->statements.emplace_back(std::move(varStmt.value()));
                }
                else {
                    errors.emplace_back("Failed to parse variable declaration.", token->location, false, false);
                    hasError = true;
                }
            }
            else if (token->type == TokenType::End) {
                blockEnd = true;
                advance();
                expect(TokenType::Identifier);
                auto endName = token->value;
                if (endName != name) {
                    errors.emplace_back(std::string("Expected end of block " + name + ", got " + endName + "."), token->location, false, false);
                    return block;
                }
            }
            else if (token->type == TokenType::EndOfFile) {
                errors.emplace_back("Unexpected end of file.", token->location, false, false);
                hasError = true;
                return block;
            }
            else {
                errors.emplace_back("Unexpected token " + std::string(Lexer::getTokenName(token->type)) + ".", token->location, false, false);
                hasError = true;
                return block;
            }

        }
        return block;
    }
} // Slangc