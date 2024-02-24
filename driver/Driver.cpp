//
// Created by f0xeri on 30.11.2023.
//

#include <cstdlib>
#include "Driver.hpp"
#include "source/SourceBuffer.hpp"
#include "lexer/Lexer.hpp"
#include "parser/Parser.hpp"
#include "check/Check.hpp"
#include "codegen/CodeGen.hpp"
#include "opt/Optimizer.hpp"

namespace Slangc {

    void Driver::run() {
        auto mainModuleName = options.getInputFilePaths()[0];
        processUnit(mainModuleName, true);
        for (auto &error : errors)
            log() << error;
        if (!errors.empty()) return;
        std::stringstream clangCallStream;
        clangCallStream << "clang ";
        std::ranges::copy(std::views::transform(options.getInputFilePaths(), [](const std::filesystem::path &p){ return p.string(); }), std::ostream_iterator<std::string>(clangCallStream, ".o "));
        std::ranges::copy(std::views::transform(options.getLibDirs(), [](const std::string &p){ return "-L" + p; }), std::ostream_iterator<std::string>(clangCallStream, " "));
        std::ranges::copy(std::views::transform(options.getLinkLibraries(), [](const std::string &p){ return "-l" + p; }), std::ostream_iterator<std::string>(clangCallStream, " "));
        std::ranges::copy(std::views::transform(options.getDefines(), [](const std::string &p){ return "-D" + p; }), std::ostream_iterator<std::string>(clangCallStream, " "));
        std::string outputFilename;
        if (options.getOutputFilePath().empty()) {
#ifdef _WIN32
            outputFilename = "a.exe";
#else
            outputFilename = "a.out";
#endif
        }
        clangCallStream << " -o " << (options.getOutputFilePath().empty() ? outputFilename : options.getOutputFilePath().string());
        system(clangCallStream.str().c_str());
    }

    std::unique_ptr<Context> Driver::processUnit(std::filesystem::path &filepath, bool isMainModule) {
        log() << "Building " << filepath.string() << "...\n";
        auto buffer = SourceBuffer::CreateFromFile(filepath.string());
        if (!buffer) {
            std::cout << toString(buffer.takeError()) << std::endl;
            exit(1);
        }
        auto lexer = Lexer(std::move(buffer.get()), errors);
        lexer.tokenize();
        auto context = std::make_unique<Context>();
        auto parser = Parser(filepath, lexer.tokens, *this, *context, errors);
        parser.parse();

        Check::checkAST(parser.moduleAST, *context, errors);
        auto codeGen = CodeGen(*context, std::move(parser.moduleAST), isMainModule);
        if (!containsErrors(errors))
            codeGen.process(errors);
        else
            return context;
        //codeGen.dumpIRToFile(filepath.string() + ".ll");
        Optimizer optimizer(options.getOptimizationLevel());
        optimizer.run(*codeGen.getModule());
        codeGen.dumpIRToFile(filepath.string() + ".ll");
        codeGen.generateObjectFile(filepath.string() + ".o");
        return context;
    }
} // Slangc