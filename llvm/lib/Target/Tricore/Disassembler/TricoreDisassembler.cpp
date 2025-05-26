//===- TricoreDisassembler.cpp - Disassembler for Tricore
//-----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is part of the Tricore Disassembler.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "TargetInfo/TricoreTargetInfo.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "tricore-disassembler"

using DecodeStatus = MCDisassembler::DecodeStatus;

namespace {
/// A disassembler class for Tricore.
class TricoreDisassembler : public MCDisassembler {
public:
  TricoreDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  virtual ~TricoreDisassembler() = default;

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};
} // namespace

static MCDisassembler *createTricoreDisassembler(const Target &T,
                                                 const MCSubtargetInfo &STI,
                                                 MCContext &Ctx) {
  return new TricoreDisassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTricoreDisassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(getTheTricoreTarget(),
                                         createTricoreDisassembler);
}

DecodeStatus TricoreDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                                 ArrayRef<uint8_t> Bytes,
                                                 uint64_t Address,
                                                 raw_ostream &CStream) const {
  // FIXME: Implement this.
  assert(false);
  return DecodeStatus::Fail;
}