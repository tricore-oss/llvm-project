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

std::string tricore::getTricoreArch(const llvm::opt::ArgList &Args) {
  if (const Arg *A = Args.getLastArg(options::OPT_march_EQ)) {
    return A->getValue();
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

std::string tricore::getTricoreTargetCPU(const llvm::opt::ArgList &Args,
                                         const llvm::Triple &Triple) {
  return getTricoreArch(Args);
}

// Decode Tricore features from string like +[no]featureA+[no]featureB+...
static bool DecodeTricoreFeatures(const Driver &D, StringRef text,
                                  StringRef CPU,
                                  llvm::Tricore::ArchKind ArchKind,
                                  std::vector<StringRef> &Features,
                                  llvm::Tricore::FPUKind &ArgFPUKind) {
  std::size_t Start = text.size() > 0 ? 0 : std::string::npos;
  auto End = text.find_first_of("+-", Start);

  while (Start != std::string::npos) {
    auto Feature = text.substr(Start, End - Start);
    if (!appendArchExtFeatures(CPU, ArchKind, Feature, Features, ArgFPUKind))
      return false;
  }
  return true;
}

static void DecodeTricoreFeaturesFromCPU(const Driver &D, StringRef CPU,
                                         std::vector<StringRef> &Features) {
  CPU = CPU.split("+").first;
  if (CPU != "generic") {
    llvm::Tricore::ArchKind ArchKind = llvm::Tricore::parseCPUArch(CPU);
    uint64_t Extension = llvm::Tricore::getDefaultExtensions(CPU, ArchKind);
    llvm::Tricore::getExtensionFeatures(Extension, Features);
  }
}

static void checkTricoreArchName(const Driver &D, const Arg *A,
                                 const ArgList &Args, llvm::StringRef MArch,
                                 llvm::StringRef CPUName,
                                 std::vector<StringRef> &Features,
                                 const llvm::Triple &Triple,
                                 llvm::Tricore::FPUKind &ArgFPUKind) {

  auto ArchNameEnd = MArch.find('+');
  StringRef ArchName = MArch.substr(0, ArchNameEnd);
  StringRef ArchFeatures = MArch.substr(ArchNameEnd);

  llvm::Tricore::ArchKind ArchKind = llvm::Tricore::parseArch(ArchName);
  if (ArchKind == llvm::Tricore::ArchKind::INVALID ||
      (ArchFeatures.size() &&
       !DecodeTricoreFeatures(D, ArchFeatures, CPUName, ArchKind, Features,
                              ArgFPUKind)))
    D.Diag(clang::diag::err_drv_unsupported_option_argument)
        << A->getSpelling() << A->getValue();
}

// Check -mcpu=. Needs ArchName to handle -mcpu=generic.
static void checkTricoreCPUName(const Driver &D, const Arg *A,
                                const ArgList &Args, llvm::StringRef CPUName,
                                llvm::StringRef ArchName,
                                std::vector<StringRef> &Features,
                                const llvm::Triple &Triple,
                                llvm::Tricore::FPUKind &ArgFPUKind) {
  auto CPUEnd = CPUName.find_first_of("-+");
  StringRef CPU = CPUName.substr(0, CPUEnd);
  StringRef CPUFeatures = CPUName.substr(CPUEnd);
  llvm::Tricore::ArchKind ArchKind =
      llvm::Tricore::parseCPUArch(CPU);

  if (ArchKind == llvm::Tricore::ArchKind::INVALID ||
      (CPUFeatures.size() &&
       !DecodeTricoreFeatures(D, CPUFeatures, CPU, ArchKind, Features,
                              ArgFPUKind)))
    D.Diag(clang::diag::err_drv_unsupported_option_argument)
        << A->getSpelling() << A->getValue();
}

void tricore::getTricoreTargetFeatures(const Driver &D, const ArgList &Args,
                                       std::vector<StringRef> &Features) {
  tricore::FloatABI FloatABI = tricore::getTricoreFloatABI(D, Args);
  if (FloatABI == tricore::FloatABI::Soft)
    Features.push_back("+soft-float");

  const Arg *ArchArg = Args.getLastArg(options::OPT_march_EQ);
  const Arg *CPUArg = Args.getLastArg(options::OPT_mcpu_EQ);
  StringRef Arch = ArchArg ? ArchArg->getValue() : "";
  StringRef Cpu = CPUArg ? CPUArg->getValue() : "";
  llvm::Tricore::ArchKind ArchKind = getLLVMArchKindForTricore(Cpu, Arch);
}

llvm::Tricore::ArchKind tricore::getLLVMArchKindForTricore(
    StringRef CPU, StringRef Arch) {

  if (CPU.empty()) {
    if (Arch.empty())
      return llvm::Tricore::ArchKind::TC13;
    return llvm::Tricore::parseArch(Arch);
  }

  return llvm::Tricore::parseCPUArch(CPU);
}