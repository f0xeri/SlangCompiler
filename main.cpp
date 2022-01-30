#include "Lexer.hpp"
#include "Parser.hpp"

int main() {
    std::string filename = "simpleclasses";
    Lexer lexer(filename + ".sl");
    lexer.tokenize();
    Parser parser(lexer.tokens);
    parser.parse();
    CodeGenContext codeGenContext;
    codeGenContext.generateCode(parser.mainModuleNode, parser.currentScope->symbols);
    codeGenContext.mModule->print(errs(), nullptr);
    std::error_code EC;
    auto outFileStram = llvm::raw_fd_ostream(filename + ".ll", EC, sys::fs::OF_None);
    codeGenContext.mModule->print(outFileStram, nullptr);
    /*std::string Str;
    raw_string_ostream OS(Str);
    OS << *codeGenContext.mModule;
    OS.flush();*/
    return 0;
}
