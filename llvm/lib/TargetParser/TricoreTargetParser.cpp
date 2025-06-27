//===-- RISCVTargetParser.cpp - Parser for target features ------*- C++ -*-===//
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

#include "llvm/TargetParser/TricoreTargetParser.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/TargetParser/RISCVISAInfo.h"

namespace llvm {
namespace Tricore {

// List of Arch Extension names.
struct ExtName {
  StringRef Name;
  uint64_t ID;
  StringRef Feature;
  StringRef NegFeature;
};

const ExtName ARCHExtNames[] = {
#define TRICORE_ARCH_EXT_NAME(NAME, ID, FEATURE, NEGFEATURE)                   \
  {NAME, Tricore::AEK_##ID, FEATURE, NEGFEATURE},
#include "llvm/TargetParser/TricoreTargetParser.def"
};

struct ArchNames {
  StringRef Name;
  FPUKind DefaultFPU;
  uint64_t ArchBaseExtensions;
  ArchKind ID;
};

static const ArchNames TricoreArchNames[] = {
#define TRICORE_ARCH(NAME, ID, ARCH_FPU, ARCH_BASE_EXT)                        \
  {NAME, ARCH_FPU, ARCH_BASE_EXT, ArchKind::ID},
#include "llvm/TargetParser/TricoreTargetParser.def"
};

// List of CPU names and their arches.
// The same CPU can have multiple arches and can be default on multiple arches.
// When finding the Arch for a CPU, first-found prevails. Sort them accordingly.
// When this becomes table-generated, we'd probably need two tables.
struct CpuNames {
  StringRef Name;
  ArchKind ArchID;
  uint64_t DefaultExtensions;
};

const CpuNames CPUNames[] = {
#define TRICORE_CPU_NAME(NAME, ID, CPU_FPU, CPU_EXT)                           \
  {NAME, Tricore::ArchKind::ID, CPU_EXT},
#include "llvm/TargetParser/TricoreTargetParser.def"
};

ArchKind parseCPUArch(StringRef CPU) {
  for (auto &C : CPUNames) {
    if (CPU.starts_with(C.Name)) {
      return C.ArchID;
    }
  }
  return ArchKind::INVALID;
}

ArchKind parseArch(StringRef Arch) {
  for (const auto &A : TricoreArchNames) {
    if (Arch.starts_with(A.Name)) {
      return A.ID;
    }
  }
  return ArchKind::INVALID;
}

StringRef getFPUName(FPUKind FPUKind) {
  switch (FPUKind) {
  case FPUKind::FK_NONE:
    return "none";
  case FPUKind::FK_SP:
    return "sp";
  case FPUKind::FK_DP:
    return "dp";
  case FPUKind::FK_INVALID:
    return "invalid";
  }
}

StringRef getArchName(ArchKind AK) {
  return TricoreArchNames[static_cast<unsigned>(AK)].Name;
}
uint64_t getArchAttr(ArchKind AK) {
  return TricoreArchNames[static_cast<unsigned>(AK)].ArchBaseExtensions;
}

FPUKind getDefaultFPU(StringRef CPU, ArchKind AK) {
  if (CPU == "generic")
    return TricoreArchNames[static_cast<unsigned>(AK)].DefaultFPU;

  return StringSwitch<FPUKind>(CPU)
#define TRICORE_CPU_NAME(NAME, ID, DEFAULT_FPU, DEFAULT_EXT)                   \
  .Case(NAME, DEFAULT_FPU)
#include "llvm/TargetParser/TricoreTargetParser.def"
      .Default(FK_INVALID);
}

uint64_t getDefaultExtensions(StringRef CPU, ArchKind AK) {
  if (CPU == "generic")
    return TricoreArchNames[static_cast<unsigned>(AK)].ArchBaseExtensions;

  return StringSwitch<uint64_t>(CPU)
#define TRICORE_CPU_NAME(NAME, ID, DEFAULT_FPU, DEFAULT_EXT)                   \
  .Case(NAME, TricoreArchNames[static_cast<unsigned>(ArchKind::ID)]            \
                      .ArchBaseExtensions |                                    \
                  DEFAULT_EXT)
#include "llvm/TargetParser/TricoreTargetParser.def"
      .Default(AEK_INVALID);
}

bool getFPUFeatures(FPUKind FPUKind, std::vector<StringRef> &Features) {
  switch (FPUKind) {
  case FPUKind::FK_INVALID:
    return false;
  case FPUKind::FK_NONE:
    break;
  case FPUKind::FK_SP:
    Features.push_back("+single-float");
    break;
  case FPUKind::FK_DP:
    Features.push_back("+double-float");
    break;
  }
  return true;
}

bool getExtensionFeatures(uint64_t Extensions,
                          std::vector<StringRef> &Features) {

  if (Extensions == AEK_INVALID)
    return false;

  for (const auto &AE : ARCHExtNames) {
    if ((Extensions & AE.ID) == AE.ID && !AE.Feature.empty())
      Features.push_back(AE.Feature);
    else if (!AE.NegFeature.empty())
      Features.push_back(AE.NegFeature);
  }

  return true;
}

bool appendArchExtFeatures(StringRef CPU, ArchKind AK, StringRef ArchExt,
                           std::vector<StringRef> &Features,
                           FPUKind &ArgFPUKind) {
  uint64_t ID = AEK_INVALID;

  for (const auto &AEK : ARCHExtNames) {
    if (AEK.Feature == ArchExt) {
      Features.push_back(AEK.Feature);
      ID = AEK.ID;
    }
    if (AEK.NegFeature == ArchExt) {
      Features.push_back(AEK.NegFeature);
      ID = AEK.ID;
    }
  }

  return ID != AEK_INVALID;
}

void fillValidCPUArchList(SmallVectorImpl<StringRef> &Values) {
  for (const auto &C : CPUNames) {
    Values.emplace_back(C.Name);
  }
}

void fillValidTuneCPUArchList(SmallVectorImpl<StringRef> &Values) {}

} // namespace Tricore
} // namespace llvm
