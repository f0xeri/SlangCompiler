#include "Lexer.hpp"
#include "Parser.hpp"
#include "Optimizer.hpp"
#include "CompilerOptions.hpp"
#include <filesystem>
#include <iostream>

void printModuleToFile(llvm::Module *module, const std::string& filename) {
    std::error_code EC;
    auto outFileStream = llvm::raw_fd_ostream(filename, EC, sys::fs::OF_None);
    module->print(outFileStream, nullptr);
}

#ifdef _WIN32
#include <windows.h>
std::filesystem::path getCurrentProcessDirectory() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, sizeof(buffer));
    return std::filesystem::path(buffer).parent_path();
}
#elif __linux__
std::filesystem::path getCurrentProcessDirectory() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::filesystem::path(std::string(result, (count > 0) ? count : 0)).parent_path();
}
#endif

std::string join(const std::vector<std::string>& vec, const std::string& delimiter, const std::string& prefix = "") {
    if (vec.empty()) return "";
    std::string result;
    result.reserve(vec.size() * (prefix.size() + delimiter.size()) + prefix.size() + vec.back().size());
    for (auto& elem : vec) {
        result += prefix;
        result += elem;
        if (elem != vec.back()) {
            result += delimiter;
        }
    }
    return result;
}

int main(int argc, char **argv) {
    CompilerOptions options(argc, argv);
    auto filenames = options.getInputFileNames();
    std::string mainFilename = filenames[0];
    Lexer lexer(mainFilename);
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
    std::string sourceFilename = mainFilename.substr(0, mainFilename.find_last_of('.'));
    printModuleToFile(codeGenContext.mModule, sourceFilename + ".ll");
    Optimizer optimizer(codeGenContext.mModule, codeGenContext.targetMachine, options.getOptimizationLevel());
    optimizer.optimize();
    printModuleToFile(codeGenContext.mModule, mainFilename + "_optimized" + ".ll");

    std::string filenamesString;
    llvm::outs() << "[INFO] Linking...\n";
    for (const auto &name : filenames) {
        std::string filename = name.substr(0, name.find_last_of('.'));
        filenamesString += filename + ".ll ";
    }
    auto libs = join(options.getLinkLibraries(), " ", "-l");

#ifdef _WIN32
    std::string mingw64path = getCurrentProcessDirectory().string() + "\\mingw64\\";
    std::string clangCall = mingw64path + "\\bin\\clang " + filenamesString + " -static-libgcc -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive -Wl,-Bdynamic " + libs + " -o " + options.getOutputFileName();
    system(clangCall.c_str());
#elif __linux__
    std::string clangCall = "clang " + filenamesString + "-static-libgcc " + libs + " -o " + options.getOutputFileName();
    system(clangCall.c_str());
#endif
    return 0;
}
// llc filetype=obj elseiftests.ll -o elseiftests.obj
// ld -m i386pep -Bdynamic -o elseiftests.exe "C:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/x86_64-w64-mingw32/lib/crt2.o" "C:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib/gcc/x86_64-w64-mingw32/11.2.0/crtbegin.o" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib/gcc/x86_64-w64-mingw32/11.2.0" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/x86_64-w64-mingw32/lib" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/x86_64-w64-mingw32/sys-root/mingw/lib" "-LC:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib/clang/13.0.0/lib/windows" "elseiftests.obj" -Bstatic --whole-archive -lwinpthread --no-whole-archive -Bdynamic -lgc-lib -lkernel32 -luser32 -lgdi32 -lws2_32 -lstb_image -lmingw32 -lgcc -lgcc_eh -lmoldname -lmingwex -lmsvcrt -ladvapi32 -lshell32 -luser32 -lkernel32 -lmingw32 -lgcc -lgcc_eh -lmoldname -lmingwex -lmsvcrt -lkernel32 "C:/Users/9piko/Desktop/slangc/SlangCompiler/cmake-build-debug/mingw64/lib/gcc/x86_64-w64-mingw32/11.2.0/crtend.o"