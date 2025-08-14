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
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class ELFTricoreAsmBackend : public TricoreAsmBackend {
  Triple::OSType OSType;

public:
  ELFTricoreAsmBackend(const MCSubtargetInfo &STI, Triple::OSType OSType)
      : TricoreAsmBackend(STI), OSType(OSType) {}

  static uint64_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                   MCContext &Ctx) {
    switch (Fixup.getTargetKind()) {
    default:
      dbgs() << "fixup: " << Fixup.getTargetKind();
      llvm_unreachable("Unknown fixup kind!");
    case llvm::FK_Data_1:
    case llvm::FK_Data_2:
    case llvm::FK_Data_4:
    case llvm::FK_Data_8:
      return Value;
    case Tricore::fixup_tricore_24rel:
      if (!isShiftedInt<24, 1>(Value))
        Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
      Value = (Value >> 1);
      return (Value >> 16) | ((Value & 0xFFFF) << 8);
    case Tricore::fixup_tricore_hi:
      if (!isInt<32>(Value))
        Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
      Value = ((Value + 0x8000) >> 16);
      break;
    case Tricore::fixup_tricore_lo:
      if (!isInt<32>(Value))
        Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
      Value = (Value - ((Value + 0x8000) & ~0xFFFFull));
      break;
    case Tricore::fixup_tricore_lo2:
      if (!isInt<32>(Value))
        Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
      Value = Value - ((Value + 0x8000) & ~0xFFFFull);
      Value = ((Value & 0x3C0) << 6) | ((Value & 0xFC00) >> 4) |
              (Value & 0x3F);
      break;
    case Tricore::fixup_tricore_15rel:
      if (!isShiftedInt<15, 1>(Value))
        Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
      return (Value >> 1);
    }
  }

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
      return; // Doesn't change encoding.
    // Apply any target-specific value adjustments.
    Value = adjustFixupValue(Fixup, Value, Ctx);

    // Shift the value into position.
    Value <<= Info.TargetOffset;

    unsigned Offset = Fixup.getOffset();
    unsigned NumBytes = alignTo(Info.TargetSize + Info.TargetOffset, 8) / 8;

    assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");

    // For each byte of the fragment that the fixup touches, mask in the
    // bits from the fixup value.
    for (unsigned i = 0; i != NumBytes; ++i) {
      Data[Offset + i] |= uint8_t((Value >> (i * 8)) & 0xff);
    }
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
      {"fixup_tricore_32rel", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_tricore_32abs", 0, 32, 0},
      {"fixup_tricore_24rel", 0, 24, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_tricore_24abs", 0, 24, 0},
      {"fixup_tricore_16sm", 16, 16, 0},
      {"fixup_tricore_hi", 12, 16, 0},
      {"fixup_tricore_lo", 12, 16, 0},
      {"fixup_tricore_lo2", 16, 16, 0},
      {"fixup_tricore_18abs", 12, 20, 0},
      {"fixup_tricore_10sm", 16, 10, 0},
      {"fixup_tricore_15rel", 16, 15, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_tricore_disp4", 12, 4, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_tricore_disp8", 8, 8, MCFixupKindInfo::FKF_IsPCRel},
      {"fixup_tricore_disp24", 8, 24, MCFixupKindInfo::FKF_IsPCRel},
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
