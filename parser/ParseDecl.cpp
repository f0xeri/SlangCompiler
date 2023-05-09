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
        expect(TokenType::Start);
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

    auto Parser::parseVarDecl(bool isGlobal) -> std::optional<DeclPtrVariant> {
        return {};
    }
} // Slangc