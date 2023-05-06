//
// Created by f0xeri on 04.05.2023.
//

#include "Parser.hpp"

namespace Slangc {

    auto Parser::parse() -> bool {
        parseImports();

        auto loc = SourceLoc{0, 0};
        auto plus = createExpr<OperatorExprNode>(loc, TokenType::Plus,
                                                 createExpr<IntExprNode>(loc, 5),
                                                 createExpr<IntExprNode>(loc, 10));

        return false;
    }

    auto Parser::parseImports() -> bool {
        return true;
    }
} // Slangc