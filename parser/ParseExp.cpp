//
// Created by f0xeri on 06.05.2023.
//

#include "Parser.hpp"

namespace Slangc {
    auto Parser::parseExpr() -> std::optional<ExprPtrVariant> {
        return parseOr();
    }

    auto Parser::parseOr() -> std::optional<ExprPtrVariant> {
        auto lhs = parseAnd();
        while (match(TokenType::Or)) {
            auto opToken = prevToken();
            auto rhs = parseAnd().value();
            lhs = createExpr<OperatorExprNode>(opToken.location, TokenType::Or, std::move(lhs.value()), std::move(rhs));
        }
        return lhs;
    }

    auto Parser::parseAnd() -> std::optional<ExprPtrVariant> {
        auto lhs = parseEquality();
        while (match(TokenType::And)) {
            auto opToken = prevToken();
            auto rhs = parseEquality().value();
            lhs = createExpr<OperatorExprNode>(opToken.location, TokenType::And, std::move(lhs.value()), std::move(rhs));
        }
        return lhs;
    }

    auto Parser::parseEquality() -> std::optional<ExprPtrVariant> {
        auto lhs = parseCmp();
        while (match(TokenType::Equal) || match(TokenType::NotEqual)) {
            auto opToken = prevToken();
            auto rhs = parseCmp().value();
            lhs = createExpr<OperatorExprNode>(opToken.location, opToken.type, std::move(lhs.value()), std::move(rhs));
        }
        return lhs;
    }

    auto Parser::parseCmp() -> std::optional<ExprPtrVariant> {
        auto lhs = parseAddSub();
        while (match({TokenType::Less, TokenType::LessOrEqual, TokenType::Greater, TokenType::GreaterOrEqual})) {
            auto opToken = prevToken();
            auto rhs = parseAddSub().value();
            lhs = createExpr<OperatorExprNode>(opToken.location, opToken.type, std::move(lhs.value()), std::move(rhs));
        }
        return lhs;
    }

    auto Parser::parseAddSub() -> std::optional<ExprPtrVariant> {
        auto lhs = parseMulDiv();
        while (match({TokenType::Plus, TokenType::Minus})) {
            auto opToken = prevToken();
            auto rhs = parseMulDiv().value();
            lhs = createExpr<OperatorExprNode>(opToken.location, opToken.type, std::move(lhs.value()), std::move(rhs));
        }
        return lhs;
    }

    auto Parser::parseMulDiv() -> std::optional<ExprPtrVariant> {
        auto lhs = parseUnary();
        while (match({TokenType::Multiplication, TokenType::Division, TokenType::Remainder})) {
            auto opToken = prevToken();
            auto rhs = parseUnary().value();
            lhs = createExpr<OperatorExprNode>(opToken.location, opToken.type, std::move(lhs.value()), std::move(rhs));
        }
        return lhs;
    }

    auto Parser::parseUnary() -> std::optional<ExprPtrVariant> {
        if (match({TokenType::Minus, TokenType::Neg})) {
            auto opToken = prevToken();
            return createExpr<UnaryOperatorExprNode>(opToken.location, opToken.type, std::move(parseCall().value()));
        }
        return parseCall();
    }

    auto Parser::parseCall() -> std::optional<ExprPtrVariant> {
        auto expr = parsePrimary();
        while (true) {
            if (match(TokenType::LParen)) {
                auto opToken = prevToken();
                advance();
                std::vector<ExprPtrVariant> args;
                if (!match(TokenType::RParen)) {
                    do {
                        args.push_back(parseExpr().value());
                    } while (match(TokenType::Comma));
                }
                expect(TokenType::RParen);
                expr = createExpr<CallExprNode>(opToken.location, std::move(expr.value()), std::move(args));
            }
            else if (match(TokenType::Dot)) {
                break;
            }
            else if (match(TokenType::LBracket)) {
                break;
            }
            else {
                break;
            }
        }
        return expr;
    }

    auto Parser::parsePrimary() -> std::optional<ExprPtrVariant> {
        SourceLoc loc = token->location;
        auto tok = *token;
        advance();
        if (tok.type == TokenType::Boolean) return createExpr<BooleanExprNode>(loc, tok.value == "true");
        else if (tok.type == TokenType::Integer) return createExpr<IntExprNode>(loc, std::stoi(tok.value));
        else if (tok.type == TokenType::Real) return createExpr<RealExprNode>(loc, std::stod(tok.value));
        else if (tok.type == TokenType::Float) return createExpr<FloatExprNode>(loc, std::stof(tok.value));
        else if (tok.type == TokenType::String) {
            if (tok.value.size() == 1) return createExpr<CharExprNode>(loc, tok.value[0]);
            return createExpr<StringExprNode>(loc, tok.value);
        }
        else if (tok.type == TokenType::Identifier) {
            return parseVar();
        }
        else if (tok.type == TokenType::LParen) {
            auto expr = parseExpr().value();
            consume(TokenType::RParen);
            return expr;
        }
        errors.emplace_back("Failed to parse expression." + tok.value, loc);
        return std::nullopt;
    }

    auto Parser::parseVar() -> std::optional<ExprPtrVariant> {
        SourceLoc loc = token->location;
        expect(TokenType::Identifier);
        auto name = token->value;
        advance();
        // type access, array access, etc.
        return createExpr<VarExprNode>(loc, name);
    }

} // Slangc