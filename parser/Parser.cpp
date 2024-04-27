//
// Created by f0xeri on 04.05.2023.
//

#include "Parser.hpp"
#include "check/Check.hpp"
#include "codegen/CodeGen.hpp"

namespace Slangc {
    std::map<std::filesystem::path, std::shared_ptr<Context>> globalImports;
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
            consume(TokenType::Semicolon);

            if (!globalImports.contains(importStr)) {
                globalImports.insert({importStr, nullptr});
                globalImports[importStr] = std::move(driver.processUnit(importStr, false));
            }
            context.imports.emplace_back(importStr.stem().string());
            // copy declarations from imported module to current module
            for (const auto& symbol: globalImports[importStr]->symbolTable.symbols) {
                if (symbol.moduleName == importStr.stem().string() && !symbol.isPrivate) {
                    context.symbolTable.insert(symbol.name, symbol.moduleName, symbol.declaration, symbol.isPrivate, true);
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
        if (!expect({TokenType::Identifier, TokenType::Function})) {
            return std::nullopt;
        }
        auto type = token->value;
        advance();
        std::optional<ExprPtrVariant> value;
        if (type == "function" || type == "procedure") {
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
                returnType = createExpr<TypeExprNode>(loc, getBuiltInTypeName(BuiltInType::Void));
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

            std::vector<ExprPtrVariant> sizes;
            while (token->type == TokenType::LBracket) {
                advance();
                ExprPtrVariant size;
                if (token->type != TokenType::RBracket) {
                    auto sizeExpr = parseExpr();
                    if (sizeExpr.has_value()) {
                        size = sizeExpr.value();
                    }
                }
                consume(TokenType::RBracket);
                sizes.push_back(std::move(size));
            }
            if (!sizes.empty()) {
                ExprPtrVariant arrExpr;
                for (auto it = sizes.rbegin(); it != sizes.rend(); ++it) {
                    arrExpr = create<ArrayExprNode>(loc, std::nullopt, result, *it);
                    result = arrExpr;
                }
            }
        }
        return result;
    }
} // Slangc