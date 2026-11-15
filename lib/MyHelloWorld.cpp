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

using namespace llvm;

PreservedAnalyses MyHelloWorld::run(Function &F, FunctionAnalysisManager &) {
  errs() << "(me-llvm-tutor) " << F.getName() << ": " << *F.getFunctionType()
         << "\n";
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
                  if (Name == "my-hello-world") {
                    FPM.addPass(MyHelloWorld());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getMyHelloWorldPluginInfo();
}
