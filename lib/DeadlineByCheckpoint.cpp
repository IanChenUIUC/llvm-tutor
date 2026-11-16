#include "DeadlineByCheckpoint.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include <llvm/IR/Analysis.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Debug.h>

using namespace llvm;

#define DEBUG_TYPE "deadline-by-checkpoint"

// This function is writen by Claude
static SmallPtrSet<Function *, 8> getAnnotatedFunctions(Module &M,
                                                        StringRef Tag) {
  SmallPtrSet<Function *, 8> Result;

  auto *Annotations = M.getNamedGlobal("llvm.global.annotations");
  if (!Annotations || !Annotations->hasInitializer())
    return Result;

  auto *Entries = dyn_cast<ConstantArray>(Annotations->getInitializer());
  if (!Entries)
    return Result;

  for (Value *Op : Entries->operands()) {
    auto *Entry = dyn_cast<ConstantStruct>(Op);
    if (!Entry)
      continue;

    auto *F = dyn_cast<Function>(Entry->getOperand(0)->stripPointerCasts());
    auto *StrGV =
        dyn_cast<GlobalVariable>(Entry->getOperand(1)->stripPointerCasts());
    if (!F || !StrGV || !StrGV->hasInitializer())
      continue;

    auto *Str = dyn_cast<ConstantDataArray>(StrGV->getInitializer());
    if (Str && Str->isCString() && Str->getAsCString() == Tag)
      Result.insert(F);
  }
  return Result;
}

bool DeadlineByCheckpoint::runOnModule(Module &M) {
  // the LLVMContext has a lot of relevant information for IR building
  // and anything else that we would have to do with LLVM
  auto &CTX = M.getContext();
  auto Deadlined = getAnnotatedFunctions(M, DEADLINE_BY_CHECKPOINT_ANNOTATION);

  // create declarations (or get) the runtime func (global variables)
  FunctionType *VoidFTy = FunctionType::get(Type::getVoidTy(CTX), false);
  FunctionCallee RuntimeEnter =
      M.getOrInsertFunction("DeadlineCheckpointRuntimeEnter", VoidFTy);
  FunctionCallee RuntimeExit =
      M.getOrInsertFunction("DeadlineCheckpointRuntimeExit", VoidFTy);
  FunctionCallee RuntimeCheck =
      M.getOrInsertFunction("DeadlineCheckpointRuntimeCheck", VoidFTy);

  // all functions become instrumented with the Checks.
  // for function calls into DeadlineByCheckpoint annotation, insert Enter/Exit
  // immediately before/after (alternative: find terminations, which is hard).
  // if there are exceptions, there are probably correctness issues...
  bool InsertedInstrumentation = false;
  for (auto &F : M) {
    IRBuilder<> Builder(CTX);
    auto FuncName = F.getName();
    for (auto &BB : F) {
      // Now, we invalidate all Analyses passes because we insert these calls
      InsertedInstrumentation = true;

      LLVM_DEBUG(dbgs() << " Injecting call to check inside " << FuncName << ":"
                        << BB.getName() << "\n");
      Builder.SetInsertPoint(BB.getFirstInsertionPt());
      Builder.CreateCall(RuntimeCheck);

      for (auto &Inst : BB) {
        if (CallInst *CI = dyn_cast<CallInst>(&Inst)) {
          Function *callee = cast<Function>(CI->getCalledOperand());
          if (Deadlined.contains(callee)) {
            LLVM_DEBUG(dbgs() << " Injecting call to enter/exit inside "
                              << FuncName << ":" << BB.getName() << "\n");
            Builder.SetInsertPoint(&Inst);
            Builder.CreateCall(RuntimeEnter);
            Builder.SetInsertPoint(Inst.getNextNode());
            Builder.CreateCall(RuntimeExit);
          }
        }
      }
    }
  }
  return InsertedInstrumentation;
}

PreservedAnalyses DeadlineByCheckpoint::run(Module &M,
                                            ModuleAnalysisManager &) {
  bool Changed = runOnModule(M);
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

llvm::PassPluginLibraryInfo getInjectFuncCallPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "deadline-checkpoint", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "deadline-checkpoint") {
                    MPM.addPass(DeadlineByCheckpoint());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getInjectFuncCallPluginInfo();
}
