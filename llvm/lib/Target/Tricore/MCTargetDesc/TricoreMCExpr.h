//====- TricoreMCExpr.h - Tricore specific MC expression classes --*- C++ -*-=====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes Tricore-specific MCExprs, used for modifiers like
// "%hi" or "%lo" etc.,
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCEXPR_H
#define LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCEXPR_H

#include "TricoreFixupKinds.h"
#include "llvm/MC/MCExpr.h"

namespace llvm {

class StringRef;
class TricoreMCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_Tricore_None,
    VK_Tricore_LO,
    VK_Tricore_HI,

    VK_Tricore_Invalid
  };

private:
  const VariantKind Kind;
  const MCExpr *Expr;

  explicit TricoreMCExpr(VariantKind Kind, const MCExpr *Expr)
      : Kind(Kind), Expr(Expr) {}

  int64_t evaluateAsInt64(int64_t Value) const;

public:
  /// @name Construction
  /// @{

  static const TricoreMCExpr *create(VariantKind Kind, const MCExpr *Expr,
                                 MCContext &Ctx);
  
  bool evaluateAsConstant(int64_t &Res) const;

  /// @}
  /// @name Accessors
  /// @{

  /// getOpcode - Get the kind of this expression.
  VariantKind getKind() const { return Kind; }

  /// getSubExpr - Get the child of this expression.
  const MCExpr *getSubExpr() const { return Expr; }

  /// getFixupKind - Get the fixup kind of this expression.
  Tricore::Fixups getFixupKind() const { return getFixupKind(Kind); }

  /// @}
  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAssembler *Asm,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  static VariantKind parseVariantKind(StringRef name);
  static bool printVariantKind(raw_ostream &OS, VariantKind Kind);
  static Tricore::Fixups getFixupKind(VariantKind Kind);
};

} // end namespace llvm.

#endif
