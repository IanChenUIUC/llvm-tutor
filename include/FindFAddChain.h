//==============================================================================
// FILE:
//    FindFAddChain.h
//
// DESCRIPTION:
//    Declares the FindFAddChain pass for the new legacy pass managers as
//    well as a printing pass for the new pass manager.
//    This pass finds all chains of FP adds (left associative) of size >1.
//
// License: MIT
//==============================================================================

#ifndef LLVM_TUTOR_FIND_FADD_CHAIN_H
#define LLVM_TUTOR_FIND_FADD_CHAIN_H

#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include <llvm/IR/InstrTypes.h>
#include <vector>

//------------------------------------------------------------------------------
// New PM interface
//------------------------------------------------------------------------------
class FindFAddChain : public llvm::AnalysisInfoMixin<FindFAddChain> {
public:
  using Result = std::vector<std::vector<llvm::BinaryOperator *>>;
  // This is one of the standard run() member functions expected by
  // PassInfoMixin. When the pass is executed by the new PM, this is the
  // function that will be called.
  Result run(llvm::Function &Func, llvm::FunctionAnalysisManager &FAM);
  // This is a helper run() member function overload which can be called by the
  // legacy pass (or any other code) without having to supply a
  // FunctionAnalysisManager argument.
  Result run(llvm::Function &Func);

private:
  friend struct llvm::AnalysisInfoMixin<FindFAddChain>;
  static llvm::AnalysisKey Key;
};

//------------------------------------------------------------------------------
// New PM interface for the printer pass
//------------------------------------------------------------------------------
class FindFAddChainPrinter : public llvm::PassInfoMixin<FindFAddChainPrinter> {
public:
  explicit FindFAddChainPrinter(llvm::raw_ostream &OutStream)
      : OS(OutStream) {};

  llvm::PreservedAnalyses run(llvm::Function &Func,
                              llvm::FunctionAnalysisManager &FAM);

private:
  llvm::raw_ostream &OS;
};

#endif
