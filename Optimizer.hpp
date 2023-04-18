//
// Created by f0xeri on 14.04.2023.
//

#ifndef SLANGC_OPTIMIZER_HPP
#define SLANGC_OPTIMIZER_HPP

#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

class Optimizer {
    llvm::Module *mModule;
    llvm::TargetMachine *machine;
    void addOptimizationPasses(llvm::PassManagerBase &passes,  llvm::legacy::FunctionPassManager &funPasses);
    void addLinkPasses(llvm::legacy::PassManagerBase &passes);

public:
    explicit Optimizer(llvm::Module *mModule, llvm::TargetMachine *machine) : mModule(mModule), machine(machine) {};
    void optimize();
};


#endif //SLANGC_OPTIMIZER_HPP
