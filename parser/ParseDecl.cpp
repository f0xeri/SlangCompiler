//
// Created by f0xeri on 06.05.2023.
//

#include "Parser.hpp"

namespace Slangc {
    auto Parser::parseModuleDecl() -> std::optional<ModuleDeclPtr> {
        SourceLoc loc = token->location;
        consume(TokenType::Module);
        expect(TokenType::Identifier);
        auto moduleName = token->value;
        advance();
        while (token->type != TokenType::Start)
        {
            if (token->type == TokenType::Function || token->type == TokenType::Procedure) {
                auto funcDecl = parseFuncDecl();
                if (!funcDecl.has_value()) {
                    errors.emplace_back("Failed to parse function declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
            }
            advance();
        }
        auto block = parseBlockStmt(moduleName);
        if (!block.has_value()) {
            errors.emplace_back("Failed to parse module block.", token->location, false, true);
            hasError = true;
            return std::nullopt;
        }
        auto moduleDecl = create<ModuleDeclNode>(loc, moduleName, std::move(block.value()));
        consume(TokenType::Dot);
        if (token->type != TokenType::EndOfFile) {
            errors.emplace_back("Expected end of file after end of module declaration.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        return moduleDecl;
    }

    auto Parser::parseVarDecl(bool isGlobal) -> std::optional<DeclPtrVariant> {
        return {};
    }

    auto Parser::parseFuncParams() -> std::optional<std::vector<FuncParamDecStmtPtr>> {
        auto params = std::vector<FuncParamDecStmtPtr>();
        consume(TokenType::LParen);
        while (token->type != TokenType::RParen) {
            if (token->type == TokenType::Identifier) {
                auto loc = token->location;
                ParameterType parameterType{};
                std::string ptype = consume(TokenType::Identifier).value;
                if (ptype == "in") parameterType = ParameterType::In;
                else if (ptype == "var") parameterType = ParameterType::Var;
                else if (ptype == "out") parameterType = ParameterType::Out;
                auto type = parseType();
                if (!type.has_value()) {
                    errors.emplace_back("Failed to parse function parameter type.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
                advance();
                auto name = consume(TokenType::Identifier).value;
                params.emplace_back(create<FuncParamDecStatementNode>(token->location, name, parameterType, std::move(type.value())));
                if (token->type == TokenType::Comma) {
                    advance();
                }
                else if (token->type != TokenType::RParen) {
                    errors.emplace_back("Expected comma or right parenthesis after function parameter declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
            }
            else {
                errors.emplace_back("Expected identifier in function parameter declaration.", token->location, false, false);
                hasError = true;
                return std::nullopt;
            }
        }
        return params;
    }

    auto Parser::parseFuncDecl() -> std::optional<DeclPtrVariant> {
        --token;
        SourceLoc loc = token->location;
        bool isPrivate = false;
        bool isExtern = false;
        bool isFunction = false;

        if (token->value == "private") isPrivate = true;
        advance();
        if (token->value == "extern") {
            isExtern = true;
            advance();
        }
        if (match(TokenType::Function)) {
            isFunction = true;
        }
        else {
            consume(TokenType::Procedure);
        }

        std::string name = consume(TokenType::Identifier).value;
        auto params = parseFuncParams();
        if (!params.has_value()) {
            errors.emplace_back("Failed to parse function parameters.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }

        ExprPtrVariant returnType;
        if (isFunction) {
            advance();
            consume(TokenType::Colon);
            auto returnTypeOpt = parseType();
            if (!returnTypeOpt.has_value()) {
                errors.emplace_back("Failed to parse function return type.", token->location, false, false);
                hasError = true;
                return std::nullopt;
            }
            returnType = std::move(returnTypeOpt.value());
        }
        else {
            returnType = createExpr<TypeExprNode>(loc, "void");
        }

        DeclPtrVariant funcDecl;
        auto mangledName = name;
        if (!isExtern) {
            mangledName = moduleAST->name + "." + name;
        }
        if (isExtern) {
            funcDecl = createDecl<ExternFuncDecStatementNode>(loc, mangledName, std::move(returnType), params.value());
        }
        else {
            funcDecl = createDecl<FuncDecStatementNode>(loc, mangledName, std::move(returnType), params.value());
        }
        analysis.insert(mangledName, funcDecl);
        auto block = parseBlockStmt(name, &params.value());
        if (!block.has_value()) {
            errors.emplace_back("Failed to parse function block.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }

        if (!isExtern) {
            auto func = std::get<FuncDecStatementPtr>(funcDecl);
            func->block = std::move(block.value());
        }
        if (token->type != TokenType::Semicolon) {
            errors.emplace_back("Expected semicolon after function declaration.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        return funcDecl;
    }
} // Slangc