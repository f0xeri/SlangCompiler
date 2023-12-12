//
// Created by f0xeri on 13.12.2023.
//

#include "Optimizer.hpp"
#include "CompilerOptions.hpp"

namespace Slangc {
    Optimizer::Optimizer(OptLevel optLevel) {
        passBuilder.registerModuleAnalyses(moduleAnalysisManager);
        passBuilder.registerCGSCCAnalyses(cGSCCAnalysisManager);
        passBuilder.registerFunctionAnalyses(functionAnalysisManager);
        passBuilder.registerLoopAnalyses(loopAnalysisManager);
        passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, cGSCCAnalysisManager,
                                         moduleAnalysisManager);
        modulePassManager = passBuilder.buildPerModuleDefaultPipeline(toLLVMOptLevel(optLevel));
    }

    void Optimizer::run(llvm::Module& module) {
        modulePassManager.run(module, moduleAnalysisManager);
    }

    llvm::OptimizationLevel Optimizer::toLLVMOptLevel(OptLevel optLevel) {
        switch (optLevel) {
            case OptLevel::O0:
                return llvm::OptimizationLevel::O0;
            case OptLevel::O1:
                return llvm::OptimizationLevel::O1;
            case OptLevel::O2:
                return llvm::OptimizationLevel::O2;
            case OptLevel::O3:
                return llvm::OptimizationLevel::O3;
            default:
                return llvm::OptimizationLevel::O0;
        }
    }
} // Slangc