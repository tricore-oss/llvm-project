//===-- TricoreTargetInfo.cpp - Tricore Target Implementation -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/TricoreTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheTricoreTarget() {
  static Target TheTricoreTarget;
  return TheTricoreTarget;
}


extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTricoreTargetInfo() {
  RegisterTarget<Triple::tricore,
                 /*HasJIT=*/false>
      X(getTheTricoreTarget(), "tricore", "Tricore", "Tricore");
}
