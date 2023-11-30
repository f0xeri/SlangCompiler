//
// Created by f0xeri on 04.05.2023.
//

#include "Parser.hpp"
#include "check/Check.hpp"
#include "codegen/CodeGen.hpp"

namespace Slangc {
    std::vector<std::string> imports;
    auto Parser::parse() -> bool {
        parseImports();

        auto loc = SourceLoc{0, 0};

        auto obj = create<TypeDecStatementNode>(loc, std::string("Object"), std::vector<DeclPtrVariant>(), std::vector<MethodDecPtr>());
        context.filename = filename;
        auto moduleNode = parseModuleDecl();
        if (moduleNode.has_value()) {
            //std::cout << moduleNode.assignExpr()->block->statements.size() << std::endl;
            moduleAST = std::move(moduleNode.value());
        }
        else {
            errors.emplace_back(filename, "Failed to parse module declaration.", token->location, false, false);
            return false;
        }
        context.symbolTable.insert("Object", "", obj, false, false);
        return true;
    }

    auto Parser::parseImports() -> bool {
        while (token->type == TokenType::Import) {
            advance();
            expect({TokenType::Identifier, TokenType::String});
            std::filesystem::path importStr = token->value;
            auto currModuleDir = filepath.parent_path();
            if (token->type == TokenType::Identifier)
                importStr = currModuleDir / (token->value + ".sl");
            else
                importStr = currModuleDir / importStr;
            advance();
            expect(TokenType::Semicolon);
            advance();
            if (std::find(imports.begin(), imports.end(), importStr) == imports.end()) {
                imports.emplace_back(importStr.stem().string());
                auto importContext = driver.processUnit(importStr, false);
                // copy declarations from imported module to current module
                for (const auto& symbol: importContext->symbolTable.symbols) {
                    if (symbol.moduleName == importStr.stem().string() && !symbol.isPrivate) {
                        context.symbolTable.insert(symbol.name, symbol.moduleName, symbol.declaration, symbol.isPrivate, true);
                    }
                }
            }
        }
        return true;
    }

    auto Parser::parseTypeName() -> std::optional<std::string> {
        if (token->type != TokenType::Identifier) {
            errors.emplace_back(filename, "Expected typeExpr name.", token->location, false, false);
            hasError = true;
            return std::nullopt;
        }
        auto typeName = token->value;
        advance();
        if (token->type == TokenType::Dot) {
            advance();
            expect(TokenType::Identifier);
            typeName += "." + token->value;
        } else {
            --token;
        }
        return typeName;
    }

    auto Parser::parseType() -> std::optional<ExprPtrVariant> {
        ExprPtrVariant result;
        SourceLoc loc = token->location;
        if (!expect({TokenType::Identifier, TokenType::Function, TokenType::Procedure})) {
            return std::nullopt;
        }
        auto type = token->value;
        advance();
        std::optional<ExprPtrVariant> value;
        if (type == "array") {
            auto indicesCount = 1;
            consume(TokenType::LBracket);
            ExprPtrVariant size;
            if (token->type != TokenType::RBracket) {
                auto sizeExpr = parseExpr();
                if (sizeExpr.has_value()) {
                    size = sizeExpr.value();
                }
            }
            else {
                size = create<IntExprNode>(loc, 0);
            }
            consume(TokenType::RBracket);
            auto arrayType = parseType();
            if (!arrayType.has_value()) {
                errors.emplace_back(filename, "Failed to parse array typeExpr.", token->location, false, false);
                hasError = true;
                return std::nullopt;
            }
            auto arrExpr = create<ArrayExprNode>(loc, std::nullopt, arrayType.value(), size);
            /*while (arrayType == "array") {
                advance();
                indicesCount++;
                consume(TokenType::LBracket);
                if (token->typeExpr != TokenType::RBracket) {
                    auto sizeExpr = parseExpr();
                    if (sizeExpr.has_value()) {
                        size = sizeExpr.assignExpr();
                    }
                }
                else {
                    size = create<IntExprNode>(loc, 0);
                }
                consume(TokenType::RBracket);

                if (token->typeExpr == TokenType::Identifier && token->assignExpr != "array") {
                    arrayType = parseTypeName().assignExpr();
                }
                arrExpr->values.emplace_back(createExpr<ArrayExprNode>(loc, std::vector<ExprPtrVariant>(), arrayType, size));
            }*/
            result = arrExpr;
        } else if (type == "function" || type == "procedure") {
            bool isFunction = type == "function";
            auto args = parseFuncParams(false);
            if (!args.has_value()) {
                errors.emplace_back(filename, "Expected function parameters.", token->location, false, false);
                hasError = true;
                return std::nullopt;
            }
            advance();
            ExprPtrVariant returnType;
            if (isFunction) {
                consume(TokenType::Colon);
                auto returnTypeOpt = parseType();
                if (returnTypeOpt.has_value()) {
                    returnType = std::move(returnTypeOpt.value());
                } else {
                    errors.emplace_back(filename, "Expected typeExpr after ':'.", token->location, false, false);
                    hasError = true;
                    return std::nullopt;
                }
            } else {
                returnType = createExpr<TypeExprNode>(loc, "void");
            }
            result = createExpr<FuncExprNode>(loc, std::move(returnType), std::move(args.value()), isFunction);
        } else {
            if (token->type == TokenType::Dot) {
                advance();
                expect(TokenType::Identifier);
                type += "." + token->value;
                advance();
            }
            result = createExpr<TypeExprNode>(loc, type);
        }
        return result;
    }
} // Slangc