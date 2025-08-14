//===-- TricoreELFObjectWriter.cpp - Tricore ELF Writer
//-----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TricoreFixupKinds.h"
#include "MCTargetDesc/TricoreMCExpr.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "TricoreFixupKinds.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class TricoreELFObjectWriter : public MCELFObjectTargetWriter {
public:
  TricoreELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(false, OSABI, ELF::EM_TRICORE,
                                /*HasRelocationAddend*/ true) {}

  ~TricoreELFObjectWriter() override = default;

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;

  bool needsRelocateWithSymbol(const MCValue &Val, const MCSymbol &Sym,
                               unsigned Type) const override;
};
} // namespace

unsigned TricoreELFObjectWriter::getRelocType(MCContext &Ctx,
                                              const MCValue &Target,
                                              const MCFixup &Fixup,
                                              bool IsPCRel) const {
  const MCExpr *Expr = Fixup.getValue();
  // Determine the type of the relocation
  unsigned Kind = Fixup.getTargetKind();
  if (Kind >= FirstLiteralRelocationKind)
    return Kind - FirstLiteralRelocationKind;

  if (IsPCRel) {
    switch (Fixup.getTargetKind()) {
    default:
      Ctx.reportError(Fixup.getLoc(), "unsupported relocation type");
      return ELF::R_RISCV_NONE;
    case FK_Data_4:
    case FK_PCRel_4:
      return ELF::R_TRICORE_32REL;
      break;
    case Tricore::fixup_tricore_24rel:
      return ELF::R_TRICORE_24REL;
    case Tricore::fixup_tricore_15rel:
      return ELF::R_TRICORE_15REL;
    }
  }
  switch (Fixup.getTargetKind()) {
  default:
    Ctx.reportError(Fixup.getLoc(), "unsupported relocation type");
    return ELF::R_RISCV_NONE;
  case FK_Data_1:
    Ctx.reportError(Fixup.getLoc(), "1-byte data relocations not supported");
    return ELF::R_TRICORE_NONE;
  case FK_Data_2:
    Ctx.reportError(Fixup.getLoc(), "2-byte data relocations not supported");
    return ELF::R_TRICORE_NONE;
  case FK_Data_4:
    return ELF::R_TRICORE_32ABS;
  case FK_Data_8:
    Ctx.reportError(Fixup.getLoc(), "8-byte data relocations not supported");
    return ELF::R_TRICORE_NONE;
  case Tricore::fixup_tricore_hi:
    return ELF::R_TRICORE_HI;
  case Tricore::fixup_tricore_lo:
    return ELF::R_TRICORE_LO;
  case Tricore::fixup_tricore_lo2:
    return ELF::R_TRICORE_LO2;
  }

  return 0;
}

bool TricoreELFObjectWriter::needsRelocateWithSymbol(const MCValue &,
                                                     const MCSymbol &,
                                                     unsigned Type) const {

  // TODO: Check for GOT relocations once implemented
  return false;
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createTricoreELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<TricoreELFObjectWriter>(OSABI);
}
