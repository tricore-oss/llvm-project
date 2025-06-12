//===-- TricoreAsmBackend.cpp - Tricore Asm Backend
//----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the TricoreAsmBackend class.
//
//===----------------------------------------------------------------------===//
//

#include "MCTargetDesc/TricoreAsmBackend.h"
#include "MCTargetDesc/TricoreFixupKinds.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "TricoreFixupKinds.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class ELFTricoreAsmBackend : public TricoreAsmBackend {
  Triple::OSType OSType;

public:
  ELFTricoreAsmBackend(const MCSubtargetInfo &STI, Triple::OSType OSType)
      : TricoreAsmBackend(STI), OSType(OSType) {}

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override {
    MCFixupKind Kind = Fixup.getKind();
    if (Kind >= FirstLiteralRelocationKind)
      return;
    MCContext &Ctx = Asm.getContext();
    MCFixupKindInfo Info = getFixupKindInfo(Kind);
    if (!Value)
      return;      // Doesn't change encoding.
    assert(false); // TODO: Implement this function
  }

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
    return createTricoreELFObjectWriter(OSABI);
  }
};

std::optional<MCFixupKind>
TricoreAsmBackend::getFixupKind(StringRef Name) const {
  if (STI.getTargetTriple().isOSBinFormatELF()) {
    unsigned Type;
    Type = llvm::StringSwitch<unsigned>(Name)
#define ELF_RELOC(NAME, ID) .Case(#NAME, ID)
#include "llvm/BinaryFormat/ELFRelocs/Tricore.def"
#undef ELF_RELOC
               .Default(-1u);
    if (Type != -1u)
      return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
  }
  return std::nullopt;
}

const MCFixupKindInfo &
TricoreAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[] = {
      // This table *must* be in the order that the fixup_* kinds are defined in
      // TricoreFixupKinds.h.
      //
      // name                      offset bits  flags
      {"fixup_tricore_lo", 0, 16, 0},
      {"fixup_tricore_hi", 16, 16, 0},
      {"fixup_tricore_rel24", 0, 25, MCFixupKindInfo::FKF_IsPCRel},
  };
  static_assert((std::size(Infos)) == Tricore::NumTargetFixupKinds,
                "Not all fixup kinds added to Infos array");

  // Fixup kinds from .reloc directive are like R_RISCV_NONE. They
  // do not require any extra processing.
  if (Kind >= FirstLiteralRelocationKind)
    return MCAsmBackend::getFixupKindInfo(FK_NONE);

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

bool TricoreAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                     const MCSubtargetInfo *STI) const {
  if (Count % 2) {
    OS.write("\0", 1);
    Count -= 1;
  }

  for (; Count >= 2; Count -= 2)
    OS.write("\0\0", 2);

  return true;
}

MCAsmBackend *llvm::createTricoreAsmBackend(const Target &T,
                                            const MCSubtargetInfo &STI,
                                            const MCRegisterInfo &MRI,
                                            const MCTargetOptions &Options) {
  return new ELFTricoreAsmBackend(STI, STI.getTargetTriple().getOS());
}
