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
    const char *&Name, TargetInfo::ConstraintInfo &info) const {
  return false;
}

std::string_view TricoreTargetInfo::getClobbers() const { return ""; }

ArrayRef<const char *> TricoreTargetInfo::getGCCRegNames() const { return {}; }
ArrayRef<TargetInfo::GCCRegAlias> TricoreTargetInfo::getGCCRegAliases() const {
  return {};
}