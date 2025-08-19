//===-- TricoreMCCodeEmitter.cpp - Convert Tricore code to machine code
//-------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the TricoreMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TricoreFixupKinds.h"
#include "TricoreBaseInfo.h"
#include "TricoreFixupKinds.h"
#include "TricoreMCExpr.h"
#include "TricoreMCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");
STATISTIC(MCNumFixups, "Number of MC fixups created");

namespace {

class TricoreMCCodeEmitter : public MCCodeEmitter {
  MCContext &Ctx;
  MCInstrInfo const &MCII;

public:
  TricoreMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx)
      : Ctx(Ctx), MCII(MCII) {}
  TricoreMCCodeEmitter(const TricoreMCCodeEmitter &) = delete;
  TricoreMCCodeEmitter &operator=(const TricoreMCCodeEmitter &) = delete;
  ~TricoreMCCodeEmitter() override = default;

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  uint64_t getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  uint64_t getDispImmOpValue(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  uint64_t getImmOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;
  uint64_t getMemEncodingBO(const MCInst &MI, unsigned OpNo,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const;
};

} // end anonymous namespace

void TricoreMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                             SmallVectorImpl<char> &CB,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  unsigned Size = Desc.getSize();

  switch (MI.getOpcode()) {
  case Tricore::PseudoTAIL: {
    MCInst TAIL =
        MCInstBuilder(Tricore::J).addExpr(MI.getOperand(0).getExpr());
    uint32_t Bits = getBinaryCodeForInstr(TAIL, Fixups, STI);
    support::endian::write(CB, Bits, llvm::endianness::little);
    MCNumEmitted++;
    return;
  }
  default:
    break;
  }

  switch (Size) {
  default:
    llvm_unreachable("Unhandled encodeInstruction length!");
  case 2: {
    uint16_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write<uint16_t>(CB, Bits, llvm::endianness::little);
    break;
  }
  case 4: {
    uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
    support::endian::write(CB, Bits, llvm::endianness::little);
    break;
  }
  }
  ++MCNumEmitted; // Keep track of the # of mi's emitted.
}

uint64_t
TricoreMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {

  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return MO.getImm();

  llvm_unreachable("Unhandled expression!");
  return 0;
}

uint64_t
TricoreMCCodeEmitter::getMemEncodingBO(const MCInst &MI, unsigned OpNo,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  return 0;
}

uint64_t
TricoreMCCodeEmitter::getDispImmOpValue(const MCInst &MI, unsigned OpNo,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  if (MO.isImm()) {
    uint64_t Res = MO.getImm();
    assert((Res & 1) == 0 && "LSB is non-zero");
    return Res >> 1;
  }

  return getImmOpValue(MI, OpNo, Fixups, STI);
}

uint64_t TricoreMCCodeEmitter::getImmOpValue(const MCInst &MI, unsigned OpNo,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  // bool EnableRelax = STI.hasFeature(Tricore::FeatureRelax);
  const MCOperand &MO = MI.getOperand(OpNo);

  MCInstrDesc const &Desc = MCII.get(MI.getOpcode());
  unsigned MIFrm = TricoreII::getFormat(Desc.TSFlags);

  // If the destination is an immediate, there is nothing to do.
  if (MO.isImm())
    return MO.getImm();

  if (!MO.isExpr()) {
    MI.dump();
    MO.dump();
  }
  assert(MO.isExpr() && "getImmOpValue expects only expressions or immediates");
  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();
  Tricore::Fixups FixupKind = Tricore::fixup_tricore_invalid;

  if (Kind == MCExpr::Target) {
    const TricoreMCExpr *RVExpr = cast<TricoreMCExpr>(Expr);

    switch (RVExpr->getKind()) {
    case TricoreMCExpr::VK_Tricore_None:
    case TricoreMCExpr::VK_Tricore_Invalid:
      llvm_unreachable("Unhandled fixup kind!");
    case TricoreMCExpr::VK_Tricore_LO:
      FixupKind = Tricore::fixup_tricore_lo;
      break;
    case TricoreMCExpr::VK_Tricore_LO2:
      FixupKind = Tricore::fixup_tricore_lo2;
      break;
    case TricoreMCExpr::VK_Tricore_HI:
      FixupKind = Tricore::fixup_tricore_hi;
      break;
    case TricoreMCExpr::VK_Tricore_24REL:
      FixupKind = Tricore::fixup_tricore_24rel;
      break;
    case llvm::TricoreMCExpr::VK_Tricore_24ABS:
      FixupKind = Tricore::fixup_tricore_24abs;
      break;
    default:
      llvm_unreachable("Unhandled fixup kind!");
    }
  } else if ((Kind == MCExpr::SymbolRef &&
              cast<MCSymbolRefExpr>(Expr)->getKind() ==
                  MCSymbolRefExpr::VK_None)) {
    switch (MIFrm) {
    default:
      dbgs() << "Missing fixup: " << Desc.getOpcode();
      break;
    case TricoreII::InstFormatRLC:
      if (Desc.getOpcode() == Tricore::MOVHrlc ||
          Desc.getOpcode() == Tricore::MOVHArlc) {
        FixupKind = Tricore::fixup_tricore_hi;
      } else {
        FixupKind = Tricore::fixup_tricore_lo;
      }
      break;
    case TricoreII::InstFormatBOL:
      FixupKind = Tricore::fixup_tricore_lo2;
      break;
    case TricoreII::InstFormatB:
      if (TricoreII::isAbsolute(Desc.getFlags())) {
        FixupKind = Tricore::fixup_tricore_24abs;
      } else {
        FixupKind = Tricore::fixup_tricore_24rel;
      }
      break;
    case TricoreII::InstFormatBRC:
    case TricoreII::InstFormatBRN:
    case TricoreII::InstFormatBRR:
      FixupKind = Tricore::fixup_tricore_15rel;
      break;
    }
  }

  assert(FixupKind != Tricore::fixup_tricore_invalid &&
         "Unhandled expression!");

  Fixups.push_back(
      MCFixup::create(0, Expr, MCFixupKind(FixupKind), MI.getLoc()));
  ++MCNumFixups;

  // TODO: Relax

  return 0;
}

#include "TricoreGenMCCodeEmitter.inc"

MCCodeEmitter *llvm::createTricoreMCCodeEmitter(const MCInstrInfo &MCII,
                                                MCContext &Ctx) {
  return new TricoreMCCodeEmitter(MCII, Ctx);
}
