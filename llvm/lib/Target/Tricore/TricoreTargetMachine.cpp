//===-- TricoreTargetMachine.cpp - Define TargetMachine for Tricore
//-------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Tricore target spec.
//
//===----------------------------------------------------------------------===//

#include "TricoreTargetMachine.h"
#include "TargetInfo/TricoreTargetInfo.h"
#include "Tricore.h"
#include "TricoreMachineFunctionInfo.h"
#include "TricoreTargetObjectFile.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/CodeGen/GlobalISel/CSEInfo.h"
#include "llvm/CodeGen/GlobalISel/IRTranslator.h"
#include "llvm/CodeGen/GlobalISel/InstructionSelect.h"
#include "llvm/CodeGen/GlobalISel/Legalizer.h"
#include "llvm/CodeGen/GlobalISel/RegBankSelect.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/InitializePasses.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetOptions.h"
#include <optional>
#include <string>

using namespace llvm;

#define DEBUG_TYPE "tricore"

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTricoreTarget() {
  // Register the target.
  RegisterTargetMachine<TricoreTargetMachine> X(getTheTricoreTarget());

  PassRegistry *PR = PassRegistry::getPassRegistry();
  initializeGlobalISel(*PR);
  initializeTricoreDAGToDAGISelLegacyPass(*PR);
}

static std::unique_ptr<TargetLoweringObjectFile> createTLOF(const Triple &TT) {
  return std::make_unique<TricoreTargetObjectFile>();
}

static std::string computeDataLayout(const Triple &TT, StringRef CPU,
                                     const TargetOptions &Options) {
  return "e-p:32:32-S64-i64:32:64-f64:32:64-a:0:32-n32";
}

static Reloc::Model getEffectiveRelocModel(bool JIT,
                                           std::optional<Reloc::Model> RM) {
  if (!RM || JIT)
    return Reloc::Static;
  return *RM;
}

TricoreTargetMachine::TricoreTargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           std::optional<Reloc::Model> RM,
                                           std::optional<CodeModel::Model> CM,
                                           CodeGenOptLevel OL, bool JIT)
    : CodeGenTargetMachineImpl(T, computeDataLayout(TT, CPU, Options), TT, CPU,
                               FS, Options, getEffectiveRelocModel(JIT, RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(createTLOF(getTargetTriple())) {
  initAsmInfo();

  // FIXME:
  setSupportsDebugEntryValues(false);
}

TricoreTargetMachine::~TricoreTargetMachine() = default;

const TricoreSubtarget *
TricoreTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute TuneAttr = F.getFnAttribute("tune-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  std::string TuneCPU =
      TuneAttr.isValid() ? TuneAttr.getValueAsString().str() : CPU;
  std::string FS =
      FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;

  // FIXME: This is related to the code below to reset the target options,
  // we need to know whether or not the soft float flag is set on the
  // function, so we can enable it as a subtarget feature.
  bool softFloat = F.getFnAttribute("use-soft-float").getValueAsBool();

  if (softFloat)
    FS += FS.empty() ? "+soft-float" : ",+soft-float";

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<TricoreSubtarget>(CPU, TuneCPU, FS, *this);
  }
  return I.get();
}

void TricoreTargetMachine::resetSubtarget(MachineFunction *MF) {
  LLVM_DEBUG(dbgs() << "resetSubtarget\n");

  Subtarget = &MF->getSubtarget<TricoreSubtarget>();
}

MachineFunctionInfo *TricoreTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return TricoreMachineFunctionInfo::create<TricoreMachineFunctionInfo>(
      Allocator, F, STI);
}

namespace {

/// Tricore Code Generator Pass Configuration Options.
class TricorePassConfig : public TargetPassConfig {
public:
  TricorePassConfig(TricoreTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {
  }

  TricoreTargetMachine &getTricoreTargetMachine() const {
    return getTM<TricoreTargetMachine>();
  }

  // const TricoreSubtarget &getTricoreSubtarget() const {
  //   return *getTricoreTargetMachine().getSubtargetImpl();
  // }

  // void addIRPasses() override;
  bool addInstSelector() override;
  // void addPreEmitPass() override;
  // void addPreRegAlloc() override;
  // bool addIRTranslator() override;
  // void addPreLegalizeMachineIR() override;
  // bool addLegalizeMachineIR() override;
  // void addPreRegBankSelect() override;
  // bool addRegBankSelect() override;
  // bool addGlobalInstructionSelect() override;

  // std::unique_ptr<CSEConfigBase> getCSEConfig() const override;
};

bool TricorePassConfig::addInstSelector() {
  addPass(createTricoreISelDag(getTricoreTargetMachine()));
  return false;
}

} // end anonymous namespace

TargetPassConfig *TricoreTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new TricorePassConfig(*this, PM);
}
