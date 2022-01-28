#include "Lexer.hpp"
#include "Parser.hpp"

int main() {
    Lexer lexer("simpleclasses.sl");
    lexer.tokenize();
    Parser parser(lexer.tokens);
    parser.parse();
    CodeGenContext codeGenContext;
    codeGenContext.generateCode(parser.mainModuleNode, parser.currentScope->symbols);
    codeGenContext.mModule->print(errs(), nullptr);
    /*std::string Str;
    raw_string_ostream OS(Str);
    OS << *codeGenContext.mModule;
    OS.flush();*/
    return 0;
}
