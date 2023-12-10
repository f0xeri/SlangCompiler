//
// Created by f0xeri on 30.11.2023.
//

#include "Driver.hpp"
#include "source/SourceBuffer.hpp"
#include "lexer/Lexer.hpp"
#include "parser/Parser.hpp"
#include "check/Check.hpp"
#include "codegen/CodeGen.hpp"

namespace Slangc {

    void Driver::run() {
        auto mainModuleName = options.getInputFilePaths()[0];
        processUnit(mainModuleName, true);
        for (auto &error : errors)
            log() << error;
    }

    std::unique_ptr<Context> Driver::processUnit(std::filesystem::path &filepath, bool isMainModule) {
        log() << std::format("Building '{}'...\n", filepath.string());
        auto buffer = SourceBuffer::CreateFromFile(filepath.string());
        if (!buffer) {
            std::cout << toString(buffer.takeError()) << std::endl;
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
        codeGen.dumpIRToFile(filepath.string() + ".ll");
        return context;
    }
} // Slangc