//===- TricoreTargetMachine.h - Define TargetMachine for Tricore ------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Tricore specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRICORE_TRICORETARGETMACHINE_H
#define LLVM_LIB_TARGET_TRICORE_TRICORETARGETMACHINE_H

#include "TricoreSubtarget.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Support/CodeGen.h"
#include <memory>
#include <optional>

namespace llvm {

class TricoreTargetMachine : public CodeGenTargetMachineImpl {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  const TricoreSubtarget *Subtarget;
  mutable StringMap<std::unique_ptr<TricoreSubtarget>> SubtargetMap;

public:
  TricoreTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       std::optional<Reloc::Model> RM,
                       std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                       bool JIT);
  ~TricoreTargetMachine() override;

  const TricoreSubtarget *getSubtargetImpl(const Function &F) const override;

  void resetSubtarget(MachineFunction *MF);

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }

  MachineFunctionInfo *
  createMachineFunctionInfo(BumpPtrAllocator &Allocator, const Function &F,
                            const TargetSubtargetInfo *STI) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_TRICORE_TRICORETARGETMACHINE_H
