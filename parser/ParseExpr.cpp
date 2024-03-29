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
        if (!lhs.has_value()) {
            return std::nullopt;
        }
        while (match(TokenType::Or)) {
            auto opToken = prevToken();
            auto rhs = parseAnd();
            if (!rhs.has_value()) {
                errors.emplace_back(filename, "Expected expression after operator.", opToken.location, false, false);
                hasError = true;
                return std::nullopt;
            }
            lhs = createExpr<OperatorExprNode>(opToken.location, TokenType::Or, std::move(lhs.value()), std::move(rhs.value()));
        }
        return lhs;
    }

    auto Parser::parseAnd() -> std::optional<ExprPtrVariant> {
        auto lhs = parseEquality();
        if (!lhs.has_value()) {
            return std::nullopt;
        }
        while (match(TokenType::And)) {
            auto opToken = prevToken();
            auto rhs = parseEquality();
            if (!rhs.has_value()) {
                errors.emplace_back(filename, "Expected expression after operator.", opToken.location, false, false);
                hasError = true;
                return std::nullopt;
            }
            lhs = createExpr<OperatorExprNode>(opToken.location, TokenType::And, std::move(lhs.value()), std::move(rhs.value()));
        }
        return lhs;
    }

    auto Parser::parseEquality() -> std::optional<ExprPtrVariant> {
        auto lhs = parseCmp();
        if (!lhs.has_value()) {
            return std::nullopt;
        }
        while (match(TokenType::Equal) || match(TokenType::NotEqual)) {
            auto opToken = prevToken();
            auto rhs = parseCmp();
            if (!rhs.has_value()) {
                errors.emplace_back(filename, "Expected expression after operator.", opToken.location, false, false);
                hasError = true;
                return std::nullopt;
            }
            lhs = createExpr<OperatorExprNode>(opToken.location, opToken.type, std::move(lhs.value()), std::move(rhs.value()));
        }
        return lhs;
    }

    auto Parser::parseCmp() -> std::optional<ExprPtrVariant> {
        auto lhs = parseAddSub();
        if (!lhs.has_value()) {
            return std::nullopt;
        }
        while (match({TokenType::Less, TokenType::LessOrEqual, TokenType::Greater, TokenType::GreaterOrEqual})) {
            auto opToken = prevToken();
            auto rhs = parseAddSub();
            if (!rhs.has_value()) {
                errors.emplace_back(filename, "Expected expression after operator.", opToken.location, false, false);
                hasError = true;
                return std::nullopt;
            }
            lhs = createExpr<OperatorExprNode>(opToken.location, opToken.type, std::move(lhs.value()), std::move(rhs.value()));
        }
        return lhs;
    }

    auto Parser::parseAddSub() -> std::optional<ExprPtrVariant> {
        auto lhs = parseMulDiv();
        if (!lhs.has_value()) {
            return std::nullopt;
        }
        while (match({TokenType::Plus, TokenType::Minus})) {
            auto opToken = prevToken();
            auto rhs = parseMulDiv();
            if (!rhs.has_value()) {
                errors.emplace_back(filename, "Expected expression after operator.", opToken.location, false, false);
                hasError = true;
                return std::nullopt;
            }
            lhs = createExpr<OperatorExprNode>(opToken.location, opToken.type, std::move(lhs.value()), std::move(rhs.value()));
        }
        return lhs;
    }

    auto Parser::parseMulDiv() -> std::optional<ExprPtrVariant> {
        auto lhs = parseUnary();
        if (!lhs.has_value()) {
            return std::nullopt;
        }
        while (match({TokenType::Multiplication, TokenType::Division, TokenType::Remainder})) {
            auto opToken = prevToken();
            auto rhs = parseUnary();
            if (!rhs.has_value()) {
                errors.emplace_back(filename, "Expected expression after operator.", opToken.location, false, false);
                hasError = true;
                return std::nullopt;
            }
            lhs = createExpr<OperatorExprNode>(opToken.location, opToken.type, std::move(lhs.value()), std::move(rhs.value()));
        }
        return lhs;
    }

    auto Parser::parseUnary() -> std::optional<ExprPtrVariant> {
        if (match({TokenType::Minus, TokenType::Neg})) {
            auto opToken = prevToken();
            auto expr = parseCall();
            if (!expr.has_value()) {
                errors.emplace_back(filename, "Expected expression after operator.", opToken.location, false, false);
                hasError = true;
                return std::nullopt;
            }
            return createExpr<UnaryOperatorExprNode>(opToken.location, opToken.type, std::move(expr.value()));
        }
        return parseCall();
    }

    auto Parser::parseCall() -> std::optional<ExprPtrVariant> {
        auto expr = parsePrimary();
        if (!expr.has_value()) {
            return std::nullopt;
        }
        while (true) {
            if (match(TokenType::LParen)) {
                auto opToken = prevToken();
                std::vector<ExprPtrVariant> args;
                if (!match(TokenType::RParen)) {
                    do {
                        auto argExpr = parseExpr();
                        if (!argExpr.has_value()) {
                            errors.emplace_back(filename, "Expected expression after '('.", opToken.location, false, false);
                            hasError = true;
                            return std::nullopt;
                        }
                        args.push_back(std::move(argExpr.value()));
                    } while (match(TokenType::Comma));
                    consume(TokenType::RParen);
                }
                expr = createExpr<CallExprNode>(opToken.location, std::move(expr.value()), std::move(args));
            }
            else if (match(TokenType::Dot)) {
                auto opToken = prevToken();
                auto name = consume(TokenType::Identifier).value;
                if (std::holds_alternative<VarExprPtr>(expr.value()) && std::ranges::find(context.imports, std::get<VarExprPtr>(expr.value())->name) != context.imports.end()) {
                    std::get<VarExprPtr>(expr.value())->name += "." + name;
                }
                else expr = createExpr<AccessExprNode>(opToken.location, std::move(expr.value()), name);
            }
            else if (match(TokenType::LBracket)) {
                auto opToken = prevToken();
                do {
                    auto indexExpr = parseExpr();
                    if (!indexExpr.has_value()) {
                        errors.emplace_back(filename, "Expected expression after '['.", opToken.location, false, false);
                        hasError = true;
                        return std::nullopt;
                    }
                    consume(TokenType::RBracket);
                    expr = createExpr<IndexExprNode>(opToken.location, std::move(expr.value()), std::move(indexExpr.value()));
                } while (match(TokenType::LBracket));
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
        else if (tok.type == TokenType::Nil) return createExpr<NilExprNode>(loc);
        else if (tok.type == TokenType::String) {
            return parseString();
        }
        else if (tok.type == TokenType::Identifier) {
            return parseVar();
        }
        else if (tok.type == TokenType::New) {
            return parseNew();
        }
        else if (tok.type == TokenType::LParen) {
            auto expr = parseExpr().value();
            consume(TokenType::RParen);
            return expr;
        }
        errors.emplace_back(filename, "Failed to parse expression.", loc);
        return std::nullopt;
    }

    auto Parser::parseVar() -> std::optional<ExprPtrVariant> {
        --token;
        SourceLoc loc = token->location;
        expect(TokenType::Identifier);
        auto name = token->value;
        advance();
        return createExpr<VarExprNode>(loc, name);
    }

    auto Parser::parseVarExpr() -> std::optional<ExprPtrVariant> {
        return parseAccess();
    }
    auto Parser::parseAccess() -> std::optional<ExprPtrVariant> {
        advance();              // because parseVar() makes token--.
        auto expr = parseVar();
        while (true) {
            if (match(TokenType::Dot)) {
                auto opToken = prevToken();
                auto name = consume(TokenType::Identifier).value;
                if (std::holds_alternative<VarExprPtr>(expr.value()) && std::ranges::find(context.imports, std::get<VarExprPtr>(expr.value())->name) != context.imports.end()) {
                    std::get<VarExprPtr>(expr.value())->name += "." + name;
                }
                else
                    expr = createExpr<AccessExprNode>(opToken.location, std::move(expr.value()), name);
            }
            else if (match(TokenType::LBracket)) {
                auto opToken = prevToken();
                do {
                    auto indexExpr = parseExpr();
                    if (!indexExpr.has_value()) {
                        errors.emplace_back(filename, "Expected expression after '['.", opToken.location, false, false);
                        hasError = true;
                        return std::nullopt;
                    }
                    consume(TokenType::RBracket);
                    expr = createExpr<IndexExprNode>(opToken.location, std::move(expr.value()), std::move(indexExpr.value()));
                } while (match(TokenType::LBracket));
            }
            else {
                break;
            }
        }
        return expr;
    }

    auto Parser::parseString() -> std::optional<ExprPtrVariant> {
        auto tok = prevToken();
        SourceLoc loc = tok.location;

        if (tok.value.starts_with("f\"")) {
            auto value = tok.value.substr(2, tok.value.size() - 3);
            std::vector<ExprPtrVariant> values;
            std::string currentValue;
            bool closedFmt = true;
            for (size_t i = 0; i < value.size(); ++i) {
                auto ch = value[i];
                // if {{ then push { to currentValue
                if (ch == '{') {
                    if (value.size() > i + 1 && value[i + 1] == '{') {
                        currentValue += ch;
                        i += 1;
                    }
                    else if (!currentValue.empty()) {
                        values.push_back(createExpr<StringExprNode>(loc, currentValue));
                        currentValue.clear();
                        closedFmt = false;
                    }
                }
                else if (ch == '}') {
                    if (closedFmt && value.size() > i + 1 && value[i + 1] == '}') {
                        currentValue += ch;
                        i += 1;
                    }
                    else
                    if (!currentValue.empty()) {
                        auto buffer = SourceBuffer::CreateFromString(currentValue);
                        Lexer lexer(std::move(buffer), errors);
                        lexer.tokenize();
                        Parser parser(filepath, lexer.tokens, driver, context, errors);
                        auto expr = parser.parseExpr();
                        if (expr.has_value() && parser.token == parser.tokens.end() - 1) {
                            values.push_back(std::move(expr.value()));
                        }
                        else {
                            errors.emplace_back(filename, "Failed to parse expression in formatted string literal.", loc);
                            hasError = true;
                        }
                        currentValue.clear();
                        closedFmt = true;
                    }
                }
                else {
                    currentValue += ch;
                }
            }
            if (!currentValue.empty()) {
                values.push_back(createExpr<StringExprNode>(loc, currentValue));
            }
            if (!closedFmt) {
                errors.emplace_back(filename, "Failed to parse formatted string literal: missing closing '}'.", loc);
                hasError = true;
            }
            return createExpr<FormattedStringExprNode>(loc, std::move(values));
        }
        else {
            auto value = tok.value.substr(1, tok.value.size() - 2);
            if (value.size() == 1)
                return createExpr<CharExprNode>(loc, value[0]);
            else
                return createExpr<StringExprNode>(loc, value);
        }
        return std::nullopt;
    }

    auto Parser::parseNew() -> std::optional<ExprPtrVariant> {
        auto tok = prevToken();
        SourceLoc loc = tok.location;
        auto type = parseType();
        if (!type.has_value()) {
            errors.emplace_back(filename, "Failed to parse type expression.", loc);
            hasError = true;
            return std::nullopt;
        }
        return createExpr<NewExprNode>(loc, std::move(type.value()));
    }

} // Slangc