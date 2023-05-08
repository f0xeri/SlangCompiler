//
// Created by f0xeri on 04.05.2023.
//

#ifndef SLANGCREFACTORED_PARSER_HPP
#define SLANGCREFACTORED_PARSER_HPP

#include <map>
#include <algorithm>
#include <vector>
#include "lexer/TokenType.hpp"
#include "Scope.hpp"
#include "lexer/Lexer.hpp"
#include <CompilerOptions.hpp>

namespace Slangc {

    using namespace AST;
    using ParserExprResult = std::optional<ExprPtrVariant>;
    using ParserStmtResult = std::optional<StmtPtrVariant>;
    using ParserDeclResult = std::optional<DeclPtrVariant>;

    class Parser {
    public:
        explicit Parser(std::vector<Token> tokens, std::vector<ErrorMessage> &errors, const CompilerOptions &options)
                : options(options), errors(errors), tokens(std::move(tokens)) {
            token = this->tokens.cbegin();
            currentScope = std::make_shared<Scope>();

        }

        const CompilerOptions &options;
        std::vector<ErrorMessage> &errors;
        ModuleDeclPtr moduleAST;
        bool hasError = false;

        auto parse() -> bool;
        auto parseImports() -> bool;

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


        auto parseModuleDecl() -> std::optional<ModuleDeclPtr>;
        auto parseVarStmt() -> std::optional<StmtPtrVariant>;
        auto parseBlockStmt(const std::string& name) -> std::optional<BlockStmtPtr>;

        auto parseVarDecl(bool isGlobal) -> std::optional<DeclPtrVariant>;

    private:
        std::vector<Token> tokens;
        std::shared_ptr<Scope> currentScope;
        std::vector<Token>::const_iterator token;

        auto advance() -> Token {
            if (token->type == TokenType::EndOfFile) {
                errors.emplace_back("Unexpected EOF token.", token->location);
                hasError = true;
                return *token;
            }
            token++;
            return *token;
        }

        void error() {
            errors.emplace_back("Unexpected token " + std::string(Lexer::getTokenName(*token)), token->location);
            hasError = true;
        }

        bool expect(TokenType tokenType) {
            if (token->type != tokenType) {
                errors.emplace_back(std::string("Unexpected token ") + std::string(Lexer::getTokenName(*token)) + std::string(", expected ") + std::string(Lexer::getTokenName(tokenType)), token->location);
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
