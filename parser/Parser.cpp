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
} // Slangc