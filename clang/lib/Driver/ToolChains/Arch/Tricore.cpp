//===--- Tricore.cpp - Tools Implementations ----------------------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Tricore.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Option/ArgList.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/TricoreTargetParser.h"
#include <cstddef>
#include <vector>

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

tricore::FloatABI tricore::getTricoreFloatABI(const Driver &D,
                                              const ArgList &Args) {
  tricore::FloatABI ABI = tricore::FloatABI::Invalid;
  if (Arg *A = Args.getLastArg(options::OPT_msoft_float, options::OPT_mno_fpu,
                               options::OPT_mhard_float, options::OPT_mfpu,
                               options::OPT_mfloat_abi_EQ)) {
    if (A->getOption().matches(options::OPT_msoft_float) ||
        A->getOption().matches(options::OPT_mno_fpu))
      ABI = tricore::FloatABI::Soft;
    else if (A->getOption().matches(options::OPT_mhard_float) ||
             A->getOption().matches(options::OPT_mfpu))
      ABI = tricore::FloatABI::Hard;
    else {
      ABI = llvm::StringSwitch<tricore::FloatABI>(A->getValue())
                .Case("soft", tricore::FloatABI::Soft)
                .Case("hard", tricore::FloatABI::Hard)
                .Default(tricore::FloatABI::Invalid);
      if (ABI == tricore::FloatABI::Invalid &&
          !StringRef(A->getValue()).empty()) {
        D.Diag(clang::diag::err_drv_invalid_mfloat_abi) << A->getAsString(Args);
        ABI = tricore::FloatABI::Hard;
      }
    }
  }

  // Default is always hard-float ABI.
  if (ABI == tricore::FloatABI::Invalid) {
    ABI = tricore::FloatABI::Hard;
  }

  return ABI;
}

std::string tricore::getTricoreTargetCPU(const llvm::opt::ArgList &Args) {
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ)) {
    StringRef Arch = A->getValue();
    llvm::Tricore::ArchKind Kind = llvm::Tricore::parseArch(Arch);

    if (Kind != llvm::Tricore::ArchKind::INVALID) {
      return llvm::Tricore::getArchName(Kind).str();
    }
  }

  if (const Arg *A = Args.getLastArg(options::OPT_mcpu_EQ)) {
    StringRef CPU = A->getValue();
    llvm::Tricore::ArchKind Kind = llvm::Tricore::parseCPUArch(CPU);

    if (Kind != llvm::Tricore::ArchKind::INVALID) {
      return llvm::Tricore::getArchName(Kind).str();
    }
  }

  return "tc13";
}

static bool DecodeTricoreFeatures(StringRef text,
                                  std::vector<StringRef> &Features) {
  std::size_t Start = text.find_first_of("+-");

  while (Start != std::string::npos) {
    auto End = text.find_first_of("+-", Start + 1);
    auto Feature = text.substr(Start, End - Start);
    if (!llvm::Tricore::appendArchExtFeatures(Feature, Features))
      return false;
    Start = End;
  }
  return true;
}

void tricore::getTricoreTargetFeatures(const Driver &D, const ArgList &Args,
                                       std::vector<StringRef> &Features) {
  llvm::StringRef ArchName;
  llvm::StringRef CpuName;
  const Arg *ArchArg = Args.getLastArg(options::OPT_march_EQ);
  const Arg *CpuArg = Args.getLastArg(clang::driver::options::OPT_mcpu_EQ);
  llvm::Tricore::ArchKind ArchKind = llvm::Tricore::ArchKind::INVALID;

  if (ArchArg) {
    ArchKind = llvm::Tricore::parseArch(ArchArg->getValue());
    if (ArchKind == llvm::Tricore::ArchKind::INVALID) {
      D.Diag(clang::diag::err_drv_invalid_arch_name)
          << ArchArg->getAsString(Args);
      return;
    }
    ArchName = ArchArg->getValue();
  }

  if (CpuArg) {
    llvm::Tricore::ArchKind Kind =
        llvm::Tricore::parseCPUArch(CpuArg->getValue());
    if (Kind == llvm::Tricore::ArchKind::INVALID) {
      D.Diag(clang::diag::err_drv_unsupported_option_argument)
          << CpuArg->getSpelling() << CpuArg->getAsString(Args);
      return;
    }
    if (!ArchName.empty() && Kind != ArchKind) {
      D.Diag(clang::diag::err_drv_unsupported_option_argument)
          << CpuArg->getSpelling() << CpuArg->getAsString(Args);
      return;
    }
    CpuName = CpuArg->getValue();
    if (ArchName.empty())
      ArchName = llvm::Tricore::getArchName(Kind);
  }

  uint64_t Extension = llvm::Tricore::getDefaultExtensions(CpuName, ArchKind);
  llvm::Tricore::getExtensionFeatures(Extension, Features);
  llvm::Tricore::FPUKind FPU = llvm::Tricore::getDefaultFPU(CpuName, ArchKind);
  llvm::Tricore::getFPUFeatures(FPU, Features);

  if (ArchArg && !DecodeTricoreFeatures(ArchName, Features)) {
    D.Diag(clang::diag::err_drv_unsupported_option_argument)
        << ArchArg->getSpelling() << ArchArg->getAsString(Args);
  }

  if (CpuArg && !DecodeTricoreFeatures(CpuName, Features)) {
    D.Diag(clang::diag::err_drv_unsupported_option_argument)
        << ArchArg->getSpelling() << ArchArg->getAsString(Args);
  }
}