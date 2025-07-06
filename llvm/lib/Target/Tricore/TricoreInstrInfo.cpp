//===- TricoreInstrInfo.cpp - Tricore Instruction Information
//-------------------===//
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
#include "TricoreRegisterInfo.h"
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
#include <iterator>

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "TricoreGenInstrInfo.inc"

// Pin the vtable to this file.
void TricoreInstrInfo::anchor() {}

TricoreInstrInfo::TricoreInstrInfo(TricoreSubtarget &ST)
    : TricoreGenInstrInfo(Tricore::ADJCALLSTACKDOWN, Tricore::ADJCALLSTACKUP),
      RI(), Subtarget(ST) {}

void TricoreInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MI,
                                   const DebugLoc &DL, MCRegister DestReg,
                                   MCRegister SrcReg, bool KillSrc,
                                   bool RenamableDest,
                                   bool RenamableSrc) const {

  if (Tricore::DGPRRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(Tricore::MOV_RR), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  }
  if (Tricore::DGPRRegClass.contains(DestReg) &&
      Tricore::AGPRRegClass.contains(SrcReg)) {
    BuildMI(MBB, MI, DL, get(Tricore::MOVD_RR), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  }
  if (Tricore::AGPRRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(Tricore::MOVAA_RR), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  }
  if (Tricore::AGPRRegClass.contains(DestReg) &&
      Tricore::DGPRRegClass.contains(SrcReg)) {
    BuildMI(MBB, MI, DL, get(Tricore::MOVA_RR), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  }
}

bool TricoreInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                     MachineBasicBlock *&TBB,
                                     MachineBasicBlock *&FBB,
                                     SmallVectorImpl<MachineOperand> &Cond,
                                     bool AllowModify) const {

  // Fall through if no terminator is at the end of the block
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  MachineBasicBlock::iterator Barrier = MBB.end();
  MachineBasicBlock::iterator CondBranch = MBB.end();

  if (I == MBB.end() || !isUnpredicatedTerminator(*I))
    return false;

  if (I->getDesc().isIndirectBranch()) {
    return true;
  }

  if (I->getDesc().isUnconditionalBranch()) {
    Barrier = I;

    if (Barrier != MBB.begin() &&
        std::prev(Barrier)->getDesc().isConditionalBranch()) {
      CondBranch = std::prev(Barrier);
    }
  }

  if (I->getDesc().isConditionalBranch()) {
    CondBranch = I;
    if (CondBranch != MBB.begin() &&
        std::prev(CondBranch)->getDesc().isConditionalBranch()) {
      return false;
    }
  }

  // No terminator found, so we can't handle this.
  if (Barrier == MBB.end() && CondBranch == MBB.end())
    return true;

  // Handle single unconditional branch
  if (CondBranch == MBB.end()) {
    TBB = Barrier->getOperand(0).getMBB();
    return false;
  }

  // Handle single conditional branch
  if (Barrier == MBB.end()) {
    Cond.push_back(MachineOperand::CreateImm(CondBranch->getOpcode()));
    Cond.push_back(CondBranch->getOperand(0));
    Cond.push_back(CondBranch->getOperand(1));
    TBB = CondBranch->getOperand(2).getMBB();
    return false;
  }

  // Handle conditional branch followed by unconditional branch
  TBB = CondBranch->getOperand(CondBranch->getNumOperands() - 1).getMBB();
  FBB = Barrier->getOperand(Barrier->getNumOperands() - 1).getMBB();
  Cond.push_back(MachineOperand::CreateImm(CondBranch->getOpcode()));
  Cond.push_back(CondBranch->getOperand(0));
  Cond.push_back(CondBranch->getOperand(1));
  return false;
}

unsigned TricoreInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                        int *BytesRemoved) const {
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;
  int Removed = 0;
  while (I != MBB.begin()) {
    --I;

    if (I->isDebugInstr())
      continue;

    if (!I->getDesc().isUnconditionalBranch() &&
        !I->getDesc().isConditionalBranch())
      break; // Not a branch

    Removed += getInstSizeInBytes(*I);
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  if (BytesRemoved)
    *BytesRemoved = Removed;
  return Count;
}

unsigned TricoreInstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
  if (BytesAdded)
    *BytesAdded = 0;

  // Shouldn't be a fall through.
  assert(TBB && "insertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 3 || Cond.size() == 0) &&
         "RISC-V branch conditions have two components!");

  // Unconditional branch.
  if (Cond.empty()) {
    MachineInstr &MI = *BuildMI(&MBB, DL, get(Tricore::J_B)).addMBB(TBB);
    if (BytesAdded)
      *BytesAdded += getInstSizeInBytes(MI);
    return 1;
  }

  const auto *Op = Cond.begin();
  auto Builder = BuildMI(&MBB, DL, get((Op++)->getImm()));
  for (; Op != Cond.end(); Op++) {
    Builder->addOperand(*Op);
  }
  Builder.addMBB(TBB);
  auto &MI = *Builder;
  if (BytesAdded)
    *BytesAdded += getInstSizeInBytes(MI);

  if (FBB) {
    MachineInstr &MI = *BuildMI(&MBB, DL, get(Tricore::J_B)).addMBB(FBB);
    if (BytesAdded)
      *BytesAdded += getInstSizeInBytes(MI);
    return 2;
  }

  return 1;
}

bool TricoreInstrInfo::foldImmediate(MachineInstr &UseMI, MachineInstr &DefMI,
                                     Register Reg,
                                     MachineRegisterInfo *MRI) const {
  uint32_t imm;

  switch (DefMI.getOpcode()) {
  default:
    return false;
  case Tricore::MOV_RLC:
    imm = DefMI.getOperand(1).getImm();
  }



  bool DeleteDef = !MRI->hasOneNonDBGUse(Reg);
  if (DeleteDef)
    DefMI.eraseFromParent();

  return true;
}