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
            currentToken = this->tokens.begin();
            currentScope = std::make_shared<Scope>();

        }

        const CompilerOptions &options;
        std::vector<ErrorMessage> &errors;
        bool hasError = false;

        auto parse() -> bool;

        auto parseImports() -> bool;

        auto parseModuleStmt() -> std::optional<ModuleStatementNode>;

    private:
        std::vector<Token> tokens;
        std::shared_ptr<Scope> currentScope;
        std::vector<Token>::iterator currentToken;
    };

} // Slangc

#endif //SLANGCREFACTORED_PARSER_HPP
