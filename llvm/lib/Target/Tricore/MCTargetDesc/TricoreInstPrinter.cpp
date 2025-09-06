//=== TricoreInstPrinter.cpp - Convert Tricore MCInst to assembly syntax --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints an Tricore MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "TricoreInstPrinter.h"
#include "TricoreBaseInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define GET_INSTRUCTION_NAME
#define PRINT_ALIAS_INSTR
#include "TricoreGenAsmWriter.inc"

void TricoreInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                   StringRef Annot, const MCSubtargetInfo &STI,
                                   raw_ostream &O) {
  bool Res = false;
  const MCInst *NewMI = MI;
  MCInst UncompressedMI;
  Res = TricoreCI::uncompress(UncompressedMI, *MI, STI);
  if (Res)
    NewMI = const_cast<MCInst *>(&UncompressedMI);
  printInstruction(NewMI, Address, STI, O);
  printAnnotation(O, Annot);
}

void TricoreInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                      const MCSubtargetInfo &STI,
                                      raw_ostream &O, const char *Modifier) {
  assert((Modifier == nullptr || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    markup(O, Markup::Immediate) << formatImm(MO.getImm());
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MO.getExpr()->print(O, &MAI);
}

void TricoreInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) {
  markup(OS, Markup::Register) << getRegisterName(Reg);
}

void TricoreInstPrinter::printMemOperand(const MCInst *MI, unsigned OpNo,
                                         const MCSubtargetInfo &STI,
                                         raw_ostream &O, const char *Modifier) {
  WithMarkup M = WithMarkup(*this, O, Markup::Memory, UseMarkup, UseColor);
  O << "[";
  if (MI->getFlags() == 1)
    O << "+";
  printOperand(MI, OpNo, STI, O, Modifier);
  if (MI->getFlags() == 2)
    O << "+";
  O << "]";
  const MCOperand OffsetOp = MI->getOperand(OpNo + 1);
  if (OffsetOp.isImm()) {
    if (OffsetOp.getImm() != 0)
      O << formatImm(OffsetOp.getImm());
    return;
  }
  assert(OffsetOp.isExpr() && "Unknown operand kind in printOperand");
  OffsetOp.getExpr()->print(O, &MAI);
}

void TricoreInstPrinter::printBranchOperand(const MCInst *MI, uint64_t Address,
                                            unsigned OpNo,
                                            const MCSubtargetInfo &STI,
                                            raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);
  if (!MO.isImm())
    return printOperand(MI, OpNo, STI, O);

  if (PrintBranchImmAsAddress) {
    uint64_t Target = Address + MO.getImm();
    Target &= 0xffffffff;
    markup(O, Markup::Target) << formatHex(Target);
  } else {
    markup(O, Markup::Target) << formatImm(MO.getImm());
  }
}