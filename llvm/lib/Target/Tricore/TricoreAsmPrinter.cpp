//===-- TricoreAsmPrinter.cpp - Tricore LLVM assembly writer
//------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format Tricore assembly language.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TricoreBaseInfo.h"
#include "MCTargetDesc/TricoreInstPrinter.h"
#include "MCTargetDesc/TricoreMCExpr.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "MCTargetDesc/TricoreTargetStreamer.h"
#include "TargetInfo/TricoreTargetInfo.h"
#include "Tricore.h"
#include "TricoreInstrInfo.h"
#include "TricoreTargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "tricore-asm-printer"

namespace {
class TricoreAsmPrinter : public AsmPrinter {
  TricoreTargetStreamer &getTargetStreamer() {
    return static_cast<TricoreTargetStreamer &>(
        *OutStreamer->getTargetStreamer());
  }

public:
  explicit TricoreAsmPrinter(TargetMachine &TM,
                             std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "Tricore Assembly Printer"; }

  void printOperand(const MachineInstr *MI, int opNum, raw_ostream &OS);
  void printMemOperand(const MachineInstr *MI, int opNum, raw_ostream &OS);

  void emitFunctionBodyStart() override;
  void emitInstruction(const MachineInstr *MI) override;

  static const char *getRegisterName(MCRegister Reg) {
    return TricoreInstPrinter::getRegisterName(Reg);
  }

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &O) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &O) override;

private:
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const;
  bool lowerToMCInst(const MachineInstr *MI, MCInst &OutMI);
};
} // end of anonymous namespace

void TricoreAsmPrinter::emitInstruction(const MachineInstr *MI) {

  MCInst OutInst;
  if (!lowerToMCInst(MI, OutInst))
    EmitToStreamer(*OutStreamer, OutInst);
}

void TricoreAsmPrinter::emitFunctionBodyStart() {
  /* Currently nothing todo. Maybe handle attribute naked or IRQ here?*/
}

void TricoreAsmPrinter::printOperand(const MachineInstr *MI, int opNum,
                                     raw_ostream &O) {
  assert(false);
}

void TricoreAsmPrinter::printMemOperand(const MachineInstr *MI, int opNum,
                                        raw_ostream &O) {
  assert(false);
}

bool TricoreAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                        const char *ExtraCode,
                                        raw_ostream &OS) {
  // First try the generic code, which knows about modifiers like 'c' and 'n'.
  if (!AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, OS))
    return false;

  return true;
}

bool TricoreAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                              unsigned OpNo,
                                              const char *ExtraCode,
                                              raw_ostream &O) {
  if (ExtraCode)
    return AsmPrinter::PrintAsmMemoryOperand(MI, OpNo, ExtraCode, O);

  const MachineOperand &AddrReg = MI->getOperand(OpNo);
  assert(MI->getNumOperands() > OpNo + 1 && "Expected additional operand");
  const MachineOperand &Offset = MI->getOperand(OpNo + 1);
  // All memory operands should have a register and an immediate operand (see
  // TricoreDAGToDAGISel::SelectInlineAsmMemoryOperand).
  if (!AddrReg.isReg())
    return true;
  if (!Offset.isImm() && !Offset.isGlobal() && !Offset.isBlockAddress() &&
      !Offset.isMCSymbol())
    return true;

  MCOperand MCO;
  if (!lowerOperand(Offset, MCO))
    return true;

  return false;
}

bool TricoreAsmPrinter::lowerToMCInst(const MachineInstr *MI, MCInst &OutMI) {

  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    if (lowerOperand(MO, MCOp))
      OutMI.addOperand(MCOp);
  }

  return false;
}

static MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym,
                                    const AsmPrinter &AP) {
  MCContext &Ctx = AP.OutContext;
  TricoreMCExpr::VariantKind Kind;

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case TricoreII::MO_None:
    Kind = TricoreMCExpr::VK_Tricore_None;
    break;
  case TricoreII::MO_CALL:
    Kind = TricoreMCExpr::VK_Tricore_24REL;
    break;
  }
  const MCExpr *ME =
      MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, Ctx);

  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset())
    ME = MCBinaryExpr::createAdd(
        ME, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);

  if (Kind != TricoreMCExpr::VK_Tricore_None)
    ME = TricoreMCExpr::create(Kind, ME, Ctx);
  return MCOperand::createExpr(ME);
}

bool TricoreAsmPrinter::lowerOperand(const MachineOperand &MO,
                                     MCOperand &MCOp) const {
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return false;
    MCOp = MCOperand::createReg(MO.getReg());
    break;
  case MachineOperand::MO_RegisterMask:
    // Regmasks are like implicit defs.
    return false;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    break;
  case MachineOperand::MO_GlobalAddress:
    MCOp = lowerSymbolOperand(MO, getSymbolPreferLocal(*MO.getGlobal()), *this);
    break;
  case MachineOperand::MO_ExternalSymbol:
    MCOp = lowerSymbolOperand(MO, GetExternalSymbolSymbol(MO.getSymbolName()),
                              *this);
    break;
  default:
    report_fatal_error("lowerOperand: unknown operand type");
  }
  return true;
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTricoreAsmPrinter() {
  RegisterAsmPrinter<TricoreAsmPrinter> X(getTheTricoreTarget());
}
