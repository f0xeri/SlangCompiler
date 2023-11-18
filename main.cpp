#include <iostream>
#include "common.hpp"
#include "source/SourceBuffer.hpp"
#include "lexer/Lexer.hpp"
#include "CompilerOptions.hpp"
#include "parser/Parser.hpp"
#include "parser/AST.hpp"
#include "analysis/Check.hpp"
#include <codegen/CodeGen.hpp>

int main(int argc, char **argv) {
    Slangc::CompilerOptions options(argc, argv);
    std::vector<Slangc::ErrorMessage> errors;

    auto buffer = Slangc::SourceBuffer::CreateFromFile("sample.sl");
    if (!buffer) {
        std::cout << toString(buffer.takeError()) << std::endl;
    }
    Slangc::Lexer lexer(std::move(buffer.get()), errors);
    lexer.tokenize();
    // lexer.printTokens();

    Slangc::Context context;
    Slangc::Parser parser(buffer->getFilename(), lexer.tokens, options, context, errors);
    parser.parse();

    Slangc::Check::checkAST(parser.moduleAST, context, errors);

    Slangc::CodeGen codeGen(std::move(parser.moduleAST));
    codeGen.process();

    std::cout << "\n";
    Slangc::printErrorMessages(errors, std::cout, false);
    return 0;
}
