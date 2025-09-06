//===-- TricoreBaseInfo.h - Top level definitions for RISC-V MC ---*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone enum definitions for the RISC-V target
// useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_TRICORE_MCTARGETDESC_TRICOREBASEINFO_H
#define LLVM_LIB_TARGET_TRICORE_MCTARGETDESC_TRICOREBASEINFO_H

#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/TargetParser/SubtargetFeature.h"
#include <cstdint>
// #include "llvm/TargetParser/TricoreISAInfo.h"
// #include "llvm/TargetParser/TricoreTargetParser.h"

namespace llvm {
namespace TricoreOp {
enum OperandType : unsigned {
  OPERAND_FIRST_Tricore_IMM = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_SIMM4 = OPERAND_FIRST_Tricore_IMM,
  OPERAND_SIMM9,
  OPERAND_SIMM10,
  OPERAND_SIMM16,
  OPERAND_UIMM4,
  OPERAND_UIMM5,
  OPERAND_UIMM8,
  OPERAND_UIMM9,
  OPERAND_UIMM16,
  OPERAND_DISP24,
  OPERAND_DISP15,
  OPERAND_DISP8,
  OPERAND_DISP4,
  OPERAND_POS,
  OPERAND_WIDTH,
};
}
namespace TricoreII {
enum {
  InstFormatPseudo = 0,
  InstFormatABS = 1,
  InstFormatABSB = 2,
  InstFormatB = 3,
  InstFormatBIT = 4,
  InstFormatBO = 5,
  InstFormatBOL = 6,
  InstFormatBRC = 7,
  InstFormatBRN = 8,
  InstFormatBRR = 9,
  InstFormatRC = 10,
  InstFormatRCPW = 11,
  InstFormatRCR = 12,
  InstFormatRCRR = 13,
  InstFormatRCRW = 14,
  InstFormatRLC = 15,
  InstFormatRR = 16,
  InstFormatRR1 = 17,
  InstFormatRR2 = 18,
  InstFormatRRPW = 19,
  InstFormatRRR = 20,
  InstFormatRRR1 = 21,
  InstFormatRRR2 = 22,
  InstFormatRRRR = 23,
  InstFormatRRRW = 24,
  InstFormatSYS = 25,
  InstFormatSB = 26,
  InstFormatSBC = 27,
  InstFormatSBR = 28,
  InstFormatSBRN = 29,
  InstFormatSC = 30,
  InstFormatSLR = 31,
  InstFormatSLRO = 32,
  InstFormatSR = 33,
  InstFormatSRC = 34,
  InstFormatSRO = 35,
  InstFormatSRR = 36,
  InstFormatSRRS = 37,
  InstFormatSSR = 38,
  InstFormatSSRO = 39,

  InstFormatMask = 63,
  InstFormatShift = 0,
};

enum {
  MO_None = 0,
  MO_CALL = 1,
  MO_HI = 2,
  MO_LO = 4,
  MO_LO2 = 8,
};
// Helper functions to read TSFlags.
/// \returns the format of the instruction.
static inline unsigned getFormat(uint64_t TSFlags) {
  return (TSFlags & InstFormatMask) >> InstFormatShift;
}

static inline bool isAbsolute(uint64_t TSFlags) {
  return (TSFlags & (1 << 6)) != 0;
}
} // namespace TricoreII
namespace TricoreCI {
bool compress(MCInst &OutInst, const MCInst &MI, const MCSubtargetInfo &STI);
bool uncompress(MCInst &OutInst, const MCInst &MI, const MCSubtargetInfo &STI);
} // namespace TricoreCI
} // namespace llvm
#endif