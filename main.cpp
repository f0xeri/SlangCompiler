#include "Lexer.hpp"
#include "Parser.hpp"
#include <filesystem>
#include <numeric>
#include <iostream>

int main(int argc, char **argv) {
    std::string mainFilename;
    std::vector<std::string> filenames;
    if (argc == 1)
    {
        llvm::errs() << "[ERROR] No input filenames. Compilation terminated.\n";
        exit(1);
    }
    else
    {
        mainFilename = argv[1];
        mainFilename = mainFilename.substr(0, mainFilename.find('.', 0));
        filenames.push_back(mainFilename);

        // temporary work around
        if (mainFilename == "--help")
        {
            std::cout << "USAGE: slangc.exe [options] file...\n";
            std::cout << "OPTIONS:\n   -gc\t\t Enable Boehm garbage collector\n   -debug\t Enable debug output\n";
            return 0;
        }

        for (int i = 2; i < argc; i++)
        {
            std::string name = argv[i];
            filenames.emplace_back(name.substr(0, name.find('.', 0)));
        }
        Lexer lexer(mainFilename + ".sl");
        lexer.tokenize();
        Parser parser(lexer.tokens);
        parser.parse();
        CodeGenContext codeGenContext(parser.mainModuleNode, true);
        codeGenContext.generateCode(parser.mainModuleNode, parser.currentScope->symbols);
        if (DEBUG) codeGenContext.mModule->print(llvm::dbgs(), nullptr);
        std::error_code EC;
        auto outFileStream = llvm::raw_fd_ostream(mainFilename + ".ll", EC, sys::fs::OF_None);
        codeGenContext.mModule->print(outFileStream, nullptr);
        std::string filenamesString;
        for (const auto &name : filenames) filenamesString += name + ".ll ";
        std::string mingw64path = std::filesystem::current_path().string() + "\\mingw64\\";
        std::string clangCall = mingw64path + "\\bin\\clang " + filenamesString + "-lgc-lib -o " + mainFilename + ".exe";
        system(clangCall.c_str());
    }
    /*std::string Str;
    raw_string_ostream OS(Str);
    OS << *codeGenContext.mModule;
    OS.flush();*/
    return 0;
}
