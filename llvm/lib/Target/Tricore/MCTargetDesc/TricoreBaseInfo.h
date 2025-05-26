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
//#include "llvm/TargetParser/TricoreISAInfo.h"
//#include "llvm/TargetParser/TricoreTargetParser.h"


namespace llvm {
namespace TricoreOp {
enum OperandType : unsigned {
  OPERAND_FIRST_Tricore_IMM = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_SIMM10 = OPERAND_FIRST_Tricore_IMM,
  OPERAND_SIMM16,
  OPERAND_UIMM8,
};
}
} // namespace llvm
#endif