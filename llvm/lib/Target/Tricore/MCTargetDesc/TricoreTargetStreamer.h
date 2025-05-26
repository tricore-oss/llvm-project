//===-- TricoreTargetStreamer.h - Tricore Target Streamer ----------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCTARGETSTREAMER_H
#define LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCTARGETSTREAMER_H

#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

class formatted_raw_ostream;

class TricoreTargetStreamer : public MCTargetStreamer {
  virtual void anchor();

public:
  TricoreTargetStreamer(MCStreamer &S);
  /// Emit ".register <reg>, #ignore".
  virtual void emitTricoreRegisterIgnore(unsigned reg){};
  /// Emit ".register <reg>, #scratch".
  virtual void emitTricoreRegisterScratch(unsigned reg){};
};

// This part is for ascii assembly output
class TricoreTargetAsmStreamer : public TricoreTargetStreamer {
  formatted_raw_ostream &OS;

public:
  TricoreTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
  void emitTricoreRegisterIgnore(unsigned reg) override;
  void emitTricoreRegisterScratch(unsigned reg) override;
};

// This part is for ELF object output
class TricoreTargetELFStreamer : public TricoreTargetStreamer {
public:
  TricoreTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);
  MCELFStreamer &getStreamer();
  void emitTricoreRegisterIgnore(unsigned reg) override {}
  void emitTricoreRegisterScratch(unsigned reg) override {}
};
} // end namespace llvm

#endif
