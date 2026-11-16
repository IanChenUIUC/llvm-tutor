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

#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Analysis.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Plugins/PassPlugin.h>
#include <llvm/Support/raw_ostream.h>

class MyHelloWorld : public llvm::AnalysisInfoMixin<MyHelloWorld> {
public:
  struct Result {
    std::string FuncName;
    std::string ReturnType;
    std::vector<std::string> ParameterType;
  };

  // The MyHelloWorld feature has two passes:
  // First, we visit each Function (run at one IR level), and create Analysis.
  // This is run exactly one and then cached (or if Invalidated), as in log.
  Result run(llvm::Function &F, llvm::FunctionAnalysisManager &);

  // We say that even with the optnone LLVM attribute, this should be run.
  static bool isRequired() { return true; }

private:
  // A special type used by analysis passes to provide an address that
  // identifies that particular analysis pass type.
  static llvm::AnalysisKey Key;
  friend struct llvm::AnalysisInfoMixin<MyHelloWorld>;
};

class MyHelloWorldPrinter : public llvm::PassInfoMixin<MyHelloWorldPrinter> {
public:
  explicit MyHelloWorldPrinter(llvm::raw_ostream &OS) : OS(OS) {}

  // The second part of MyHelloWorld feature:
  // Use the Analysis result from MyHelloWorld above to do the read-only pass.
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &);

  // We say that even with the optnone LLVM attribute, this should be run.
  static bool isRequired() { return true; }

private:
  llvm::raw_ostream &OS;
};

#endif
