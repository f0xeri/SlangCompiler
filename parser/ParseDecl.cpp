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
        context.moduleName = moduleName;
        moduleAST = create<ModuleDeclNode>(loc, moduleName, create<BlockStmtNode>(loc, std::vector<StmtPtrVariant>()));
        advance();
        while (token->type != TokenType::Start)
        {
            if (token->type == TokenType::Function || token->type == TokenType::Procedure || (token->type == TokenType::Extern && ((token + 1)->type == TokenType::Procedure || (token + 1)->type == TokenType::Function))) {
                auto funcDecl = parseFuncDecl();
                if (!funcDecl.has_value()) {
                    errors.emplace_back(filename, "Failed to parse function declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
            }
            else if (token->type == TokenType::Class || (token->type == TokenType::Extern && (token + 1)->type == TokenType::Class)) {
                auto type = parseClassDecl();
                if (!type.has_value()) {
                    errors.emplace_back(filename, "Failed to parse class declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
            }
            else if (token->type == TokenType::Variable || (token->type == TokenType::Extern && (token + 1)->type == TokenType::Variable)) {
                auto varDecl = parseVarDecl();
                if (!varDecl.has_value()) {
                    errors.emplace_back(filename, "Failed to parse variable declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
            }
            advance();
        }
        auto block = parseBlockStmt(moduleName);
        if (!block.has_value()) {
            errors.emplace_back(filename, "Failed to parse module block.", token->location, false, true);
            hasError = true;
            return std::nullopt;
        }
        auto moduleDecl = create<ModuleDeclNode>(loc, moduleName, std::move(block.value()));
        consume(TokenType::Dot);
        if (token->type != TokenType::EndOfFile) {
            errors.emplace_back(filename, "Expected end of file after end of module declaration.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        return moduleDecl;
    }

    auto Parser::parseVarDecl() -> std::optional<DeclPtrVariant> {
        token--;
        SourceLoc loc = token->location;
        bool isPrivate = false;
        bool isExtern = false;
        bool isGlobal = false;

        if (token->value == "private") {
            isPrivate = true;
            isGlobal = true;
        }
        else if (token->value == "public") {
            isGlobal = true;
        }
        advance();
        if (token->value == "extern") {
            isExtern = true;
            advance();
        }
        std::optional<DeclPtrVariant> result = std::nullopt;
        consume(TokenType::Variable);
        consume(TokenType::Minus);
        auto type = parseType();
        if (!type.has_value()) {
            errors.emplace_back(filename, "Expected typeExpr.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        auto name = consume(TokenType::Identifier).value;
        if (isGlobal && !isExtern) name = moduleAST->name + "." + name;
        std::optional<ExprPtrVariant> value;
        if (match(TokenType::Assign)) {
            value = parseExpr();
        }
        expect(TokenType::Semicolon);

        if (std::holds_alternative<ArrayExprPtr>(type.value())) {
            auto arrExpr = std::get<ArrayExprPtr>(type.value());
            auto indicesCount = arrExpr->getIndicesCount();
            if (!hasError) {
                result = createDecl<ArrayDecStatementNode>(loc, name, std::move(arrExpr), std::move(value), indicesCount, isGlobal, isPrivate, isExtern);
            }
        }
        else if (std::holds_alternative<FuncExprPtr>(type.value())) {
            auto funcExpr = std::get<FuncExprPtr>(type.value());
            funcExpr->isFunctionPtr = true;
            if (!hasError) {
                result = createDecl<FuncPointerStatementNode>(loc, name, funcExpr, std::move(value), funcExpr->isFunction, isGlobal, isPrivate, isExtern);
            }
        }
        else if (std::holds_alternative<TypeExprPtr>(type.value())) {
            auto typeExpr = std::get<TypeExprPtr>(type.value());
            if (!hasError) {
                result = createDecl<VarDecStatementNode>(loc, name, typeExpr->type, std::move(value), isGlobal, isPrivate, isExtern);
            }
        }
        if (result.has_value()) context.symbolTable.insert(name, moduleAST->name, result.value(), isPrivate, false);
        return result;
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
                errors.emplace_back(filename, "Failed to parse function parameter typeExpr.", token->location, false, false);
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
                errors.emplace_back(filename, "Expected comma or right parenthesis after function parameter declaration.", token->location, false, false);
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
            errors.emplace_back(filename, "Failed to parse function parameters.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }

        advance();
        ExprPtrVariant returnType;
        if (isFunction) {
            consume(TokenType::Colon);
            auto returnTypeOpt = parseType();
            if (!returnTypeOpt.has_value()) {
                errors.emplace_back(filename, "Failed to parse function return typeExpr.", token->location, false, false);
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

        auto funcExpr = create<FuncExprNode>(loc, returnType, params.value(), isFunction);
        funcDecl = createDecl<FuncDecStatementNode>(loc, mangledName, funcExpr, std::nullopt, isPrivate, isFunction, isExtern);
        context.symbolTable.insert(mangledName, moduleAST->name, funcDecl, isPrivate, false);

        --token;
        auto block = parseBlockStmt(name, &params.value());
        if (!block.has_value()) {
            errors.emplace_back(filename, "Failed to parse function block.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        if (!isExtern) {
            auto func = std::get<FuncDecStatementPtr>(funcDecl);
            func->block = std::move(block.value());
        }
        if (token->type != TokenType::Semicolon) {
            errors.emplace_back(filename, "Expected semicolon after function declaration.", token->location, false, false);
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
            errors.emplace_back(filename, "Failed to parse field typeExpr.", token->location, false, false);
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
                //context.insert(name, std::get<FieldArrayVarDecPtr>(result.assignExpr()));
            }
        }
        else if (std::holds_alternative<FuncExprPtr>(type.value())) {
            auto funcExpr = std::get<FuncExprPtr>(type.value());
            funcExpr->isFunctionPtr = true;
            if (!hasError) {
                result = createDecl<FieldFuncPointerStatementNode>(loc, name, typeName, fieldId, funcExpr, funcExpr->isFunction, isPrivate, std::move(value));
                //context.insert(name, std::get<FieldFuncPointerStmtPtr>(result.assignExpr()));
            }
        }
        else if (std::holds_alternative<TypeExprPtr>(type.value())) {
            auto typeExpr = std::get<TypeExprPtr>(type.value());
            if (!hasError) {
                result = createDecl<FieldVarDecNode>(loc, name, typeName, isPrivate, fieldId, typeExpr->type, std::move(value));
                //context.insert(name, std::get<FieldVarDecPtr>(result.assignExpr()));
            }
        }
        return result;
    }

    auto Parser::parseMethodDecl(const std::string& typeName, size_t vtableIndex) -> std::optional<DeclPtrVariant> {
        --token;
        SourceLoc loc = token->location;
        //context.enterScope();
        bool isPrivate = consume(TokenType::VisibilityType).value == "private";
        bool isVirtual = false;
        if (token->type == TokenType::Virtual) {
            isVirtual = true;
            advance();
        }
        consume(TokenType::Method);
        bool isFunction = false;
        std::string basicName = consume(TokenType::Identifier).value;
        std::string name = typeName + "." + basicName;
        consume(TokenType::LParen);
        auto tok = moduleAST->name + "." + consume(TokenType::Identifier).value;
        if (tok != typeName) {
            errors.emplace_back(filename, std::string("\"this\" should have \"" + typeName + "\" typeExpr, not " + tok), token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        auto thisName = consume(TokenType::Identifier).value;
        consume(TokenType::RParen);
        auto args = parseFuncParams();
        if (!args.has_value()) {
            errors.emplace_back(filename, "Failed to parse method arguments.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        args.value().insert(args.value().begin(), create<FuncParamDecStatementNode>(loc, thisName, ParameterType::Out, createExpr<TypeExprNode>(loc, typeName)));
        for (auto& arg : args.value()) {
            //context.insert(arg->name, arg);
        }
        advance();
        std::optional<ExprPtrVariant> returnType;
        if (token->type == TokenType::Colon) {
            advance();
            returnType = parseType();
            isFunction = true;
            if (!returnType.has_value()) {
                errors.emplace_back(filename, "Failed to parse method return typeExpr.", token->location, false, false);
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
        auto funcExpr = create<FuncExprNode>(loc, returnType.value(), args.value(), isFunction);
        auto funcDecl = create<MethodDecNode>(loc, name, funcExpr, thisName, block, isPrivate, isFunction, isVirtual, vtableIndex);
        //context.insert(name, funcDecl);
        --token;
        auto parsedBlock = parseBlockStmt(basicName);
        if (parsedBlock.has_value()) {
            block = std::move(parsedBlock.value());
        }
        else {
            errors.emplace_back(filename, "Failed to parse method body.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        consume(TokenType::Semicolon);
        funcDecl->block = std::move(block);
        //context.exitScope();
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
        auto mangledName = name;
        if (!isExtern) {
            mangledName = moduleAST->name + "." + name;
        }
        consume(TokenType::Inherits);
        auto parentTypeName = parseTypeName().value();
        auto classNode = createDecl<TypeDecStatementNode>(loc, mangledName, std::vector<DeclPtrVariant>{}, std::vector<MethodDecPtr>{}, parentTypeName, isPrivate, isExtern);
        //context.insert(name, classNode);
        //context.types.emplace_back(std::get<TypeDecStmtPtr>(classNode));
        uint32_t fieldIndex = 0;
        std::vector<DeclPtrVariant> fields;
        std::vector<MethodDecPtr> methods;
        advance();
        // if current type is inherited from another type, add invisible parent field at 0 index
        if (parentTypeName != "Object") {
            auto parentField = createDecl<FieldVarDecNode>(loc, "", mangledName, true, fieldIndex, parentTypeName, std::nullopt);
            ++fieldIndex;
            fields.emplace_back(parentField);
        }
        bool vtableRequired = false;
        size_t vtableIndex = 0;
        while (token->type != TokenType::End) {
            consume(TokenType::VisibilityType);
            if (token->type == TokenType::Field) {
                auto field = parseFieldDecl(mangledName, fieldIndex);
                ++fieldIndex;
                if (!field.has_value()) {
                    errors.emplace_back(filename, "Failed to parse field declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
                fields.emplace_back(field.value());
            }
            else if (token->type == TokenType::Method || token->type == TokenType::Virtual) {
                if (token->type == TokenType::Virtual) {
                    vtableRequired = true;
                }
                auto method = parseMethodDecl(mangledName, vtableIndex++);
                if (!method.has_value()) {
                    errors.emplace_back(filename, "Failed to parse method declaration.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
                methods.emplace_back(std::get<MethodDecPtr>(method.value()));
            }
        }
        advance();
        //context.exitScope();
        if (token->type == TokenType::Identifier && token->value == name) {
            std::get<TypeDecStmtPtr>(classNode)->fields = std::move(fields);
            std::get<TypeDecStmtPtr>(classNode)->methods = std::move(methods);
        }
        else {
            errors.emplace_back(filename, std::string("Expected end of " + name + ", got " + token->value + "."), token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        std::get<TypeDecStmtPtr>(classNode)->vtableRequired = vtableRequired;
        //context.types[mangledName] = std::get<TypeDecStmtPtr>(classNode);
        context.symbolTable.insert(mangledName, moduleAST->name, classNode, isPrivate, false);
        return classNode;
    }
} // Slangc