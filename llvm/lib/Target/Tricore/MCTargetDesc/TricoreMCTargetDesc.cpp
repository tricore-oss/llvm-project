//===-- TricoreMCTargetDesc.cpp - Tricore Target Descriptions -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Tricore specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "TricoreMCTargetDesc.h"
#include "TricoreAsmBackend.h"
#include "TricoreBaseInfo.h"
#include "TricoreELFStreamer.h"
#include "TricoreInstPrinter.h"
#include "TricoreMCAsmInfo.h"
#include "TricoreTargetStreamer.h"
#include "TargetInfo/TricoreTargetInfo.h"
#include "llvm/DebugInfo/CodeView/CodeView.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "TricoreGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "TricoreGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "TricoreGenRegisterInfo.inc"

void TRICORE_MC::initLLVMToCVRegMapping(MCRegisterInfo *MRI) {
 // FIXME: This is a hack to get the correct mapping for the
}

/// Select the Tricore CPU for the given triple and cpu name.
StringRef TRICORE_MC::selectTricoreCPU(const Triple &TT, StringRef CPU) {
  // FIXME: This is a hack to get the correct mapping for the
  return CPU;
}

static MCInstrInfo *createTricoreMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitTricoreMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createTricoreMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitTricoreMCRegisterInfo(X, Tricore::A11);
  return X;
}

static MCSubtargetInfo *createTricoreMCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  CPU = TRICORE_MC::selectTricoreCPU(TT, CPU);
  return createTricoreMCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCAsmInfo *createTricoreMCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  MCAsmInfo *MAI;
  MAI = new TricoreELFMCAsmInfo(TT);

  // unsigned SP = MRI.getDwarfRegNum(Tricore::SP, true);
  // MCCFIInstruction Inst = MCCFIInstruction::createDefCfaRegister(nullptr, SP);
  // MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCInstPrinter *createTricoreMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new TricoreInstPrinter(MAI, MII, MRI);
}

static MCStreamer *createMCStreamer(const Triple &T, MCContext &Context,
                                    std::unique_ptr<MCAsmBackend> &&MAB,
                                    std::unique_ptr<MCObjectWriter> &&OW,
                                    std::unique_ptr<MCCodeEmitter> &&Emitter) {
  MCStreamer *S;
  S = createTricoreELFStreamer(Context, std::move(MAB), std::move(OW),
                            std::move(Emitter));
  return S;
}

static MCTargetStreamer *createTricoreAsmTargetStreamer(MCStreamer &S,
                                                     formatted_raw_ostream &OS,
                                                     MCInstPrinter *InstPrint) {
  return new TricoreTargetAsmStreamer(S, OS);
}

static MCTargetStreamer *createTricoreNullTargetStreamer(MCStreamer &S) {
  return new TricoreTargetStreamer(S);
}

static MCTargetStreamer *
createTricoreObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new TricoreTargetELFStreamer(S, STI);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTricoreTargetMC() {
  for (Target *T : {&getTheTricoreTarget()}) {
    // Register the MC asm info.
    RegisterMCAsmInfoFn X(*T, createTricoreMCAsmInfo);

       // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createTricoreMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createTricoreMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createTricoreMCSubtargetInfo);

    // Register the MC Code Emitter.
    TargetRegistry::RegisterMCCodeEmitter(*T, createTricoreMCCodeEmitter);

    // Register the asm backend.
    TargetRegistry::RegisterMCAsmBackend(*T, createTricoreAsmBackend);

    // Register the object target streamer.
    TargetRegistry::RegisterObjectTargetStreamer(*T,
                                                 createTricoreObjectTargetStreamer);
    
    // Register the elf streamer. 
    TargetRegistry::RegisterELFStreamer(*T, createMCStreamer);

    // Register the asm streamer.
    TargetRegistry::RegisterAsmTargetStreamer(*T, createTricoreAsmTargetStreamer);

    // Register the null streamer.
    TargetRegistry::RegisterNullTargetStreamer(*T, createTricoreNullTargetStreamer);

    // Register the MCInstPrinter
    TargetRegistry::RegisterMCInstPrinter(*T, createTricoreMCInstPrinter);
  }
}
