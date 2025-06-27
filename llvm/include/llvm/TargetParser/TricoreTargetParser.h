//===-- TricoreTargetParser - Parser for target features ----------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a target parser to recognise hardware features
// for RISC-V CPUs.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGETPARSER_TRICORETARGETPARSER_H
#define LLVM_TARGETPARSER_TRICORETARGETPARSER_H

#include "llvm/ADT/StringRef.h"
#include <vector>

namespace llvm {

class Triple;

namespace Tricore {

// Note that this is not the same as the AArch64 list
enum ArchExtKind : uint64_t {
  AEK_INVALID = 0,
  AEK_NONE = 1,
  AEK_FPU_SP = 1 << 1,
  AEK_FPU_DP = 1 << 2,
  AEK_CRC = 1 << 3,
  AEK_VIRT = 1 << 4,
};

// Arch names.
enum class ArchKind {
#define TRICORE_ARCH(NAME, ID, ARCH_FPU, ARCH_BASE_EXT) ID,
#include "llvm/TargetParser/TricoreTargetParser.def"
};

enum FPUKind {
  FK_INVALID,
  FK_NONE,
  FK_SP,
  FK_DP,
};

// Parse
ArchKind parseCPUArch(StringRef CPU);
ArchKind parseArch(StringRef Arch);

// FPU
StringRef getFPUName(FPUKind FPUKind);

// Arch
StringRef getArchName(ArchKind ArchKind);
uint64_t getArchAttr(ArchKind AK);

// Defaults
FPUKind getDefaultFPU(StringRef CPU, ArchKind AK);
uint64_t getDefaultExtensions(StringRef CPU, ArchKind AK);

// Features
bool getFPUFeatures(FPUKind FPUKind, std::vector<StringRef> &Features);
bool getExtensionFeatures(uint64_t Extensions,
                          std::vector<StringRef> &Features);
bool appendArchExtFeatures(StringRef CPU, ArchKind AK, StringRef ArchExt,
                           std::vector<StringRef> &Features,
                           FPUKind &ArgFPUKind);

void fillValidCPUArchList(SmallVectorImpl<StringRef> &Values);
void fillValidTuneCPUArchList(SmallVectorImpl<StringRef> &Values);

} // namespace Tricore
} // namespace llvm

#endif
