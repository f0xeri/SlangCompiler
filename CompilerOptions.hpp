//
// Created by f0xeri on 21.04.2023.
//

#ifndef SLANGC_COMPILEROPTIONS_HPP
#define SLANGC_COMPILEROPTIONS_HPP

#include <string>
#include <filesystem>
#include "llvm/Support/CommandLine.h"

using namespace llvm;

enum OptLevel {
    O0, O1, O2, O3
};

class CompilerOptions {
public:
    CompilerOptions(int argc, char **argv) {
        llvm::cl::OptionCategory options("Compiler options");

        llvm::cl::opt<std::string> OutputFileName(
                "o", llvm::cl::desc("Output file name"), llvm::cl::value_desc("filename"),
                llvm::cl::cat(options));

        cl::opt<OptLevel> OptimizationLevel(cl::desc("Optimization level:"),
                                            cl::values(
                                                    clEnumVal(O0, "No optimizations"),
                                                    clEnumVal(O1, "Enable trivial optimizations"),
                                                    clEnumVal(O2, "Enable default optimizations"),
                                                    clEnumVal(O3, "Enable expensive optimizations")), cl::init(O0),
                                            llvm::cl::value_desc("level"), cl::cat(options));

        llvm::cl::opt<bool> DebugInfo(
                "g", llvm::cl::desc("Include debug information"), llvm::cl::cat(options));

        llvm::cl::list<std::string> Libraries(
                "l", llvm::cl::desc("Libraries to link"), llvm::cl::value_desc("library"),
                llvm::cl::Prefix, llvm::cl::cat(options));

        llvm::cl::list<std::string> InputFiles(
                llvm::cl::Positional, llvm::cl::desc("<input files>"), llvm::cl::OneOrMore,
                llvm::cl::cat(options));

        llvm::cl::opt<std::string> Define(
                "D", llvm::cl::desc("Define macro"), llvm::cl::value_desc("macro"),
                llvm::cl::Prefix, llvm::cl::cat(options));

        cl::HideUnrelatedOptions(options);
        cl::SetVersionPrinter([](raw_ostream &OS) {
            OS << "Slangc LLVM compiler version 1.0\n";
            OS << "Target:" LLVM_DEFAULT_TARGET_TRIPLE << "\n";
            OS << "InstalledDir: " << std::filesystem::current_path().generic_string() << "\n";
        });
        llvm::cl::ParseCommandLineOptions(argc, argv, "Slangc LLVM compiler\n");

        outputFileName_ = OutputFileName;
        optimizationLevel_ = OptimizationLevel;
        debug_ = DebugInfo;
        libraries_ = Libraries;
        inputFiles_ = InputFiles;
    };
    [[nodiscard]] const std::vector<std::string>& getInputFileNames() const { return inputFiles_; }
    [[nodiscard]] const std::string& getOutputFileName() const { return outputFileName_; }
    [[nodiscard]] bool isDebug() const { return debug_; }
    [[nodiscard]] const std::vector<std::string>& getLinkLibraries() const { return libraries_; }
    [[nodiscard]] OptLevel getOptimizationLevel() const { return optimizationLevel_; }
private:
    std::string outputFileName_;
    bool debug_;
    std::vector<std::string> libraries_;
    std::vector<std::string> inputFiles_;
    OptLevel optimizationLevel_;
};

#endif //SLANGC_COMPILEROPTIONS_HPP
