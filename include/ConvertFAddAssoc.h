//==============================================================================
// FILE:
//    ConvertFAddAssoc.h
//
// DESCRIPTION:
//    Declares the ConvertFAddAssoc pass.
//
// License: MIT
//==============================================================================

#ifndef LLVM_TUTOR_CONVERT_FADD_ASSOC_H
#define LLVM_TUTOR_CONVERT_FADD_ASSOC_H

#include "FindFAddChain.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

struct ConvertFAddAssoc : llvm::PassInfoMixin<ConvertFAddAssoc> {
  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &FAM);
};

#endif // !LLVM_TUTOR_CONVERT_FCMP_EQ_H
