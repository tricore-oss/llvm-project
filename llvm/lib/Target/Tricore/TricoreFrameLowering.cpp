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
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

TricoreFrameLowering::TricoreFrameLowering(const TricoreSubtarget &ST)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0,
                          Align(8),
                          /*StackRealignable=*/false) {}

void TricoreFrameLowering::emitSPAdjustment(MachineFunction &MF,
                                            MachineBasicBlock &MBB,
                                            MachineBasicBlock::iterator MBBI,
                                            int NumBytes, unsigned ADDrr,
                                            unsigned ADDri) const {
  DebugLoc dl;
  const TricoreInstrInfo &TII =
      *static_cast<const TricoreInstrInfo *>(MF.getSubtarget().getInstrInfo());
  BuildMI(MBB, MBBI, dl, TII.get(Tricore::SUBA_SC), Tricore::A10)
      .addReg(Tricore::A10)
      .addImm(-NumBytes);
}

void TricoreFrameLowering::emitPrologue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  TricoreMachineFunctionInfo *FuncInfo =
      MF.getInfo<TricoreMachineFunctionInfo>();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TricoreSubtarget &Subtarget = MF.getSubtarget<TricoreSubtarget>();
  const TricoreInstrInfo &TII =
      *static_cast<const TricoreInstrInfo *>(Subtarget.getInstrInfo());
  const TricoreRegisterInfo &RegInfo =
      *static_cast<const TricoreRegisterInfo *>(Subtarget.getRegisterInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();

  // Get the number of bytes to allocate from the FrameInfo
  int NumBytes = (int)MFI.getStackSize();

  if (MFI.adjustsStack() && hasReservedCallFrame(MF))
    NumBytes += MFI.getMaxCallFrameSize();

  NumBytes = Subtarget.getAdjustedFrameSize(NumBytes);

  // Finally, ensure that the size is sufficiently aligned for the
  // data on the stack.
  NumBytes = alignTo(NumBytes, MFI.getMaxAlign());

  // Update stack size with corrected value.
  MFI.setStackSize(NumBytes);

  emitSPAdjustment(MF, MBB, MBBI, -NumBytes, 0, 0);
}

MachineBasicBlock::iterator TricoreFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {

  return MBB.erase(I);
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
  bool isFixed = MFI.isFixedObjectIndex(FI);

  // Addressable stack objects are accessed using neg. offsets from
  // %fp, or positive offsets from %sp.
  bool UseFP;

  // Tricore uses FP-based references in general, even when "hasFP" is
  // false. That function is rather a misnomer, because %fp is
  // actually always available, unless isLeafProc.
  if (FuncInfo->isLeafProc()) {
    // If there's a leaf proc, all offsets need to be %sp-based,
    // because we haven't caused %fp to actually point to our frame.
    UseFP = false;
  } else if (isFixed) {
    // Otherwise, argument access should always use %fp.
    UseFP = true;
  } else {
    // Finally, default to using %fp.
    UseFP = true;
  }

  int64_t FrameOffset =
      MF.getFrameInfo().getObjectOffset(FI) + Subtarget.getStackPointerBias();

  if (UseFP) {
    FrameReg = RegInfo->getFrameRegister(MF);
    return StackOffset::getFixed(FrameOffset);
  } else {
    FrameReg = SP::O6; // %sp
    return StackOffset::getFixed(FrameOffset +
                                 MF.getFrameInfo().getStackSize());
  }
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
