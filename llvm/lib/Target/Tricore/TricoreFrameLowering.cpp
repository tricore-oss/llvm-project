//===-- TricoreFrameLowering.cpp - Tricore Frame Information
//------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Tricore implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "TricoreFrameLowering.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "TricoreInstrInfo.h"
#include "TricoreMachineFunctionInfo.h"
#include "TricoreSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

TricoreFrameLowering::TricoreFrameLowering(const TricoreSubtarget &ST)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0,
                          Align(8),
                          /*StackRealignable=*/false) {}

void TricoreFrameLowering::emitSPAdjustment(MachineFunction &MF,
                                            MachineBasicBlock &MBB,
                                            MachineBasicBlock::iterator MBBI,
                                            int NumBytes) const {
  DebugLoc DL;
  const TricoreInstrInfo &TII =
      *static_cast<const TricoreInstrInfo *>(MF.getSubtarget().getInstrInfo());
  if (isInt<10>(NumBytes)) {
    BuildMI(MBB, MBBI, DL, TII.get(Tricore::LEAbo), Tricore::A10)
        .addReg(Tricore::A10)
        .addImm(NumBytes);
  } else if (isInt<16>(NumBytes)) {
    BuildMI(MBB, MBBI, DL, TII.get(Tricore::LEAbol), Tricore::A10)
        .addReg(Tricore::A10)
        .addImm(NumBytes);
  }
}

void TricoreFrameLowering::emitPrologue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();

  // Get the number of bytes to allocate from the FrameInfo
  int FrameSize = (int)MFI.getStackSize();

  // Reserve space for call frame if known
  if (MFI.adjustsStack() && hasReservedCallFrame(MF))
    FrameSize += MFI.getMaxCallFrameSize();

  if (FrameSize == 0)
    return;

  FrameSize = alignTo(FrameSize, getStackAlign());

  // Finally, ensure that the size is sufficiently aligned for the
  // data on the stack.
  FrameSize = alignTo(FrameSize, MFI.getMaxAlign());

  // Update stack size with corrected value.
  MFI.setStackSize(FrameSize);

  emitSPAdjustment(MF, MBB, MBBI, -FrameSize);
}

MachineBasicBlock::iterator TricoreFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {

  DebugLoc DL = MI->getDebugLoc();

  if (!hasReservedCallFrame(MF)) {
    // If space has not been reserved for a call frame, ADJCALLSTACKDOWN and
    // ADJCALLSTACKUP must be converted to instructions manipulating the stack
    // pointer. This is necessary when there is a variable length stack
    // allocation (e.g. alloca), which means it's not possible to allocate
    // space for outgoing arguments from within the function prologue.
    int64_t Amount = MI->getOperand(0).getImm();

    if (Amount != 0) {
      // Ensure the stack remains aligned after adjustment.
      Amount = alignSPAdjust(Amount);

      if (MI->getOpcode() == Tricore::ADJCALLSTACKDOWN)
        Amount = -Amount;

      emitSPAdjustment(MF, MBB, MI, Amount);
    }
  }

  return MBB.erase(MI);
}

void TricoreFrameLowering::emitEpilogue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {}

bool TricoreFrameLowering::hasReservedCallFrame(
    const MachineFunction &MF) const {
  // Reserve call frame if there are no variable sized objects on the stack.
  return !MF.getFrameInfo().hasVarSizedObjects();
}

// hasFPImpl - Return true if the specified function should have a dedicated
// frame pointer register.  This is true if the function has variable sized
// allocas or if frame pointer elimination is disabled.
bool TricoreFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken();
}

StackOffset
TricoreFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                             Register &FrameReg) const {
  const TricoreSubtarget &Subtarget = MF.getSubtarget<TricoreSubtarget>();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TricoreRegisterInfo *RegInfo = Subtarget.getRegisterInfo();
  const TricoreMachineFunctionInfo *FuncInfo =
      MF.getInfo<TricoreMachineFunctionInfo>();

  int64_t FrameOffset = MF.getFrameInfo().getObjectOffset(FI);

  if (hasFP(MF)) {
    FrameReg = RegInfo->getFrameRegister(MF);
    return StackOffset::getFixed(FrameOffset);
  }
  FrameReg = Tricore::A10;
  return StackOffset::getFixed(FrameOffset + MF.getFrameInfo().getStackSize());
}


static bool LLVM_ATTRIBUTE_UNUSED
verifyLeafProcRegUse(MachineRegisterInfo *MRI) {
  return true;
}

bool TricoreFrameLowering::isLeafProc(MachineFunction &MF) const {}

void TricoreFrameLowering::remapRegsForLeafProc(MachineFunction &MF) const {}

void TricoreFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                                BitVector &SavedRegs,
                                                RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
}
