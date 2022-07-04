#include "Lexer.hpp"
#include "Parser.hpp"
#include <filesystem>
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
        if (parser.hasError)
        {
            std::cout << "[ERROR] Compilation failed.\n";
            exit(-1);
        }
        CodeGenContext codeGenContext(parser.mainModuleNode, true, parser.currentScope->symbols);
        codeGenContext.generateCode(parser.mainModuleNode);
        if (DEBUG) codeGenContext.mModule->print(llvm::dbgs(), nullptr);
        std::error_code EC;
        auto outFileStream = llvm::raw_fd_ostream(mainFilename + ".ll", EC, sys::fs::OF_None);
        codeGenContext.mModule->print(outFileStream, nullptr);
        std::string filenamesString;
        for (const auto &name : filenames) filenamesString += name + ".ll ";
        std::string mingw64path = std::filesystem::current_path().string() + "\\mingw64\\";
        std::string clangCall = mingw64path + "\\bin\\clang " + filenamesString + "-static-libgcc -lgc-lib -lglew -lglfw3 -lopengl32 -lkernel32 -luser32 -lgdi32 -lws2_32 -pthread -lstb_image -DGLEW_STATIC -o " + mainFilename + ".exe";
        system(clangCall.c_str());
    }
    /*std::string Str;
    raw_string_ostream OS(Str);
    OS << *codeGenContext.mModule;
    OS.flush();*/
    return 0;
}
