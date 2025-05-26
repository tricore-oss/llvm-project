//===- TricoreInstrInfo.cpp - Tricore Instruction Information -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Tricore implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "TricoreInstrInfo.h"
#include "MCTargetDesc/TricoreBaseInfo.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "Tricore.h"
#include "TricoreSubtarget.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Target/TargetMachine.h"
#include <cassert>

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "TricoreGenInstrInfo.inc"

// Pin the vtable to this file.
void TricoreInstrInfo::anchor() {}

TricoreInstrInfo::TricoreInstrInfo(TricoreSubtarget &ST)
    : TricoreGenInstrInfo(Tricore::ADJCALLSTACKDOWN, Tricore::ADJCALLSTACKUP), RI(),
      Subtarget(ST) {}