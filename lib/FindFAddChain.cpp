//=============================================================================
// FILE:
//    FindFAddChain.cpp
//
// DESCRIPTION:
//    Visits all instructions in a function and returns all chains of FP adds
//    of size >1. The results can be printed through the use of a printing pass.
//
// USAGE:
//      opt --load-pass-plugin libFindFAddChain.dylib `\`
//        --passes='print<find-fadd-chain>' --disable-output <input-llvm-file>
//
// License: MIT
//=============================================================================
#include "FindFAddChain.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/IR/InstrTypes.h>

#include <algorithm>
#include <string>

using namespace llvm;

static void printFAddChains(raw_ostream &OS, Function &Func,
                            const FindFAddChain::Result &FAddChains) noexcept {
  if (FAddChains.empty())
    return;

  OS << "Floating-point add chains in \"" << Func.getName() << "\":\n";

  // Using a ModuleSlotTracker for printing makes it so full function analysis
  // for slot numbering only occurs once instead of every time an instruction
  // is printed.
  ModuleSlotTracker Tracker(Func.getParent());

  for (auto &Chain : FAddChains) {
    for (auto *Inst : Chain) {
      Inst->print(OS, Tracker);
      OS << '\n';
    }
    OS << '\n';
  }
}

static constexpr char PassArg[] = "find-fadd-chain";
static constexpr char PassName[] = "Floating-point addition chains locator";
static constexpr char PluginName[] = "FindFAddChain";

//------------------------------------------------------------------------------
// FindFAddChain implementation
//------------------------------------------------------------------------------
FindFAddChain::Result FindFAddChain::run(Function &Func,
                                         FunctionAnalysisManager &FAM) {
  return run(Func);
}

FindFAddChain::Result FindFAddChain::run(Function &Func) {
  Result Chains;
  for (Instruction &Inst : instructions(Func)) {
    auto *BinOp = dyn_cast<BinaryOperator>(&Inst);
    if (!BinOp || BinOp->getOpcode() != Instruction::FAdd)
      continue;

    if (BinOp->hasOneUse()) {
      Use &OnlyUse = *BinOp->use_begin();
      auto *Next = dyn_cast<BinaryOperator>(OnlyUse.getUser());
      if (Next && Next->getOpcode() == Instruction::FAdd &&
          OnlyUse.getOperandNo() == 0)
        continue;
    }

    BinaryOperator *U;
    std::vector<BinaryOperator *> Chain{BinOp};
    while ((U = dyn_cast<BinaryOperator>(BinOp->getOperand(0))) &&
           U->getOpcode() == Instruction::FAdd && U->hasOneUse()) {
      Chain.push_back(U);
      BinOp = U;
    }

    std::reverse(Chain.begin(), Chain.end());
    if (Chain.size() > 1)
      Chains.push_back(Chain);
  }

  return Chains;
}

PreservedAnalyses FindFAddChainPrinter::run(Function &Func,
                                            FunctionAnalysisManager &FAM) {
  auto &Comparisons = FAM.getResult<FindFAddChain>(Func);
  printFAddChains(OS, Func, Comparisons);
  return PreservedAnalyses::all();
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::AnalysisKey FindFAddChain::Key;

PassPluginLibraryInfo getFindFAddChainPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, PluginName, LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            // #1 REGISTRATION FOR "FAM.getResult<FindFAddChain>(Function)"
            PB.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager &FAM) {
                  FAM.registerPass([&] { return FindFAddChain(); });
                });
            // #2 REGISTRATION FOR "opt -passes=print<find-fadd-chain>"
            PB.registerPipelineParsingCallback(
                [&](StringRef Name, FunctionPassManager &FPM,
                    ArrayRef<PassBuilder::PipelineElement>) {
                  std::string PrinterPassElement =
                      formatv("print<{0}>", PassArg);
                  if (!Name.compare(PrinterPassElement)) {
                    FPM.addPass(FindFAddChainPrinter(llvm::outs()));
                    return true;
                  }

                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getFindFAddChainPluginInfo();
}
