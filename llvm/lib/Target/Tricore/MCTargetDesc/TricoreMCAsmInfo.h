//===- TricoreMCAsmInfo.h - Tricore asm properties -----------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the TricoreMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCASMINFO_H
#define LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {

class Triple;

class TricoreELFMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit TricoreELFMCAsmInfo(const Triple &TheTriple);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCASMINFO_H
