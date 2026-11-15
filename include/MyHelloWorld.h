//=============================================================================
// FILE:
//    MyHelloWorld.cpp
//
// DESCRIPTION:
//    Visits all functions in a module, prints the signature.
//
// USAGE:
//    opt -load-pass-plugin=libHelloWorld.dylib -passes="hello-world" `\`
//      -disable-output <input-llvm-file>
//
//
// License: MIT
//=============================================================================

#ifndef ME_LLVM_TUTOR_MYHELLOWORLD_H
#define ME_LLVM_TUTOR_MYHELLOWORLD_H

#include <llvm/IR/Analysis.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Plugins/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>

class MyHelloWorld : public llvm::PassInfoMixin<MyHelloWorld> {
public:
  // The new pass manager uses Concepts for a pass (rather than inheritance)
  // namely, it must expose the run function, taking in the IR level and a
  // manager. This manager maintains cached state (which analyse are preserved).
  // In particular, we say that printing this pass (printing the type) preserves
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);

  // We say that even with the optnone LLVM attribute, this should be run.
  static bool isRequired() { return true; }
};

#endif
