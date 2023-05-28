//
// Created by f0xeri on 04.05.2023.
//

#include "Parser.hpp"

namespace Slangc {

    auto Parser::parse() -> bool {
        parseImports();

        auto loc = SourceLoc{0, 0};
        moduleAST = create<ModuleDeclNode>(loc, "test", create<BlockStmtNode>(loc, std::vector<StmtPtrVariant>()));
        auto moduleNode = parseModuleDecl();
        if (moduleNode.has_value()) {
            std::cout << moduleNode.value()->block->statements.size() << std::endl;
            moduleAST = std::move(moduleNode.value());
        }
        else {
            errors.emplace_back("Failed to parse module declaration.", token->location, false, false);
            return false;
        }
        return true;
    }

    auto Parser::parseImports() -> bool {
        return true;
    }

    auto Parser::parseTypeName() -> std::optional<std::string> {
        if (token->type != TokenType::Identifier) {
            errors.emplace_back("Expected type name.", token->location, false, false);
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
            auto arrayType = parseType().value();
            auto arrExpr = create<ArrayExprNode>(loc, std::nullopt, arrayType, size);
            /*while (arrayType == "array") {
                advance();
                indicesCount++;
                consume(TokenType::LBracket);
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

                if (token->type == TokenType::Identifier && token->value != "array") {
                    arrayType = parseTypeName().value();
                }
                arrExpr->values.emplace_back(createExpr<ArrayExprNode>(loc, std::vector<ExprPtrVariant>(), arrayType, size));
            }*/
            result = arrExpr;
        } else if (type == "function" || type == "procedure") {
            bool isFunction = type == "function";
            auto args = parseFuncParams(false);
            if (!args.has_value()) {
                errors.emplace_back("Expected function parameters.", token->location, false, false);
                hasError = true;
            }
            advance();
            ExprPtrVariant returnType;
            if (isFunction) {
                consume(TokenType::Colon);
                auto returnTypeOpt = parseType();
                if (returnTypeOpt.has_value()) {
                    returnType = std::move(returnTypeOpt.value());
                } else {
                    errors.emplace_back("Expected type after ':'.", token->location, false, false);
                    hasError = true;
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