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

    class Parser {
    public:
        explicit Parser(const std::map<std::uint64_t, std::vector<Token>> &tokens, std::vector<ErrorMessage> &errors, const CompilerOptions& options)
                : options(options), errors(errors), tokens(tokens) {}

        const CompilerOptions& options;
        std::vector<ErrorMessage> &errors;
        bool hasError = false;

        auto parse() -> bool;
		auto parseImports() -> bool;

    private:
        const std::map<std::uint64_t, std::vector<Token>> &tokens;
        std::shared_ptr<Scope> currentScope;
    };

} // Slangc

#endif //SLANGCREFACTORED_PARSER_HPP
