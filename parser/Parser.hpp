//
// Created by f0xeri on 04.05.2023.
//

#ifndef SLANGCREFACTORED_PARSER_HPP
#define SLANGCREFACTORED_PARSER_HPP

#include <map>
#include <algorithm>
#include <utility>
#include <vector>
#include "lexer/TokenType.hpp"
#include "Scope.hpp"
#include "lexer/Lexer.hpp"
#include "AST.hpp"
#include "check/Context.hpp"
#include "CompilerOptions.hpp"
#include "driver/Driver.hpp"

namespace Slangc {
    using ParserExprResult = std::optional<ExprPtrVariant>;
    using ParserStmtResult = std::optional<StmtPtrVariant>;
    using ParserDeclResult = std::optional<DeclPtrVariant>;

    class Parser {
        Context &context;
        Driver &driver;
        std::filesystem::path filepath;
    public:
        explicit Parser(std::filesystem::path &filepath, std::vector<Token> tokens, Driver &driver, Context &context, std::vector<ErrorMessage> &errors)
                : context(context), driver(driver), errors(errors), tokens(std::move(tokens)), filepath(filepath) {
            token = this->tokens.begin();
            filename = filepath.string();
        }

        //const CompilerOptions &options;
        std::vector<ErrorMessage> &errors;
        ModuleDeclPtr moduleAST;
        std::string filename;
        bool hasError = false;

        auto parse() -> bool;
        auto parseImports() -> bool;
        auto parseTypeName() -> std::optional<std::string>;
        auto parseType() -> std::optional<ExprPtrVariant>;

        auto parseExpr() -> std::optional<ExprPtrVariant>;
        auto parseOr() -> std::optional<ExprPtrVariant>;
        auto parseAnd() -> std::optional<ExprPtrVariant>;
        auto parseEquality() -> std::optional<ExprPtrVariant>;
        auto parseCmp() -> std::optional<ExprPtrVariant>;
        auto parseAddSub() -> std::optional<ExprPtrVariant>;
        auto parseMulDiv() -> std::optional<ExprPtrVariant>;
        auto parseUnary() -> std::optional<ExprPtrVariant>;
        auto parseCall() -> std::optional<ExprPtrVariant>;
        auto parsePrimary() -> std::optional<ExprPtrVariant>;
        auto parseVar() -> std::optional<ExprPtrVariant>;

        auto parseVarExpr() -> std::optional<ExprPtrVariant>;
        auto parseAccess() -> std::optional<ExprPtrVariant>;

        auto parseModuleDecl() -> std::optional<ModuleDeclPtr>;
        auto parseBlockStmt(const std::string& name, std::vector<FuncParamDecStmtPtr> *args = nullptr) -> std::optional<BlockStmtPtr>;
        auto parseVarStmt() -> std::optional<StmtPtrVariant>;
        auto parseIfStmt() -> std::optional<StmtPtrVariant>;
        auto parseWhileStmt() -> std::optional<StmtPtrVariant>;
        auto parseOutputStmt() -> std::optional<StmtPtrVariant>;
        auto parseInputStmt() -> std::optional<StmtPtrVariant>;
        auto parseLetStmt() -> std::optional<StmtPtrVariant>;
        auto parseReturnStmt() -> std::optional<StmtPtrVariant>;
        auto parseCallStmt() -> std::optional<StmtPtrVariant>;
        auto parseDeleteStmt() -> std::optional<StmtPtrVariant>;

        auto parseFuncParams(bool named = true) -> std::optional<std::vector<FuncParamDecStmtPtr>>;
        auto parseFuncDecl() -> std::optional<DeclPtrVariant>;
        auto parseFieldDecl(const std::string& typeName, uint32_t fieldId) -> std::optional<DeclPtrVariant>;
        auto parseMethodDecl(const std::string& typeName, size_t vtableIndex) -> std::optional<DeclPtrVariant>;
        auto parseClassDecl() -> std::optional<DeclPtrVariant>;
        auto parseVarDecl() -> std::optional<DeclPtrVariant>;

    private:
        std::vector<Token> tokens;
        std::vector<Token>::const_iterator token;

        auto advance() -> Token {
            if (token->type == TokenType::EndOfFile) {
                errors.emplace_back(filename, "Unexpected EOF token.", token->location);
                hasError = true;
                return *token;
            }
            token++;
            return *token;
        }

        void error() {
            errors.emplace_back(filename, "Unexpected token " + std::string(Lexer::getTokenName(*token)) + ".", token->location);
            hasError = true;
        }

        bool expect(TokenType tokenType) {
            if (token->type != tokenType) {
                errors.emplace_back(filename, std::string("Unexpected token ") + std::string(Lexer::getTokenName(*token)) + std::string(", expected ") + std::string(Lexer::getTokenName(tokenType)) + ".", token->location);
                hasError = true;
                return false;
            }
            return true;
        }

        bool expect(std::initializer_list<TokenType> tokenTypes) {
            if (std::none_of(tokenTypes.begin(), tokenTypes.end(), [this](TokenType tokenType) { return token->type == tokenType; })) {
                std::string err = "Unexpected token ";
                err += Lexer::getTokenName(*token);
                err += ", expected one of: ";
                for (auto it = tokenTypes.begin(); it != tokenTypes.end(); ++it) {
                    err += Lexer::getTokenName(*it);
                    if (it != tokenTypes.end() - 1) {
                        err += ", ";
                    }
                }
                err += ".";
                errors.emplace_back(filename, err, token->location);
                hasError = true;
                return false;
            }
            return true;
        }

        bool match(TokenType tokenType) {
            if (token->type == tokenType) {
                advance();
                return true;
            }
            return false;
        }

        bool match(std::initializer_list<TokenType> tokenTypes) {
            if (std::any_of(tokenTypes.begin(), tokenTypes.end(), [this](TokenType tokenType) { return token->type == tokenType; })) {
                advance();
                return true;
            }
            return false;
        }

        auto prevToken() -> Token {
            return *(token - 1);
        }

        auto consume(TokenType tokenType) -> Token {
            expect(tokenType);
            Token tok = *token;
            advance();
            return tok;
        }

        static auto oneOfDefaultTypes(std::string_view name) -> bool {
            return (name == "integer" || name == "float" || name == "real" || name == "boolean" || name == "character");
        }
    };

} // Slangc

#endif //SLANGCREFACTORED_PARSER_HPP
