#include "Tricore.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

using namespace clang;
using namespace clang::targets;

void TricoreTargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  Values.push_back("generic");
  Values.push_back("tc16");
  Values.push_back("tc18");
}

TricoreTargetInfo::CPUKind TricoreTargetInfo::getCPUKind(StringRef Name) const {
  if (Name == "tc16")
    return CK_TC16;
  if (Name == "tc18")
    return CK_TC18;
  return CK_GENERIC;
}

void TricoreTargetInfo::getTargetDefines(const LangOptions &Opts,
                                         MacroBuilder &Builder) const {}

static constexpr Builtin::Info BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, ALL_LANGUAGES},
#define LANGBUILTIN(ID, TYPE, ATTRS, LANG)                                     \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::NO_HEADER, LANG},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, nullptr, HeaderDesc::HEADER, ALL_LANGUAGES},
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE)                               \
  {#ID, TYPE, ATTRS, FEATURE, HeaderDesc::NO_HEADER, ALL_LANGUAGES},
#define TARGET_HEADER_BUILTIN(ID, TYPE, ATTRS, HEADER, LANGS, FEATURE)         \
  {#ID, TYPE, ATTRS, FEATURE, HeaderDesc::HEADER, LANGS},
#include "clang/Basic/BuiltinsTricore.inc"
};

ArrayRef<Builtin::Info> TricoreTargetInfo::getTargetBuiltins() const {
  return llvm::ArrayRef(BuiltinInfo, clang::Tricore::LastTSBuiltin -
                                         Builtin::FirstTSBuiltin);
}

TargetInfo::BuiltinVaListKind TricoreTargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::VoidPtrBuiltinVaList;
}

bool TricoreTargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &Info) const {
  switch (*Name) {
  default:
  case 'd':
  case 'a':
    Info.setAllowsRegister();
    return true;
  case 'D':
    Info.setAllowsRegister();
    return true;
  case 'A':
    Info.setAllowsRegister();
    return true;
    break;
  }
  return false;
}

std::string
TricoreTargetInfo::convertConstraint(const char *&Constraint) const {
  std::string R;
  switch (*Constraint) {
  case 'r':
    return std::string("d");
  default:
    return std::string(1, *Constraint);
  }
}

bool TricoreTargetInfo::validateConstraintModifier(
    StringRef Constraint, char Modifier, unsigned Size,
    std::string &SuggestedModifier) const {
  bool isOutput = (Constraint[0] == '=');
  bool isInOut = (Constraint[0] == '+');

  // Strip off constraint modifiers.
  Constraint = Constraint.ltrim("=+&");

  switch (Constraint[0]) {
  default:
    return false;
  case 'd':
  case 'D':
  case 'a':
  case 'A':
    return isOutput || isInOut || Size <= 32;
  }
}

std::string_view TricoreTargetInfo::getClobbers() const { return ""; }

ArrayRef<const char *> TricoreTargetInfo::getGCCRegNames() const {
  // clang-format off
  static const char *const GCCRegNames[] = {
    "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
    "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "a8", "a9", "a8", "a10", "a11", "a12", "a13", "a14", "a15",
    "e0", "e2", "e4", "e6", "e8", "e10", "e12", "e14", "psw",
    "pcxi", "pc", "fcx", "lcx", "isp", "isr", "icr", "pipn", "biv", "btv"
  };

  return GCCRegNames;
}

ArrayRef<TargetInfo::GCCRegAlias> TricoreTargetInfo::getGCCRegAliases() const {
  static const TargetInfo::GCCRegAlias GCCRegAliases[] = {
    {{"sp"}, "a10"},
    {{"ra"}, "a11"},
  };
  return GCCRegAliases;
}