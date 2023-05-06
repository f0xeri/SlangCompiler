//
// Created by f0xeri on 06.05.2023.
//

#include "Parser.hpp"

namespace Slangc {
    auto Parser::parseModuleStmt() -> std::optional<ModuleStatementNode> {
        SourceLoc start = currentToken->location;
        return std::nullopt;
    }
} // Slangc