#include "Lexer.hpp"
#include "Parser.hpp"
#include <filesystem>

int main(int argc, char **argv) {
    std::string filename;

    if (argc == 1)
    {
        llvm::errs() << "[ERROR] No input files. Compilation terminated.\n";
        exit(1);
    }
    else
    {
        filename = argv[1];
        filename = filename.substr(0, filename.find('.', 0));
        Lexer lexer(filename + ".sl");
        lexer.tokenize();
        Parser parser(lexer.tokens);
        parser.parse();
        CodeGenContext codeGenContext;
        codeGenContext.generateCode(parser.mainModuleNode, parser.currentScope->symbols);
        if (DEBUG) codeGenContext.mModule->print(llvm::dbgs(), nullptr);
        std::error_code EC;
        auto outFileStream = llvm::raw_fd_ostream(filename + ".ll", EC, sys::fs::OF_None);
        codeGenContext.mModule->print(outFileStream, nullptr);
        std::string clangCall = std::filesystem::current_path().string() + "\\mingw64\\bin\\clang " + filename + ".ll -o " + filename + ".exe";
        system(clangCall.c_str());
    }
    /*std::string Str;
    raw_string_ostream OS(Str);
    OS << *codeGenContext.mModule;
    OS.flush();*/
    return 0;
}
