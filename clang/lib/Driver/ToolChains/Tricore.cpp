//===--- Tricore.cpp - Tricore ToolChain Implementations ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Tricore.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/InputInfo.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/TargetParser/SubtargetFeature.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

namespace {


const StringRef PossibleTricoreLibcLocations[] = {
    "/usr/Tricore",
    "/usr/lib/Tricore",
};

} // end anonymous namespace

/// Tricore Toolchain
TricoreToolChain::TricoreToolChain(const Driver &D, const llvm::Triple &Triple,
                           const ArgList &Args)
    : Generic_ELF(D, Triple, Args) {
  GCCInstallation.init(Triple, Args);

  // Only add default libraries if the user hasn't explicitly opted out.
  if (!Args.hasArg(options::OPT_nostdlib) &&
      !Args.hasArg(options::OPT_nodefaultlibs) && GCCInstallation.isValid()) {
    GCCInstallPath = GCCInstallation.getInstallPath();
    std::string GCCParentPath(GCCInstallation.getParentLibPath());
    getProgramPaths().push_back(GCCParentPath + "/../bin");
  }
}

void TricoreToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,
                                             ArgStringList &CC1Args) const {
  if (DriverArgs.hasArg(options::OPT_nostdinc) ||
      DriverArgs.hasArg(options::OPT_nostdlibinc))
    return;

  // Omit if there is no Tricore-libc installed.
  std::optional<std::string> TricoreLibcRoot = findTricoreLibcInstallation();
  if (!TricoreLibcRoot)
    return;

  // Add 'Tricore-libc/include' to clang system include paths if applicable.
  std::string TricoreInc = *TricoreLibcRoot + "/include";
  if (llvm::sys::fs::is_directory(TricoreInc))
    addSystemInclude(DriverArgs, CC1Args, TricoreInc);
}

void TricoreToolChain::addClangTargetOptions(
    const llvm::opt::ArgList &DriverArgs, llvm::opt::ArgStringList &CC1Args,
    Action::OffloadKind DeviceOffloadKind) const {

}

Tool *TricoreToolChain::buildLinker() const {
  return new tools::Tricore::Linker(getTriple(), *this);
}

std::string
TricoreToolChain::getCompilerRT(const llvm::opt::ArgList &Args, StringRef Component,
                            FileType Type = ToolChain::FT_Static) const {
  assert(Type == ToolChain::FT_Static && "Tricore only supports static libraries");
  // Since Tricore can never be a host environment, its compiler-rt library files
  // should always have ".a" suffix, even on windows.
  SmallString<32> File("/libclang_rt.");
  File += Component.str();
  File += ".a";
  // Return the default compiler-rt path appended with
  // "Tricore/libclang_rt.$COMPONENT.a".
  SmallString<256> Path(ToolChain::getCompilerRTPath());
  llvm::sys::path::append(Path, "Tricore");
  llvm::sys::path::append(Path, File.str());
  return std::string(Path);
}

void Tricore::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                               const InputInfo &Output,
                               const InputInfoList &Inputs, const ArgList &Args,
                               const char *LinkingOutput) const {
  const auto &TC = static_cast<const TricoreToolChain &>(getToolChain());
  const Driver &D = getToolChain().getDriver();
  bool LinkerIsLLD = false;                                

  // Compute the linker program path, and use GNU "tricore-ld" as default.
  const Arg *A = Args.getLastArg(options::OPT_fuse_ld_EQ);
  std::string Linker = A ? getToolChain().GetLinkerPath(&LinkerIsLLD)
                         : getToolChain().GetProgramPath(getShortName());

  ArgStringList CmdArgs;

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  // Enable garbage collection of unused sections.
  if (!Args.hasArg(options::OPT_r))
    CmdArgs.push_back("--gc-sections");

  // Add library search paths before we specify libraries.
  Args.AddAllArgs(CmdArgs, options::OPT_L);
  getToolChain().AddFilePathLibArgs(Args, CmdArgs);

  // Currently we only support libgcc and compiler-rt.
  auto RtLib = TC.GetRuntimeLibType(Args);
  assert(
      (RtLib == ToolChain::RLT_Libgcc || RtLib == ToolChain::RLT_CompilerRT) &&
      "unknown runtime library");

  // Only add default libraries if the user hasn't explicitly opted out.
  bool LinkStdlib = false;
  if (!Args.hasArg(options::OPT_nostdlib) && !Args.hasArg(options::OPT_r) &&
      !Args.hasArg(options::OPT_nodefaultlibs)) {
  }

  if (!Args.hasArg(options::OPT_r)) {

  }

  if (D.isUsingLTO()) {
    assert(!Inputs.empty() && "Must have at least one input.");
    // Find the first filename InputInfo object.
    auto Input = llvm::find_if(
        Inputs, [](const InputInfo &II) -> bool { return II.isFilename(); });
    if (Input == Inputs.end())
      // For a very rare case, all of the inputs to the linker are
      // InputArg. If that happens, just use the first InputInfo.
      Input = Inputs.begin();

    addLTOOptions(TC, Args, CmdArgs, Output, *Input,
                  D.getLTOMode() == LTOK_Thin);
  }

  // If the family name is known, we can link with the device-specific libgcc.
  // Without it, libgcc will simply not be linked. This matches Tricore-gcc
  // behavior.
  if (LinkStdlib) {
    //assert(!CPU.empty() && "CPU name must be known in order to link stdlibs");

    CmdArgs.push_back("--start-group");

    // Add the object file for the CRT.
    // std::string CrtFileName = std::string("-l:crt") + CPU + std::string(".o");
    // CmdArgs.push_back(Args.MakeArgString(CrtFileName));

    // Link to libgcc.
    if (RtLib == ToolChain::RLT_Libgcc)
      CmdArgs.push_back("-lgcc");

    // Link to generic libraries of Tricore-libc.
    CmdArgs.push_back("-lm");
    CmdArgs.push_back("-lc");

    // Add the relocatable inputs.
    AddLinkerInputs(getToolChain(), Inputs, Args, CmdArgs, JA);

    // We directly use libclang_rt.builtins.a as input file, instead of using
    // '-lclang_rt.builtins'.
    if (RtLib == ToolChain::RLT_CompilerRT) {
      std::string RtLib =
          getToolChain().getCompilerRT(Args, "builtins", ToolChain::FT_Static);
      if (llvm::sys::fs::exists(RtLib))
        CmdArgs.push_back(Args.MakeArgString(RtLib));
    }

    CmdArgs.push_back("--end-group");

    // Add Tricore-libc's linker script to lld by default, if it exists.
    if (!Args.hasArg(options::OPT_T) &&
        Linker.find("tricore-ld") == std::string::npos) {
      /* TODO: */
      std::string Path("");
      // Path += *FamilyName;
      Path += ".x";
      if (llvm::sys::fs::exists(Path))
        CmdArgs.push_back(Args.MakeArgString("-T" + Path));
    }
    // Otherwise add user specified linker script to either tricore-ld or lld.
    else
      Args.AddAllArgs(CmdArgs, options::OPT_T);

    if (Args.hasFlag(options::OPT_mrelax, options::OPT_mno_relax, false))
      CmdArgs.push_back("--relax");
  } else {
    AddLinkerInputs(getToolChain(), Inputs, Args, CmdArgs, JA);
  }

  /* TODO: add -mcpu option */
  // if (Linker.find("tricore-ld") != std::string::npos && FamilyName)
  //   CmdArgs.push_back(Args.MakeArgString(std::string("-m") + *FamilyName));

  C.addCommand(std::make_unique<Command>(
      JA, *this, ResponseFileSupport::AtFileCurCP(), Args.MakeArgString(Linker),
      CmdArgs, Inputs, Output));
}

std::optional<std::string> TricoreToolChain::findTricoreLibcInstallation() const {
  // Search Tricore-libc installation according to Tricore-gcc installation.
  std::string GCCParent(GCCInstallation.getParentLibPath());
  std::string Path(GCCParent + "/Tricore");
  if (llvm::sys::fs::is_directory(Path))
    return Path;
  Path = GCCParent + "/../Tricore";
  if (llvm::sys::fs::is_directory(Path))
    return Path;

  // Search Tricore-libc installation from possible locations, and return the first
  // one that exists, if there is no Tricore-gcc installed.
  for (StringRef PossiblePath : PossibleTricoreLibcLocations) {
    std::string Path = getDriver().SysRoot + PossiblePath.str();
    if (llvm::sys::fs::is_directory(Path))
      return Path;
  }

  return std::nullopt;
}
