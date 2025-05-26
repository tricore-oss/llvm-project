//===-- TricoreTargetStreamer.cpp - Tricore Target Streamer Methods -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Tricore specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "TricoreTargetStreamer.h"
#include "TricoreInstPrinter.h"
#include "TricoreMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

// pin vtable to this file
TricoreTargetStreamer::TricoreTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void TricoreTargetStreamer::anchor() {}

TricoreTargetAsmStreamer::TricoreTargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS)
    : TricoreTargetStreamer(S), OS(OS) {}

void TricoreTargetAsmStreamer::emitTricoreRegisterIgnore(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(TricoreInstPrinter::getRegisterName(reg)).lower()
     << ", #ignore\n";
}

void TricoreTargetAsmStreamer::emitTricoreRegisterScratch(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(TricoreInstPrinter::getRegisterName(reg)).lower()
     << ", #scratch\n";
}

TricoreTargetELFStreamer::TricoreTargetELFStreamer(MCStreamer &S,
                                               const MCSubtargetInfo &STI)
    : TricoreTargetStreamer(S) {
  ELFObjectWriter &W = getStreamer().getWriter();
  unsigned EFlags = W.getELFHeaderEFlags();

  W.setELFHeaderEFlags(EFlags);
}

MCELFStreamer &TricoreTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
