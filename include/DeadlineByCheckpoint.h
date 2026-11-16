//=============================================================================
// FILE:
//    DeadlineByCheckpoint.cpp
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

#ifndef ME_LLVM_TUTOR_DEADLINE_CHECKPOINT_H
#define ME_LLVM_TUTOR_DEADLINE_CHECKPOINT_H

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Analysis.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Plugins/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>

#define DEADLINE_BY_CHECKPOINT_ANNOTATION "dl_chk"

class DeadlineByCheckpoint : public llvm::PassInfoMixin<DeadlineByCheckpoint> {
public:
  // wrapper around runOnModule
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &);

  // This is more a convention, rather than a strict API rule (as far as I can
  // tell). It does the interesting work on the pass.
  bool runOnModule(llvm::Module &M);

  // We say that even with the optnone LLVM attribute, this should be run.
  static bool isRequired() { return true; }
};

#endif
