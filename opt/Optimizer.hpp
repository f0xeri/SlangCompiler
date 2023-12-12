//
// Created by f0xeri on 13.12.2023.
//

#ifndef SLANGCREFACTORED_OPTIMIZER_HPP
#define SLANGCREFACTORED_OPTIMIZER_HPP

#include <llvm/IR/Module.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/Analysis/CGSCCPassManager.h>
#include "CompilerOptions.hpp"

namespace Slangc {
    class Optimizer {
        llvm::PassBuilder passBuilder;
        llvm::LoopAnalysisManager loopAnalysisManager;
        llvm::FunctionAnalysisManager functionAnalysisManager;
        llvm::CGSCCAnalysisManager cGSCCAnalysisManager;
        llvm::ModuleAnalysisManager moduleAnalysisManager;
        llvm::ModulePassManager modulePassManager;

    public:
        explicit Optimizer(OptLevel optLevel);
        void run(llvm::Module& module);
        static llvm::OptimizationLevel toLLVMOptLevel(OptLevel optLevel);
    };

} // Slangc

#endif //SLANGCREFACTORED_OPTIMIZER_HPP
