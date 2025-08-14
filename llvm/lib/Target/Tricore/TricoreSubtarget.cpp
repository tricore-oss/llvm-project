//===-- TricoreSubtarget.cpp - TRICORE Subtarget Information
//------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the TRICORE specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "TricoreSubtarget.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

#define DEBUG_TYPE "sparc-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "TricoreGenSubtargetInfo.inc"

void TricoreSubtarget::anchor() {}

TricoreSubtarget &TricoreSubtarget::initializeSubtargetDependencies(
    StringRef CPU, StringRef TuneCPU, StringRef FS) {
  // Determine default and user specified characteristics
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "tc13";

  if (TuneCPU.empty())
    TuneCPU = CPUName;

  // Parse features string.
  ParseSubtargetFeatures(CPUName, TuneCPU, FS);

  return *this;
}

TricoreSubtarget::TricoreSubtarget(const StringRef &CPU,
                                   const StringRef &TuneCPU,
                                   const StringRef &FS, const TargetMachine &TM)
    : TricoreGenSubtargetInfo(TM.getTargetTriple(), CPU, TuneCPU, FS),
      ReserveRegister(TM.getMCRegisterInfo()->getNumRegs()),
      TargetTriple(TM.getTargetTriple()),
      InstrInfo(initializeSubtargetDependencies(CPU, TuneCPU, FS)),
      TLInfo(TM, *this), FrameLowering(*this) {}

bool TricoreSubtarget::enableMachineScheduler() const { return true; }
