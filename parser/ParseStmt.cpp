//
// Created by f0xeri on 06.05.2023.
//

#include "Parser.hpp"

#define PARSE_STMT(type) \
    if (auto stmt = parse##type(); stmt.has_value()) { \
        block->statements.emplace_back(std::move(stmt.value())); \
    } else { \
        errors.emplace_back(filename, "Failed to parse " #type ".", token->location, false, false); \
        hasError = true; \
    }

namespace Slangc {
    auto Parser::parseBlockStmt(const std::string &name, std::vector<FuncParamDecStmtPtr> *args) -> std::optional<BlockStmtPtr> {
        //context.enterScope();
        if (args != nullptr) {
            for (auto &arg : *args) {
                //context.insert(arg->name, arg);
            }
        }
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
            else if (name == "if" && (token->type == TokenType::Else || token->type == TokenType::Elseif)) {
                //context.exitScope();
                return block;
            }
            else if (token->type == TokenType::End) {
                advance();
                expect(TokenType::Identifier);
                auto endName = token->value;
                if (token->value != moduleAST->name) {
                    errors.emplace_back(filename,
                                        std::string("Expected end of module " + name + ", got " + endName + "."),
                                        token->location, false, false);
                    hasError = true;
                }
                advance();
                return block;
            }
            else if (token->type == TokenType::RBrace) {
                advance();
                return block;
            }
            else if (token->type == TokenType::EndOfFile) {
                errors.emplace_back(filename, "Unexpected end of file.", token->location, false, false);
                hasError = true;
                //context.exitScope();
                return block;
            } else {
                errors.emplace_back(filename, "Unexpected token " + std::string(Lexer::getTokenName(token->type)) + ".",
                                    token->location, false, false);
                hasError = true;
                //context.exitScope();
                return block;
            }
            if (result.has_value()) {
                block->statements.emplace_back(std::move(result.value()));
            }
            else {
                errors.emplace_back(filename, "Failed to parse statement.", token->location, false, false);
                hasError = true;
                //context.exitScope();
                return block;
            }
        }
    }

    auto Parser::parseVarStmt() -> std::optional<StmtPtrVariant> {
        SourceLoc loc = token->location;
        std::optional<StmtPtrVariant> result = std::nullopt;
        consume(TokenType::Variable);

        auto type = parseType();
        if (!type.has_value()) {
            errors.emplace_back(filename, "Expected typeExpr.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        auto name = consume(TokenType::Identifier).value;
        std::optional<ExprPtrVariant> value;
        if (match(TokenType::Assign)) {
            value = parseExpr();
        }
        consume(TokenType::Semicolon);

        if (std::holds_alternative<ArrayExprPtr>(type.value())) {
            auto arrExpr = std::get<ArrayExprPtr>(type.value());
            auto indicesCount = arrExpr->getIndicesCount();
            if (!hasError) {
                result = createStmt<ArrayDecStatementNode>(loc, name, std::move(arrExpr), std::move(value), indicesCount);
            }
        }
        else if (std::holds_alternative<FuncExprPtr>(type.value())) {
            auto funcExpr = std::get<FuncExprPtr>(type.value());
            funcExpr->isFunctionPtr = true;
            if (!hasError) {
                result = createStmt<FuncPointerStatementNode>(loc, name, funcExpr, std::move(value), funcExpr->isFunction);
            }
        }
        else if (std::holds_alternative<TypeExprPtr>(type.value())) {
            auto typeExpr = std::get<TypeExprPtr>(type.value());
            if (!hasError) {
                result = createStmt<VarDecStatementNode>(loc, name, typeExpr->type, std::move(value));
            }
        }
        return result;
    }

    auto Parser::parseIfStmt() -> std::optional<StmtPtrVariant> {
        auto loc = token->location;
        consume(TokenType::If);
        auto condition = parseExpr();
        if (!condition.has_value()) {
            errors.emplace_back(filename, "Expected expression.", token->location, false, false);
            hasError = true;
            return {};
        }
        expect(TokenType::LBrace);
        std::optional<BlockStmtPtr> trueBlock = std::nullopt;
        std::optional<BlockStmtPtr> falseBlock = std::nullopt;
        auto elseIfNodes = std::vector<ElseIfStatementPtr>();

        if (token->type != TokenType::End && token->type != TokenType::Else) {
            trueBlock = parseBlockStmt("if");
        }
        while (token->type == TokenType::Elseif) {
            advance();
            auto elseIfCondition = parseExpr();
            if (!elseIfCondition.has_value()) {
                errors.emplace_back(filename, "Expected expression.", token->location, false, false);
                hasError = true;
                return {};
            }
            expect(TokenType::LBrace);
            auto elseIfBlock = parseBlockStmt("if");
            elseIfNodes.emplace_back(create<ElseIfStatementNode>(loc, std::move(elseIfCondition.value()), std::move(elseIfBlock.value())));
        }
        if (token->type == TokenType::Else) {
            advance();
            expect(TokenType::LBrace);
            falseBlock = parseBlockStmt("else");
        }
        consume(TokenType::Semicolon);
        return createStmt<IfStatementNode>(loc, std::move(condition.value()), std::move(trueBlock.value()), std::move(falseBlock), std::move(elseIfNodes));
    }

    auto Parser::parseWhileStmt() -> std::optional<StmtPtrVariant> {
        auto loc = token->location;
        consume(TokenType::While);
        auto condition = parseExpr();
        expect(TokenType::LBrace);
        auto block = parseBlockStmt("while");
        consume(TokenType::Semicolon);
        if (condition.has_value() && block.has_value()) {
            return createStmt<WhileStatementNode>(loc, std::move(condition.value()), std::move(block.value()));
        }
        errors.emplace_back(filename, "Failed to parse while statement.", loc, false, false);
        hasError = true;
        return std::nullopt;
    }

    auto Parser::parseOutputStmt() -> std::optional<StmtPtrVariant> {
        auto loc = token->location;
        consume(TokenType::Output);
        auto expr = parseExpr();
        consume(TokenType::Semicolon);
        if (expr.has_value()) {
            return createStmt<OutputStatementNode>(loc, std::move(expr.value()));
        }
        errors.emplace_back(filename, "Failed to parse output statement.", loc, false, false);
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
        errors.emplace_back(filename, "Failed to parse input statement.", loc, false, false);
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
        errors.emplace_back(filename, "Failed to parse let statement.", loc, false, false);
        hasError = true;
        return std::nullopt;
    }

    auto Parser::parseReturnStmt() -> std::optional<StmtPtrVariant> {
        auto loc = token->location;
        consume(TokenType::Return);
        if (token->type == TokenType::Semicolon) {
            advance();
            return createStmt<ReturnStatementNode>(loc, create<TypeExprNode>(loc, "void"));
        }
        auto expr = parseExpr();
        consume(TokenType::Semicolon);
        if (expr.has_value()) {
            return createStmt<ReturnStatementNode>(loc, std::move(expr.value()));
        }
        errors.emplace_back(filename, "Failed to parse return statement.", loc, false, false);
        hasError = true;
        return std::nullopt;
    }

    auto Parser::parseCallStmt() -> std::optional<StmtPtrVariant> {
        consume(TokenType::Call);
        auto expr = parseExpr();
        consume(TokenType::Semicolon);
        // check if expr has assignExpr and is a call expr
        if (expr.has_value() && std::holds_alternative<CallExprPtr>(expr.value())) {
            // move expr from variant to caller
            //auto x = std::get<CallExprPtr>(expr.assignExpr());
            //auto t = x->getType(check);
            return std::move(std::get<CallExprPtr>(expr.value()));
        }
        errors.emplace_back(filename, "Failed to parse call statement.", token->location, false, false);
        hasError = true;
        return std::nullopt;
    }

    auto Parser::parseDeleteStmt() -> std::optional<StmtPtrVariant> {
        consume(TokenType::Delete);
        auto expr = parseExpr();
        consume(TokenType::Semicolon);
        if (expr.has_value()) {
            return createStmt<DeleteStmtNode>(token->location, std::move(expr.value()));
        }
        errors.emplace_back(filename, "Failed to parse delete statement.", token->location, false, false);
        hasError = true;
        return std::nullopt;
    }
} // Slangc