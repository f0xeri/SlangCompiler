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
            if (token->type == TokenType::Function || token->type == TokenType::Procedure || (token->type == TokenType::Extern && ((token + 1)->type == TokenType::Procedure || (token + 1)->type == TokenType::Function))) {
                auto funcDecl = parseFuncDecl();
                if (!funcDecl.has_value()) {
                    errors.emplace_back("Failed to parse function declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
            }
            else if (token->type == TokenType::Class || (token->type == TokenType::Extern && (token + 1)->type == TokenType::Class)) {
                auto type = parseClassDecl();
                if (!type.has_value()) {
                    errors.emplace_back("Failed to parse class declaration.", token->location, false, false);
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

    auto Parser::parseFuncParams(bool named) -> std::optional<std::vector<FuncParamDecStmtPtr>> {
        auto params = std::vector<FuncParamDecStmtPtr>();
        consume(TokenType::LParen);
        while (token->type != TokenType::RParen) {
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
            std::string name;
            if (named) {
                name = consume(TokenType::Identifier).value;
            }
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

        advance();
        ExprPtrVariant returnType;
        if (isFunction) {
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
            funcDecl = createDecl<ExternFuncDecStatementNode>(loc, mangledName, std::move(returnType), params.value(), isPrivate, isFunction);
        }
        else {
            funcDecl = createDecl<FuncDecStatementNode>(loc, mangledName, std::move(returnType), params.value(), std::nullopt, isPrivate, isFunction);
        }
        analysis.insert(mangledName, funcDecl);
        --token;
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

    auto Parser::parseFieldDecl(const std::string& typeName, uint32_t fieldId) -> std::optional<DeclPtrVariant> {
        --token;
        std::optional<DeclPtrVariant> result = std::nullopt;
        SourceLoc loc = token->location;
        bool isPrivate = consume(TokenType::VisibilityType).value == "private";
        consume(TokenType::Field);
        consume(TokenType::Minus);
        auto type = parseType();
        if (!type.has_value()) {
            errors.emplace_back("Failed to parse field type.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        std::string name = consume(TokenType::Identifier).value;
        std::optional<ExprPtrVariant> value;
        if (match(TokenType::Assign)) {
            value = parseExpr();
        }
        consume(TokenType::Semicolon);

        if (std::holds_alternative<ArrayExprPtr>(type.value())) {
            auto arrExpr = std::get<ArrayExprPtr>(type.value());
            auto indicesCount = arrExpr->getIndicesCount();
            if (!hasError) {
                result = createDecl<FieldArrayVarDecNode>(loc, name, typeName, fieldId, std::move(arrExpr), std::move(value), indicesCount, isPrivate);
                analysis.insert(name, std::get<FieldArrayVarDecPtr>(result.value()));
            }
        }
        else if (std::holds_alternative<FuncExprPtr>(type.value())) {
            auto funcExpr = std::get<FuncExprPtr>(type.value());
            if (!hasError) {
                result = createDecl<FieldFuncPointerStatementNode>(loc, name, typeName, fieldId, std::move(funcExpr->type), std::move(funcExpr->params), funcExpr->isFunction, isPrivate, std::move(value));
                analysis.insert(name, std::get<FieldFuncPointerStatementPtr>(result.value()));
            }
        }
        else if (std::holds_alternative<TypeExprPtr>(type.value())) {
            auto typeExpr = std::get<TypeExprPtr>(type.value());
            if (!hasError) {
                result = createDecl<FieldVarDecNode>(loc, name, typeName, isPrivate, fieldId, typeExpr->type, std::move(value));
                analysis.insert(name, std::get<FieldVarDecPtr>(result.value()));
            }
        }
        return result;
    }

    auto Parser::parseMethodDecl(const std::string& typeName) -> std::optional<DeclPtrVariant> {
        --token;
        SourceLoc loc = token->location;
        analysis.enterScope();
        bool isPrivate = consume(TokenType::VisibilityType).value == "private";
        consume(TokenType::Method);
        bool isFunction = false;
        std::string basicName = consume(TokenType::Identifier).value;
        std::string name = moduleAST->name + "." + typeName + "." + basicName;
        consume(TokenType::LParen);
        auto tok = consume(TokenType::Identifier).value;
        if (tok != typeName) {
            errors.emplace_back(std::string("\"this\" should have \"" + typeName + "\" type, not " + tok), token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        auto thisName = consume(TokenType::Identifier).value;
        consume(TokenType::RParen);
        auto args = parseFuncParams();
        if (!args.has_value()) {
            errors.emplace_back("Failed to parse method arguments.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        args.value().insert(args.value().begin(), create<FuncParamDecStatementNode>(loc, thisName, ParameterType::Out, createExpr<TypeExprNode>(loc, typeName)));
        for (auto& arg : args.value()) {
            analysis.insert(arg->name, arg);
        }
        advance();
        std::optional<ExprPtrVariant> returnType;
        if (token->type == TokenType::Colon) {
            advance();
            returnType = parseType();
            isFunction = true;
            if (!returnType.has_value()) {
                errors.emplace_back("Failed to parse method return type.", token->location, false, false);
                hasError = true;
                return std::nullopt;
            }
        }
        else {
            returnType = createExpr<TypeExprNode>(loc, "void");
        }
        loc = token->location;
        std::vector<StmtPtrVariant> statements;
        auto block = create<BlockStmtNode>(loc, statements);
        auto funcDecl = create<MethodDecNode>(loc, name, std::move(returnType.value()), thisName, args.value(), block, isFunction, isPrivate);
        analysis.insert(name, funcDecl);
        --token;
        auto parsedBlock = parseBlockStmt(basicName);
        if (parsedBlock.has_value()) {
            block = std::move(parsedBlock.value());
        }
        else {
            errors.emplace_back("Failed to parse method body.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        consume(TokenType::Semicolon);
        funcDecl->block = std::move(block);
        analysis.exitScope();
        return funcDecl;
    }

    auto Parser::parseClassDecl() -> std::optional<DeclPtrVariant> {
        --token;
        SourceLoc loc = token->location;
        bool isPrivate = consume(TokenType::VisibilityType).value == "private";
        bool isExtern = false;
        if (token->type == TokenType::Extern) {
            isExtern = true;
            advance();
        }
        consume(TokenType::Class);
        std::string name = consume(TokenType::Identifier).value;
        consume(TokenType::Inherits);
        auto parent = analysis.lookup(parseTypeName().value());
        if (parent == nullptr || !std::holds_alternative<TypeDecStatementPtr>(*parent)) {
            errors.emplace_back("Parent class is not declared.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        auto classNode = createDecl<TypeDecStatementNode>(loc, name, std::vector<DeclPtrVariant>{}, std::vector<MethodDecPtr>{}, std::get<TypeDecStatementPtr>(*parent), isPrivate, isExtern);
        analysis.insert(name, classNode);
        uint32_t fieldIndex = 0;
        std::vector<DeclPtrVariant> fields;
        std::vector<MethodDecPtr> methods;
        advance();
        while (token->type != TokenType::End) {
            consume(TokenType::VisibilityType);
            if (token->type == TokenType::Field) {
                auto field = parseFieldDecl(name, fieldIndex);
                ++fieldIndex;
                if (!field.has_value()) {
                    errors.emplace_back("Failed to parse field declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
                fields.emplace_back(field.value());
            }
            else if (token->type == TokenType::Method) {
                auto method = parseMethodDecl(name);
                if (!method.has_value()) {
                    errors.emplace_back("Failed to parse method declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
                methods.emplace_back(std::get<MethodDecPtr>(method.value()));
            }
        }
        advance();
        analysis.exitScope();
        if (token->type == TokenType::Identifier && token->value == name) {
            std::get<TypeDecStatementPtr>(classNode)->fields = std::move(fields);
            std::get<TypeDecStatementPtr>(classNode)->methods = std::move(methods);
        }
        else {
            errors.emplace_back(std::string("Expected end of " + name + ", got " + token->value + "."), token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        return classNode;
    }
} // Slangc