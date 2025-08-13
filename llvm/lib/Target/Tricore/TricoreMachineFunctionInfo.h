//===- TricoreMachineFunctionInfo.h - Tricore Machine Function Info -*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares  Tricore specific per-machine-function information.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_TRICORE_TRICOREMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_TRICORE_TRICOREMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class TricoreMachineFunctionInfo : public MachineFunctionInfo {
  virtual void anchor();

private:
  /// Indicates if arguments passed using the stack are being
  /// used inside the function.
  bool HasStackArgs;

  /// Whether or not the function is an interrupt handler.
  bool IsInterruptHandler;

  /// FrameIndex for start of varargs area.
  int VarArgsFrameIndex;

public:
  TricoreMachineFunctionInfo() {}
  TricoreMachineFunctionInfo(const Function &F,
                             const TargetSubtargetInfo *STI) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  bool getHasStackArgs() const { return HasStackArgs; }
  void setHasStackArgs(bool B) { HasStackArgs = B; }

  bool isInterruptHandler() const { return IsInterruptHandler; }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Idx) { VarArgsFrameIndex = Idx; }
};
} // namespace llvm

#endif
