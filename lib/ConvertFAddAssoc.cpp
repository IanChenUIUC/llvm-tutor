//=============================================================================
// FILE:
//    ConvertFAddAssoc.cpp
//
// DESCRIPTION:
//    Transformation pass which uses the results of the FindFAddChain analysis
//    pass to reverse the order of all chains of floating point adds.
//
//    This example demonstrates how to couple an analysis pass with a
//    transformation pass, the use of statistics (the STATISTIC macro), and LLVM
//    debugging operations (the LLVM_DEBUG macro and the llvm::dbgs() output
//    stream). It also demonstrates how instructions can be modified without
//    having to completely replace them.
//
//    Originally developed for [1].
//
//    [1] "Writing an LLVM Optimization" by Jonathan Smith
//
// USAGE:
//      opt --load-pass-plugin libConvertFAddAssoc.dylib [--stats] `\`
//        --passes='convert-fcmp-eq' --disable-output <input-llvm-file>
//
// License: MIT
//=============================================================================
#include "ConvertFAddAssoc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <llvm/IR/InstrTypes.h>

using namespace llvm;

void convertFAddAssocInstruction(std::vector<BinaryOperator *> Chain) {
  assert(Chain.size() && "Got empty chain");
  assert(Chain[0]->getModule() &&
         "The given chain does not belong to a module");

  Module *M = Chain[0]->getModule();
  LLVMContext &CTX = M->getContext();

  // a0 = x0 + x1
  // a1 = a0 + x2
  // a2 = a1 + x3
  //
  // (((x0 + x1) + x2) + x3)
  // (x0 + (x1 + (x2 + x3)))
  //
  // a0 = x3 + x2
  // a1 = a0 + x1
  // a2 = a1 + x0

  std::vector<Value *> ops;
  ops.push_back(Chain[0]->getOperand(0));
  for (auto *op : Chain)
    ops.push_back(op->getOperand(1));
  std::reverse(ops.begin(), ops.end());

  IRBuilder<> Builder(CTX);
  Builder.SetInsertPoint(Chain.back());

  Value *last = ops[0];
  for (int i = 1; i < ops.size(); ++i)
    last = Builder.CreateFAdd(last, ops[i]);

  Chain.back()->replaceAllUsesWith(last);
  for (auto op = Chain.rbegin(); op != Chain.rend(); ++op)
    (*op)->eraseFromParent();
}

static constexpr char PassArg[] = "convert-fadd-assoc";
static constexpr char PluginName[] = "ConvertFAddAssoc";

#define DEBUG_TYPE ::PassArg
STATISTIC(FAddAssocConversionCount,
          "Number of re-ordered FAdd operations converted");

//------------------------------------------------------------------------------
// ConvertFAddAssoc implementation
//------------------------------------------------------------------------------
PreservedAnalyses ConvertFAddAssoc::run(Function &Func,
                                        FunctionAnalysisManager &FAM) {
  auto &Chains = FAM.getResult<FindFAddChain>(Func);
  bool Modified = false;

  if (Func.hasFnAttribute(Attribute::OptimizeNone)) {
    LLVM_DEBUG(dbgs() << "Ignoring optnone-marked function \"" << Func.getName()
                      << "\"\n");
    Modified = false;
  } else {
    for (auto &Chain : Chains) {
      convertFAddAssocInstruction(Chain);
      FAddAssocConversionCount += Chain.size();
      Modified = true;
    }
  }

  return Modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
PassPluginLibraryInfo getConvertFAddAssocPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, PluginName, LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [&](StringRef Name, FunctionPassManager &FPM,
                    ArrayRef<PassBuilder::PipelineElement>) {
                  if (!Name.compare(PassArg)) {
                    FPM.addPass(ConvertFAddAssoc());
                    return true;
                  }

                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getConvertFAddAssocPluginInfo();
}
