#include "Lexer.hpp"
#include "Parser.hpp"
#include "Optimizer.hpp"
#include <filesystem>
#include <iostream>

void printModuleToFile(llvm::Module *module, std::string filename) {
    std::error_code EC;
    auto outFileStream = llvm::raw_fd_ostream(filename, EC, sys::fs::OF_None);
    module->print(outFileStream, nullptr);
}

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
        printModuleToFile(codeGenContext.mModule, mainFilename + ".ll");
        Optimizer optimizer(codeGenContext.mModule, codeGenContext.targetMachine);
        optimizer.optimize();
        printModuleToFile(codeGenContext.mModule, mainFilename + "_optimized" + ".ll");

        std::string filenamesString;

        llvm::errs() << "[INFO] Linking...\n";
        for (const auto &name : filenames) filenamesString += name + ".ll ";
        std::string mingw64path = std::filesystem::current_path().string() + "\\mingw64\\";
        std::string clangCall = mingw64path + "\\bin\\clang " + filenamesString + " -static-libgcc -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive -Wl,-Bdynamic -lgc-lib -lglew -lglfw3 -lopengl32 -lkernel32 -luser32 -lgdi32 -lws2_32 -lstb_image -DGLEW_STATIC -mno-stack-arg-probe -O0 -o " + mainFilename + ".exe";
        system(clangCall.c_str());

        filenames[0] = mainFilename + "_optimized";
        filenamesString = "";
        for (const auto &name : filenames) filenamesString += name + ".ll ";
        clangCall = mingw64path + "\\bin\\clang " + filenamesString + " -static-libgcc -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive -Wl,-Bdynamic -lgc-lib -lglew -lglfw3 -lopengl32 -lkernel32 -luser32 -lgdi32 -lws2_32 -lstb_image -DGLEW_STATIC -mno-stack-arg-probe -O0 -o " + mainFilename + "_optimized.exe";
        system(clangCall.c_str());
        //llvm::errs() << "[INFO] Done.\n";
    }
    return 0;
}
// llc filetype=obj elseiftests.ll -o elseiftests.obj
// ld -m i386pep -Bdynamic -o elseiftests.exe "C:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/x86_64-w64-mingw32/lib/crt2.o" "C:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib/gcc/x86_64-w64-mingw32/11.2.0/crtbegin.o" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib/gcc/x86_64-w64-mingw32/11.2.0" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/x86_64-w64-mingw32/lib" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/x86_64-w64-mingw32/sys-root/mingw/lib" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib/clang/13.0.0/lib/windows" "elseiftests.obj" -Bstatic --whole-archive -lwinpthread --no-whole-archive -Bdynamic -lgc-lib -lkernel32 -luser32 -lgdi32 -lws2_32 -lstb_image -lmingw32 -lgcc -lgcc_eh -lmoldname -lmingwex -lmsvcrt -ladvapi32 -lshell32 -luser32 -lkernel32 -lmingw32 -lgcc -lgcc_eh -lmoldname -lmingwex -lmsvcrt -lkernel32 "C:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib/gcc/x86_64-w64-mingw32/11.2.0/crtend.o"