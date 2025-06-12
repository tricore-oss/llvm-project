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
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
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
      llvm_unreachable("Unimplemented fixup -> relocation");
    case Tricore::fixup_tricore_rel24:
      return ELF::R_TRICORE_24REL;
    }
  }
  return 0;
}

bool TricoreELFObjectWriter::needsRelocateWithSymbol(const MCValue &,
                                                     const MCSymbol &,
                                                     unsigned Type) const {}

std::unique_ptr<MCObjectTargetWriter>
llvm::createTricoreELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<TricoreELFObjectWriter>(OSABI);
}
