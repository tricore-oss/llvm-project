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
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
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
  std::unique_ptr<MCInstrInfo const> const MCII;

public:
  TricoreDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx,
                      MCInstrInfo const *MCII)
      : MCDisassembler(STI, Ctx), MCII(MCII) {}
  virtual ~TricoreDisassembler() = default;

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;

private:
  DecodeStatus getInstruction32(MCInst &Instr, uint64_t &Size,
                                ArrayRef<uint8_t> Bytes, uint64_t Address,
                                raw_ostream &CStream) const;

  DecodeStatus getInstruction16(MCInst &Instr, uint64_t &Size,
                                ArrayRef<uint8_t> Bytes, uint64_t Address,
                                raw_ostream &CStream) const;
};
} // namespace

static MCDisassembler *createTricoreDisassembler(const Target &T,
                                                 const MCSubtargetInfo &STI,
                                                 MCContext &Ctx) {
  return new TricoreDisassembler(STI, Ctx, T.createMCInstrInfo());
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTricoreDisassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(getTheTricoreTarget(),
                                         createTricoreDisassembler);
}

static DecodeStatus DecodeAGPRRegisterClass(MCInst &Inst, uint32_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder);
static DecodeStatus DecodeDGPRRegisterClass(MCInst &Inst, uint32_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder);
static DecodeStatus DecodePGPRRegisterClass(MCInst &Inst, uint32_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder);
static DecodeStatus DecodeEGPRRegisterClass(MCInst &Inst, uint32_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder);
template <unsigned N>
static DecodeStatus decodeUImmOperand(MCInst &Inst, uint32_t Imm,
                                      int64_t Address,
                                      const MCDisassembler *Decoder);
template <unsigned N>
static DecodeStatus decodeSImmOperand(MCInst &Inst, uint32_t Imm,
                                      int64_t Address,
                                      const MCDisassembler *Decoder);

template <unsigned N>
static DecodeStatus decodeDispImmOperand(MCInst &Inst, uint32_t Imm,
                                         int64_t Address,
                                         const MCDisassembler *Decoder);

#include "TricoreGenDisassemblerTables.inc"

static DecodeStatus DecodeAGPRRegisterClass(MCInst &Inst, uint32_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder) {
  if (RegNo >= 16)
    return MCDisassembler::Fail;

  MCRegister Reg = Tricore::A0 + RegNo;

  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeDGPRRegisterClass(MCInst &Inst, uint32_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder) {
  if (RegNo >= 16)
    return MCDisassembler::Fail;

  MCRegister Reg = Tricore::D0 + RegNo;

  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodePGPRRegisterClass(MCInst &Inst, uint32_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder) {
  if (RegNo >= 16 || (RegNo % 2 == 1))
    return MCDisassembler::Fail;

  MCRegister Reg = Tricore::P0 + RegNo / 2;

  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeEGPRRegisterClass(MCInst &Inst, uint32_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder) {
  if (RegNo >= 16 || RegNo % 2 == 1)
    return MCDisassembler::Fail;

  MCRegister Reg = Tricore::E0 + RegNo / 2;

  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeUImmOperand(MCInst &Inst, uint32_t Imm,
                                      int64_t Address,
                                      const MCDisassembler *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::createImm(Imm));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeSImmOperand(MCInst &Inst, uint32_t Imm,
                                      int64_t Address,
                                      const MCDisassembler *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::createImm(SignExtend64<N>(Imm)));
  return MCDisassembler::Success;
}

template <unsigned N>
static DecodeStatus decodeDispImmOperand(MCInst &Inst, uint32_t Imm,
                                         int64_t Address,
                                         const MCDisassembler *Decoder) {
  assert((isUInt<N>(Imm) && "Invalid immediate"));
  Inst.addOperand(MCOperand::createImm(SignExtend64<N>(Imm) << 1));
  return MCDisassembler::Success;
}

DecodeStatus TricoreDisassembler::getInstruction32(MCInst &Instr,
                                                   uint64_t &Size,
                                                   ArrayRef<uint8_t> Bytes,
                                                   uint64_t Address,
                                                   raw_ostream &CStream) const {
  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }
  Size = 4;

  uint32_t Insn = support::endian::read32le(Bytes.data());

  return decodeInstruction(DecoderTable32, Instr, Insn, Address, this, STI);
}

DecodeStatus TricoreDisassembler::getInstruction16(MCInst &Instr,
                                                   uint64_t &Size,
                                                   ArrayRef<uint8_t> Bytes,
                                                   uint64_t Address,
                                                   raw_ostream &CStream) const {
  if (Bytes.size() < 2) {
    Size = 0;
    return MCDisassembler::Fail;
  }
  Size = 2;

  uint32_t Insn = support::endian::read16le(Bytes.data());
  DecodeStatus Status =
      decodeInstruction(DecoderTable16, Instr, Insn, Address, this, STI);

  if (Status == MCDisassembler::Success) {
    const MCInstrDesc &MCID = MCII->get(Instr.getOpcode());
    auto *I = Instr.begin();
    for (auto Reg : MCID.implicit_defs()) {
      I = Instr.insert(I, MCOperand::createReg(Reg));
    }
    for (auto Reg : MCID.implicit_uses()) {
      I = Instr.insert(I, MCOperand::createReg(Reg));
    }
  }

  return Status;
}

DecodeStatus TricoreDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                                 ArrayRef<uint8_t> Bytes,
                                                 uint64_t Address,
                                                 raw_ostream &CStream) const {
  // It's a 16 bit instruction if bit 0 is 0.
  if ((Bytes[0] & 0x01) != 0x01)
    return getInstruction16(Instr, Size, Bytes, Address, CStream);

  return getInstruction32(Instr, Size, Bytes, Address, CStream);
}