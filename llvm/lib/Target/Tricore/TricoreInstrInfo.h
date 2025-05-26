//===- TricoreInstrInfo.h - Tricore Instruction Information -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Tricore implementation of the TargetInstrInfo class.
//
// FIXME: We need to override TargetInstrInfo::getInlineAsmLength method in
// order for TricoreLongBranch pass to work correctly when the code has inline
// assembly.  The returned value doesn't have to be the asm instruction's exact
// size in bytes; TricoreLongBranch only expects it to be the correct upper bound.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRICORE_TRICOREINSTRINFO_H
#define LLVM_LIB_TARGET_TRICORE_TRICOREINSTRINFO_H

#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "Tricore.h"
#include "TricoreRegisterInfo.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include <cstdint>

#define GET_INSTRINFO_HEADER
#include "TricoreGenInstrInfo.inc"

namespace llvm {

class MachineInstr;
class MachineOperand;
class TricoreSubtarget;
class TargetRegisterClass;
class TargetRegisterInfo;

class TricoreInstrInfo : public TricoreGenInstrInfo {
  const TricoreRegisterInfo RI;
  const TricoreSubtarget &Subtarget;
  virtual void anchor();

protected:

public:
  explicit TricoreInstrInfo(TricoreSubtarget &ST);

  static const TricoreInstrInfo *create(TricoreSubtarget &STI);

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const TricoreRegisterInfo &getRegisterInfo() const { return RI; }
};


} // end namespace llvm

#endif // LLVM_LIB_TARGET_TRICORE_TRICOREINSTRINFO_H
