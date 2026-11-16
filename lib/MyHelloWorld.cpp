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

#include "MyHelloWorld.h"
#include <llvm/IR/Analysis.h>
#include <llvm/Support/raw_ostream.h>
#include <vector>

using namespace llvm;

// This is to identify the AnalysisPass for the caching to work.
llvm::AnalysisKey MyHelloWorld::Key;

MyHelloWorld::Result MyHelloWorld::run(Function &F, FunctionAnalysisManager &) {
  errs() << "(me-llvm-tutor) " << "MyHelloWorld::run() " << F.getName() << "\n";

  auto typeString = [](Type *Ty) -> std::string {
    std::string ret;
    llvm::raw_string_ostream rso(ret);
    Ty->print(rso);
    return rso.str();
  };

  std::vector<std::string> ParameterType;
  for (auto &Arg : F.args())
    ParameterType.push_back(typeString(Arg.getType()));

  return MyHelloWorld::Result{.FuncName = std::string(F.getName()),
                              .ReturnType = typeString(F.getReturnType()),
                              .ParameterType = ParameterType};
}

PreservedAnalyses MyHelloWorldPrinter::run(Function &F,
                                           FunctionAnalysisManager &FAM) {
  auto &Result = FAM.getResult<MyHelloWorld>(F);

  OS << "(me-llvm-tutor) " << "MyHelloWorldPrinter::run()\n";
  OS << "\t" << Result.ReturnType << " " << Result.FuncName << "(";

  bool first = true;
  for (const auto &Str : Result.ParameterType) {
    if (!first)
      OS << ", ";
    OS << Str;
    first = false;
  }
  OS << ")\n";

  return PreservedAnalyses::all();
}

// Registration: this [ex](https://llvm.org/docs/WritingAnLLVMNewPMPass.html)
// shows it as a cmake project config. Because we are doing out-of-tree, I guess
// we do it from this extenral function.
//
// Roughly, in opt, we pass a -passes= option, that needs to be parsed.
// If the pass is named "my-hello-world", the registered Callback adds a new
// Pass with MyHelloWorld.
//
// Aside: the ArrayRef<..> is the nested pipeline (e.g. foo(bar,baz)).
PassPluginLibraryInfo getMyHelloWorldPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MyHelloWorld", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "print<my-hello-world>") {
                    FPM.addPass(MyHelloWorldPrinter(llvm::errs()));
                    return true;
                  }
                  return false;
                });
            PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager &FAM) {
                  FAM.registerPass([&] { return MyHelloWorld(); });
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getMyHelloWorldPluginInfo();
}
