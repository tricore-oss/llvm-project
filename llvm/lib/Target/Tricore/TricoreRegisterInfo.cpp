//===- TricoreRegisterInfo.cpp - TRICORE Register Information
//-------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the TRICORE implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//

#include "TricoreRegisterInfo.h"
#include "Tricore.h"
#include "TricoreMachineFunctionInfo.h"
#include "TricoreSubtarget.h"
#include "TricoreTargetMachine.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "tricore-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "TricoreGenRegisterInfo.inc"

TricoreRegisterInfo::TricoreRegisterInfo()
    : TricoreGenRegisterInfo(Tricore::A11) {}

/// Code Generation virtual methods...
const MCPhysReg *
TricoreRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_EABI_SaveList;
}
const uint32_t *
TricoreRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                          CallingConv::ID CC) const {
  return CSR_EABI_RegMask;
}

BitVector
TricoreRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(Tricore::A10);
  Reserved.set(Tricore::A11);
  Reserved.set(Tricore::PC);
  Reserved.set(Tricore::PSW);
  Reserved.set(Tricore::PCXI);

  return Reserved;
}

const TargetRegisterClass *
TricoreRegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                        unsigned Kind) const {
  return &Tricore::AGPRRegClass;
}

bool TricoreRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                              int SPAdj, unsigned FIOperandNum,
                                              RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  DebugLoc DL = MI.getDebugLoc();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  Register FrameReg;
  StackOffset Offset =
      getFrameLowering(MF)->getFrameIndexReference(MF, FrameIndex, FrameReg);

  if (!isInt<32>(Offset.getFixed())) {
    report_fatal_error(
        "Frame offsets outside of the signed 32-bit range not supported");
  }

  return false;
}

Register
TricoreRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return Tricore::A10;
}