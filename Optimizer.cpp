//
// Created by f0xeri on 14.04.2023.
//

#include "Optimizer.hpp"

void Optimizer::addOptimizationPasses(llvm::PassManagerBase &passes, llvm::legacy::FunctionPassManager &funPasses) {
    llvm::PassManagerBuilder builder;
    builder.OptLevel = optLevel;
    builder.SizeLevel = optLevel;
    builder.Inliner = llvm::createFunctionInliningPass(optLevel, optLevel, false);
    builder.LoopVectorize = true;
    builder.SLPVectorize = true;
    machine->adjustPassManager(builder);

    builder.populateFunctionPassManager(funPasses);
    builder.populateModulePassManager(passes);
}

void Optimizer::addLinkPasses(llvm::PassManagerBase &passes) {
    llvm::PassManagerBuilder builder;
    builder.VerifyInput = true;
    builder.Inliner = llvm::createFunctionInliningPass(optLevel, optLevel, false);
    builder.populateLTOPassManager(passes);
}

void Optimizer::optimize() {
    mModule->setTargetTriple(machine->getTargetTriple().str());
    mModule->setDataLayout(machine->createDataLayout());
    llvm::legacy::PassManager passes;
    passes.add(new llvm::TargetLibraryInfoWrapperPass(machine->getTargetTriple()));
    passes.add(llvm::createTargetTransformInfoWrapperPass(machine->getTargetIRAnalysis()));

    llvm::legacy::FunctionPassManager fnPasses(mModule);
    fnPasses.add(llvm::createTargetTransformInfoWrapperPass(machine->getTargetIRAnalysis()));

    addOptimizationPasses(passes, fnPasses);
    addLinkPasses(passes);

    fnPasses.doInitialization();
    for (llvm::Function &func : *mModule) {
        fnPasses.run(func);
    }
    fnPasses.doFinalization();

    passes.add(llvm::createVerifierPass());
    passes.run(*mModule);
}
