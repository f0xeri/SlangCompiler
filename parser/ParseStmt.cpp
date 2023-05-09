//
// Created by f0xeri on 06.05.2023.
//

#include "Parser.hpp"

#define PARSE_STMT(type) \
    if (auto stmt = parse##type(); stmt.has_value()) { \
        block->statements.emplace_back(std::move(stmt.value())); \
    } else { \
        errors.emplace_back("Failed to parse " #type ".", token->location, false, false); \
        hasError = true; \
    }

namespace Slangc {
    auto Parser::parseBlockStmt(const std::string &name) -> std::optional<BlockStmtPtr> {
        auto block = create<BlockStmtNode>(token->location, std::vector<StmtPtrVariant>());
        std::optional<StmtPtrVariant> result = std::nullopt;
        advance();
        while (true) {
            if (token->type == TokenType::Variable) result = parseVarStmt();
            else if (token->type == TokenType::If) result = parseIfStmt();
            else if (token->type == TokenType::While) result = parseWhileStmt();
            else if (token->type == TokenType::Output) result = parseOutputStmt();
            else if (token->type == TokenType::Input) result = parseInputStmt();
            else if (token->type == TokenType::Let) result = parseLetStmt();
            else if (token->type == TokenType::Return) result = parseReturnStmt();
            else if (token->type == TokenType::Call) result = parseCallStmt();
            else if (token->type == TokenType::Delete) result = parseDeleteStmt();
            else if (token->type == TokenType::End) {
                advance();
                expect(TokenType::Identifier);
                auto endName = token->value;
                if (endName != name) {
                    errors.emplace_back(std::string("Expected end of block " + name + ", got " + endName + "."), token->location, false, false);
                } else { advance(); }
                return block;
            } else if (token->type == TokenType::EndOfFile) {
                errors.emplace_back("Unexpected end of file.", token->location, false, false);
                hasError = true;
                return block;
            } else {
                errors.emplace_back("Unexpected token " + std::string(Lexer::getTokenName(token->type)) + ".",
                                    token->location, false, false);
                hasError = true;
                return block;
            }
            if (result.has_value()) {
                block->statements.emplace_back(std::move(result.value()));
            }
            else {
                errors.emplace_back("Failed to parse statement.", token->location, false, false);
                hasError = true;
                return block;
            }
        }
    }

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
        } else if (type == "array") {
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
                arrExpr->values.emplace_back(
                        createExpr<ArrayExprNode>(loc, std::vector<ExprPtrVariant>(), arrayType, std::move(size)));
            }
            name = consume(TokenType::Identifier).value;
            if (match(TokenType::Assign)) {
                value = parseExpr();
            }
            result = createStmt<ArrayDecStatementNode>(loc, name, std::move(arrExpr), std::move(value), indicesCount);
        } else if (type == "function" || type == "procedure") {
            // ...
        } else {
            // custom types...
        }
        consume(TokenType::Semicolon);
        return result;
    }

    auto Parser::parseIfStmt() -> std::optional<StmtPtrVariant> {
        return {};
    }

    auto Parser::parseWhileStmt() -> std::optional<StmtPtrVariant> {
        return {};
    }

    auto Parser::parseOutputStmt() -> std::optional<StmtPtrVariant> {
        auto loc = token->location;
        consume(TokenType::Output);
        auto expr = parseExpr();
        consume(TokenType::Semicolon);
        if (expr.has_value()) {
            return createStmt<OutputStatementNode>(loc, std::move(expr.value()));
        }
        errors.emplace_back("Failed to parse output statement.", loc, false, false);
        hasError = true;
        return std::nullopt;
    }

    auto Parser::parseInputStmt() -> std::optional<StmtPtrVariant> {
        auto loc = token->location;
        consume(TokenType::Input);
        auto expr = parseVarExpr();
        consume(TokenType::Semicolon);
        if (expr.has_value()) {
            return createStmt<InputStatementNode>(loc, std::move(expr.value()));
        }
        errors.emplace_back("Failed to parse input statement.", loc, false, false);
        hasError = true;
        return std::nullopt;
    }

    auto Parser::parseLetStmt() -> std::optional<StmtPtrVariant> {
        auto loc = token->location;
        consume(TokenType::Let);
        auto varExpr = parseVarExpr();
        consume(TokenType::Assign);
        auto value = parseExpr();
        consume(TokenType::Semicolon);
        if (varExpr.has_value() && value.has_value()) {
            return createStmt<AssignExprNode>(loc, std::move(varExpr.value()), std::move(value.value()));
        }
        errors.emplace_back("Failed to parse let statement.", loc, false, false);
        hasError = true;
        return std::nullopt;
    }

    auto Parser::parseReturnStmt() -> std::optional<StmtPtrVariant> {
        return {};
    }

    auto Parser::parseCallStmt() -> std::optional<StmtPtrVariant> {
        return {};
    }

    auto Parser::parseDeleteStmt() -> std::optional<StmtPtrVariant> {
        return {};
    }
} // Slangc