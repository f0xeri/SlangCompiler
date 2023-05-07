//
// Created by f0xeri on 04.05.2023.
//

#include "Parser.hpp"

namespace Slangc {

    auto Parser::parse() -> bool {
        parseImports();

        auto loc = SourceLoc{0, 0};
        auto plus = createExpr<OperatorExprNode>(loc,
                                                 TokenType::Plus,
                                                 createExpr<IntExprNode>(loc, 5),
                                                 createExpr<IntExprNode>(loc, 10));
        moduleAST = create<ModuleStatementNode>(loc, "test", create<BlockExprNode>(loc, std::vector<StmtPtrVariant>()));
        moduleAST->block->statements.push_back(createStmt<ReturnStatementNode>(loc, createExpr<IntExprNode>(loc, 0)));

        while (token->type != TokenType::EndOfFile) {
            auto decl = parseVarDecl(false);
            if (decl.has_value()) {
                //moduleAST->block->statements.emplace_back(std::move(decl.value()));
            }
            else {
                return false;
            }
        }
        return true;
    }

    auto Parser::parseImports() -> bool {
        return true;
    }
} // Slangc