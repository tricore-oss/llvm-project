//===-- TricoreMCExpr.cpp - Tricore specific MC expression classes --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the assembly expression modifiers
// accepted by the Tricore architecture (e.g. "%hi", "%lo", ...).
//
//===----------------------------------------------------------------------===//

#include "TricoreMCExpr.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

#define DEBUG_TYPE "sparcmcexpr"

const TricoreMCExpr*
TricoreMCExpr::create(VariantKind Kind, const MCExpr *Expr,
                      MCContext &Ctx) {
    return new (Ctx) TricoreMCExpr(Kind, Expr);
}

void TricoreMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {

  bool closeParen = printVariantKind(OS, Kind);

  const MCExpr *Expr = getSubExpr();
  Expr->print(OS, MAI);

  if (closeParen)
    OS << ')';
}

bool TricoreMCExpr::printVariantKind(raw_ostream &OS, VariantKind Kind)
{
  switch (Kind) {
  case VK_Tricore_None:     return false;
  case VK_Tricore_LO:       OS << "%lo(";  return true;
  case VK_Tricore_HI:       OS << "%hi(";  return true;
  }
  llvm_unreachable("Unhandled TricoreMCExpr::VariantKind");
}

TricoreMCExpr::VariantKind TricoreMCExpr::parseVariantKind(StringRef name)
{
  return StringSwitch<TricoreMCExpr::VariantKind>(name)
      .Case("lo", VK_Tricore_LO)
      .Case("hi", VK_Tricore_HI)
      .Default(VK_Tricore_None);
}

Tricore::Fixups TricoreMCExpr::getFixupKind(TricoreMCExpr::VariantKind Kind) {
  switch (Kind) {
  default: llvm_unreachable("Unhandled TricoreMCExpr::VariantKind");
  case VK_Tricore_LO:       return Tricore::fixup_tricore_lo;
  case VK_Tricore_HI:       return Tricore::fixup_tricore_hi;
  }
}

bool TricoreMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                            const MCAssembler *Asm,
                                            const MCFixup *Fixup) const {
  return getSubExpr()->evaluateAsRelocatable(Res, Asm, Fixup);
}

void TricoreMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

bool TricoreMCExpr::evaluateAsConstant(int64_t &Res) const {
  MCValue Value;

  if (Kind == VK_Tricore_HI || Kind == VK_Tricore_LO)
    return false;

  if (!getSubExpr()->evaluateAsRelocatable(Value, nullptr, nullptr))
    return false;

  if (!Value.isAbsolute())
    return false;

  Res = evaluateAsInt64(Value.getConstant());
  return true;
}

int64_t TricoreMCExpr::evaluateAsInt64(int64_t Value) const {
  switch (Kind) {
  default:
    llvm_unreachable("Invalid kind");
  case VK_Tricore_HI:
    return SignExtend64<16>(Value);
  case VK_Tricore_LO:
  
    return (Value >> 16) & 0xffff;
  }
}
