//===--- Tricore.h - declare sparc target feature support ---------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares Tricore TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_TRICORE_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_TRICORE_H
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/TricoreTargetParser.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY TricoreTargetInfo : public TargetInfo {
  static const TargetInfo::GCCRegAlias GCCRegAliases[];
  static const char *const GCCRegNames[];
  std::string CPU;

public:
  TricoreTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    HasStrictFP = true;

    SizeType = UnsignedLong;
    IntPtrType = SignedLong;
    PtrDiffType = SignedLong;
    LongDoubleWidth = 64;
    LongDoubleAlign = 64;
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    IntMaxType = SignedLongLong;
    Int64Type = SignedLongLong;
    resetDataLayout("e-p:32:32-S64-i64:32:64-f64:32:64-a:0:32-n32");

    MaxAtomicPromoteWidth = 64;
    MaxAtomicInlineWidth = 32;
  }

  bool isValidCPUName(StringRef Name) const override {
    return llvm::Tricore::parseCPUArch(Name) != llvm::Tricore::ArchKind::INVALID;
  }

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;
  void fillValidTuneCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    if (!isValidCPUName(Name))
      return false;
    CPU = Name;
    return true;
  }

  std::pair<unsigned, unsigned> hardwareInterferenceSizes() const override {
    return std::make_pair(32, 32);
  }

  bool hasBitIntType() const override { return true; }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
  ArrayRef<Builtin::Info> getTargetBuiltins() const override;
  BuiltinVaListKind getBuiltinVaListKind() const override;

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override;
  bool validateConstraintModifier(
      StringRef /*Constraint*/, char /*Modifier*/, unsigned /*Size*/,
      std::string & /*SuggestedModifier*/) const override;
  std::string convertConstraint(const char *&Constraint) const override;
  std::string_view getClobbers() const override;

  ArrayRef<const char *> getGCCRegNames() const override;
  ArrayRef<GCCRegAlias> getGCCRegAliases() const override;
};
} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_TRICORE_H
