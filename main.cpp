#include <iostream>
#include "Lexer.hpp"
#include "Parser.hpp"

int main() {
    Lexer lexer("classes.sl");
    lexer.tokenize();
    Parser parser(lexer.tokens);
    parser.parse();
    return 0;
}
