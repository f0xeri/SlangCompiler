#include "Lexer.hpp"
#include "Parser.hpp"

int main() {
    Lexer lexer("simplemath.sl");
    lexer.tokenize();
    Parser parser(lexer.tokens);
    parser.parse();
    return 0;
}
