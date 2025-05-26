//===-- Tricore.h - Top-level interface for Tricore representation --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// Tricore back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRICORE_TRICORE_H
#define LLVM_LIB_TARGET_TRICORE_TRICORE_H

#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
    class TricoreTargetMachine;
    class FunctionPass;
    class PassRegistry;

    FunctionPass *createTricoreISelDag(TricoreTargetMachine &TM);

    void initializeTricoreDAGToDAGISelLegacyPass(PassRegistry &);
}

#endif // LLVM_LIB_TARGET_TRICORE_TRICORE_H
