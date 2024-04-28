//
// Created by f0xeri on 06.05.2023.
//

#include "Parser.hpp"

namespace Slangc {
    auto Parser::parseModuleDecl() -> std::optional<ModuleDeclPtr> {
        SourceLoc loc = token->location;
        consume(TokenType::Module);
        auto moduleName = consume(TokenType::Identifier).value;
        context.moduleName = moduleName;
        moduleAST = create<ModuleDeclNode>(loc, moduleName, create<BlockStmtNode>(loc, std::vector<StmtPtrVariant>()));
        advance();
        while (prevToken().type != TokenType::Start && prevToken().type != TokenType::EndOfFile)
        {
            if (token->type == TokenType::Function || (token->type == TokenType::Extern && (token + 1)->type == TokenType::Function)) {
                auto funcDecl = parseFuncDecl();
                if (!funcDecl.has_value()) {
                    SLANGC_LOG(filename, "Failed to parse function declaration.", token->location, LogLevel::Error, false);
                    hasError = true;
                    return std::nullopt;
                }
            }
            else if (token->type == TokenType::Class || (token->type == TokenType::Extern && (token + 1)->type == TokenType::Class)) {
                auto type = parseClassDecl();
                if (!type.has_value()) {
                    SLANGC_LOG(filename, "Failed to parse class declaration.", token->location, LogLevel::Error, false);
                    hasError = true;
                    return std::nullopt;
                }
            }
            else if (token->type == TokenType::Variable || (token->type == TokenType::Extern && (token + 1)->type == TokenType::Variable)) {
                auto varDecl = parseVarDecl();
                if (!varDecl.has_value()) {
                    SLANGC_LOG(filename, "Failed to parse variable declaration.", token->location, LogLevel::Error, false);
                    hasError = true;
                    return std::nullopt;
                }
            }
            else {
                SLANGC_LOG(filename, "Unexpected token. Failed to parse module declaration.", token->location, LogLevel::Error, false);
                hasError = true;
                return std::nullopt;
            }
            advance();
        }
        --token;
        auto block = parseBlockStmt(moduleName);
        if (!block.has_value()) {
            SLANGC_LOG(filename, "Failed to parse module block.", token->location, LogLevel::Error, true);
            hasError = true;
            return std::nullopt;
        }
        auto moduleDecl = create<ModuleDeclNode>(loc, moduleName, std::move(block.value()));
        consume(TokenType::Dot);
        if (token->type != TokenType::EndOfFile) {
            SLANGC_LOG(filename, "Expected end of file after end of module declaration.", token->location, LogLevel::Error, false);
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

        auto type = parseType();
        if (!type.has_value()) {
            SLANGC_LOG(filename, "Expected typeExpr.", token->location, LogLevel::Error, false);
            hasError = true;
            return std::nullopt;
        }
        auto name = consume(TokenType::Identifier).value;
        if (isGlobal && !isExtern) name = moduleAST->name + "." + name;
        std::optional<ExprPtrVariant> value;
        if (match(TokenType::Assign)) {
            value = parseExpr();
        }
        consume(TokenType::Semicolon);

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
                SLANGC_LOG(filename, "Failed to parse function parameter typeExpr.", token->location, LogLevel::Error, false);
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
                SLANGC_LOG(filename, "Expected comma or right parenthesis after function parameter declaration.", token->location, LogLevel::Error, false);
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
        consume(TokenType::Function);

        std::string name = consume(TokenType::Identifier).value;
        auto params = parseFuncParams();
        if (!params.has_value()) {
            SLANGC_LOG(filename, "Failed to parse function parameters.", token->location, LogLevel::Error, false);
            hasError = true;
            return std::nullopt;
        }

        advance();
        std::optional<ExprPtrVariant> returnType;
        if (token->type == TokenType::Colon) {
            advance();
            returnType = parseType();
            isFunction = true;
            if (!returnType.has_value()) {
                SLANGC_LOG(filename, "Failed to parse function return typeExpr.", token->location, LogLevel::Error, false);
                hasError = true;
                return std::nullopt;
            }
        }
        else {
            returnType = createExpr<TypeExprNode>(loc, getBuiltInTypeName(BuiltInType::Void));
        }

        DeclPtrVariant funcDecl;
        auto mangledName = name;
        if (!isExtern) {
            mangledName = moduleAST->name + "." + name;
        }

        auto funcExpr = create<FuncExprNode>(loc, returnType.value(), params.value(), isFunction);
        funcDecl = createDecl<FuncDecStatementNode>(loc, mangledName, funcExpr, std::nullopt, isPrivate, isFunction, isExtern);
        context.symbolTable.insert(mangledName, moduleAST->name, funcDecl, isPrivate, false);

        expect(TokenType::LBrace);
        auto block = parseBlockStmt(name, &params.value());
        if (!block.has_value()) {
            SLANGC_LOG(filename, "Failed to parse function block.", token->location, LogLevel::Error, false);
            hasError = true;
            return std::nullopt;
        }
        //if (!isExtern) {
            auto func = std::get<FuncDecStatementPtr>(funcDecl);
            func->block = std::move(block.value());
        //}
        consume(TokenType::Semicolon);

        return funcDecl;
    }

    auto Parser::parseFieldDecl(const std::string& typeName, uint32_t fieldId) -> std::optional<DeclPtrVariant> {
        --token;
        std::optional<DeclPtrVariant> result = std::nullopt;
        SourceLoc loc = token->location;
        bool isPrivate = consume(TokenType::VisibilityType).value == "private";
        consume(TokenType::Variable);

        auto type = parseType();
        if (!type.has_value()) {
            SLANGC_LOG(filename, "Failed to parse field typeExpr.", token->location, LogLevel::Error, false);
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
            SLANGC_LOG(filename, std::string("\"this\" should have \"" + typeName + "\" typeExpr, not " + tok), token->location, LogLevel::Error, false);
            hasError = true;
            return std::nullopt;
        }
        auto thisName = consume(TokenType::Identifier).value;
        consume(TokenType::RParen);
        auto args = parseFuncParams();
        if (!args.has_value()) {
            SLANGC_LOG(filename, "Failed to parse method arguments.", token->location, LogLevel::Error, false);
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
                SLANGC_LOG(filename, "Failed to parse method return typeExpr.", token->location, LogLevel::Error, false);
                hasError = true;
                return std::nullopt;
            }
        }
        else {
            returnType = createExpr<TypeExprNode>(loc, getBuiltInTypeName(BuiltInType::Void));
        }
        loc = token->location;
        std::vector<StmtPtrVariant> statements;
        auto block = create<BlockStmtNode>(loc, statements);
        auto funcExpr = create<FuncExprNode>(loc, returnType.value(), args.value(), isFunction);
        auto funcDecl = create<MethodDecNode>(loc, name, funcExpr, thisName, block, isPrivate, isFunction, isVirtual, vtableIndex);
        //context.insert(name, funcDecl);

        expect(TokenType::LBrace);
        auto parsedBlock = parseBlockStmt(basicName);
        if (parsedBlock.has_value()) {
            block = std::move(parsedBlock.value());
        }
        else {
            SLANGC_LOG(filename, "Failed to parse method body.", token->location, LogLevel::Error, false);
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
        std::string parentTypeName = "Object";
        if (token->type == TokenType::Colon) {
            advance();
            parentTypeName = parseTypeName().value();
            advance();
        }
        auto classNode = createDecl<TypeDecStatementNode>(loc, mangledName, std::vector<DeclPtrVariant>{}, std::vector<MethodDecPtr>{}, parentTypeName, isPrivate, isExtern);
        //context.insert(name, classNode);
        //context.types.emplace_back(std::get<TypeDecStmtPtr>(classNode));
        uint32_t fieldIndex = 0;
        std::vector<DeclPtrVariant> fields;
        std::vector<MethodDecPtr> methods;

        consume(TokenType::LBrace);
        // if current type is inherited from another type, add invisible parent field at 0 index
        if (parentTypeName != "Object") {
            auto parentField = createDecl<FieldVarDecNode>(loc, "", mangledName, true, fieldIndex, parentTypeName, std::nullopt);
            ++fieldIndex;
            fields.emplace_back(parentField);
        }
        bool vtableRequired = false;
        size_t vtableIndex = 0;
        while (token->type != TokenType::RBrace && token->type != TokenType::EndOfFile) {
            consume(TokenType::VisibilityType);
            if (token->type == TokenType::Variable) {
                auto field = parseFieldDecl(mangledName, fieldIndex);
                ++fieldIndex;
                if (!field.has_value()) {
                    SLANGC_LOG(filename, "Failed to parse field declaration.", token->location, LogLevel::Error, false);
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
                    SLANGC_LOG(filename, "Failed to parse method declaration.", token->location, LogLevel::Error, false);
                    hasError = true;
                    return std::nullopt;
                }
                methods.emplace_back(std::get<MethodDecPtr>(method.value()));
            }
        }
        advance();
        consume(TokenType::Semicolon);
        //context.exitScope();

        std::get<TypeDecStmtPtr>(classNode)->fields = std::move(fields);
        std::get<TypeDecStmtPtr>(classNode)->methods = std::move(methods);

        std::get<TypeDecStmtPtr>(classNode)->vtableRequired = vtableRequired;
        //context.types[mangledName] = std::get<TypeDecStmtPtr>(classNode);
        context.symbolTable.insert(mangledName, moduleAST->name, classNode, isPrivate, false);
        return classNode;
    }
}