//===-- TricoreAsmParser.cpp - Parse Tricore assembly to MCInst instructions
//----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TricoreMCExpr.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "TargetInfo/TricoreTargetInfo.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCAsmParserExtension.h"
#include "llvm/MC/MCParser/MCAsmParserUtils.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/SubtargetFeature.h"
#include "llvm/TargetParser/Triple.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

using namespace llvm;

#define DEBUG_TYPE "tricore-asm-parser"

namespace {

class TricoreAsmParser : public MCTargetAsmParser {
  MCAsmParser &Parser;
  const MCRegisterInfo &MRI;

  /// @name Auto-generated Match Functions
  /// {

#define GET_ASSEMBLER_HEADER
#include "TricoreGenAsmMatcher.inc"

  /// }

  // public interface of the MCTargetAsmParser.
  bool matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;
  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;
  ParseStatus tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                               SMLoc &EndLoc) override;
  bool parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;
  ParseStatus parseDirective(AsmToken DirectiveID) override;

  unsigned validateTargetOperandClass(MCParsedAsmOperand &Op,
                                      unsigned Kind) override;

public:
  TricoreAsmParser(const MCSubtargetInfo &sti, MCAsmParser &parser,
                   const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, sti, MII), Parser(parser),
        MRI(*Parser.getContext().getRegisterInfo()) {}

  enum TricoreMatchResultTy {
    Match_Dummy = FIRST_TARGET_MATCH_RESULT_TY,
#define GET_OPERAND_DIAGNOSTIC_TYPES
#include "TricoreGenAsmMatcher.inc"
#undef GET_OPERAND_DIAGNOSTIC_TYPES
  };
};
} // namespace

namespace {

/// TricoreOperand - Instances of this class represent a parsed Tricore machine
/// instruction.
class TricoreOperand : public MCParsedAsmOperand {
public:
  enum RegisterKind {
    rk_None,
    rk_Data,
    rk_Addr,
    rk_DataDouble,
    rk_AddrDouble,
  };

private:
  enum KindTy {
    k_Token,
    k_Register,
    k_Immediate,
    k_MemoryReg,
    k_MemoryImm,
    k_ASITag,
    k_PrefetchTag,
    k_TailRelocSym, // Special kind of immediate for TLS relocation purposes.
  } Kind;

  SMLoc StartLoc, EndLoc;

  struct Token {
    const char *Data;
    unsigned Length;
  };

  struct RegOp {
    unsigned RegNum;
    RegisterKind Kind;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  struct MemOp {
    unsigned Base;
    unsigned OffsetReg;
    const MCExpr *Off;
  };

  union {
    struct Token Tok;
    struct RegOp Reg;
    struct ImmOp Imm;
    struct MemOp Mem;
    unsigned ASI;
    unsigned Prefetch;
  };

public:
  TricoreOperand(KindTy K) : Kind(K) {}

public:
  static bool evaluateConstantImm(const MCExpr *Expr, int64_t &Imm,
                                  TricoreMCExpr::VariantKind &VK) {
    if (auto *RE = dyn_cast<TricoreMCExpr>(Expr)) {
      VK = RE->getKind();
      return RE->evaluateAsConstant(Imm);
    }

    if (auto CE = dyn_cast<MCConstantExpr>(Expr)) {
      VK = TricoreMCExpr::VK_Tricore_None;
      Imm = CE->getValue();
      return true;
    }

    return false;
  }

  bool isSImm10() const {
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      assert(
          false); // IsValid = TricoreAsmParser::classifySymbolRef(getImm(), VK);
    else
      IsValid = isInt<10>(Imm);
    return IsValid &&
           ((IsConstantImm && VK == TricoreMCExpr::VK_Tricore_None) ||
            VK == TricoreMCExpr::VK_Tricore_LO);
  }

  bool isSImm16() const {
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      assert(
          false); // IsValid = TricoreAsmParser::classifySymbolRef(getImm(), VK);
    else
      IsValid = isInt<12>(Imm);
    return IsValid &&
           ((IsConstantImm && VK == TricoreMCExpr::VK_Tricore_None) ||
            VK == TricoreMCExpr::VK_Tricore_LO);
  }

  bool isUImm8() const {
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      assert(
          false); // IsValid = TricoreAsmParser::classifySymbolRef(getImm(), VK);
    else
      IsValid = isUInt<8>(Imm);
    return IsValid &&
           ((IsConstantImm && VK == TricoreMCExpr::VK_Tricore_None) ||
            VK == TricoreMCExpr::VK_Tricore_LO);
  }

  StringRef getToken() const {
    assert(Kind == k_Token && "Invalid access!");
    return StringRef(Tok.Data, Tok.Length);
  }

  const MCExpr *getImm() const {
    assert((Kind == k_Immediate) && "Invalid access!");
    return Imm.Val;
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    // Add as immediate when possible.  Null MCExpr = 0.
    if (!Expr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    const MCExpr *Expr = getImm();
    addExpr(Inst, Expr);
  }
};
} // namespace

bool TricoreAsmParser::matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                               OperandVector &Operands,
                                               MCStreamer &Out,
                                               uint64_t &ErrorInfo,
                                               bool MatchingInlineAsm) {
  assert(false); // FIXME: Implement this.
  return false;
}

bool TricoreAsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                     SMLoc &EndLoc) {
  assert(false); // FIXME: Implement this.
  return false;
}

ParseStatus TricoreAsmParser::tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                               SMLoc &EndLoc) {
  assert(false); // FIXME: Implement this.
  return ParseStatus::Failure;
}

bool TricoreAsmParser::parseInstruction(ParseInstructionInfo &Info,
                                        StringRef Name, SMLoc NameLoc,
                                        OperandVector &Operands) {
  assert(false); // FIXME: Implement this.
  return false;
}

ParseStatus TricoreAsmParser::parseDirective(AsmToken DirectiveID) {
  assert(false); // FIXME: Implement this.
  return ParseStatus::Failure;
}
unsigned TricoreAsmParser::validateTargetOperandClass(MCParsedAsmOperand &Op,
                                                      unsigned Kind) {
  assert(false); // FIXME: Implement this.
  return 0;
}

#define GET_MATCHER_IMPLEMENTATION
#include "TricoreGenAsmMatcher.inc"

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTricoreAsmParser() {
  RegisterMCAsmParser<TricoreAsmParser> A(getTheTricoreTarget());
}