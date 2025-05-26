//===-- TricoreAsmBackend.h - Tricore Asm Backend
//------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the TricoreAsmBackend class.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_LIB_TARGET_TRICORE_MCTARGETDESC_TRICOREASMBACKEND_H
#define LLVM_LIB_TARGET_TRICORE_MCTARGETDESC_TRICOREASMBACKEND_H

#include "MCTargetDesc/TricoreFixupKinds.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/TargetParser/Triple.h"

namespace llvm {

class MCAssembler;
struct MCFixupKindInfo;
class MCRegisterInfo;
class Target;

class TricoreAsmBackend : public MCAsmBackend {
  const MCSubtargetInfo &STI;

public:
  TricoreAsmBackend(const MCSubtargetInfo &STI)
      : MCAsmBackend(llvm::endianness::little), STI(STI) {}

  std::optional<MCFixupKind> getFixupKind(StringRef Name) const override;
  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;

  unsigned getNumFixupKinds() const override {
    return Tricore::NumTargetFixupKinds;
  }

  void relaxInstruction(MCInst &Inst,
                        const MCSubtargetInfo &STI) const override {
    // FIXME.
    llvm_unreachable("relaxInstruction() unimplemented");
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

}; // class TricoreAsmBackend

} // namespace llvm

#endif
