//===-- TricoreAsmParser.cpp - Parse Tricore assembly to MCInst instructions
//----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TricoreInstPrinter.h"
#include "MCTargetDesc/TricoreMCExpr.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "TargetInfo/TricoreTargetInfo.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/Twine.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmMacro.h"
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
#include "llvm/MC/MCRegister.h"
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

  ParseStatus parseCallTarget(OperandVector &Operands);
  bool parseOperand(OperandVector &Operands, StringRef Mnemonic);

  unsigned validateTargetOperandClass(MCParsedAsmOperand &Op,
                                      unsigned Kind) override;

  // Check instruction constraints.
  bool validateInstruction(MCInst &Inst, OperandVector &Operands);

  // post process pseudo instructions
  bool processInstruction(MCInst &Inst, SMLoc IDLoc, OperandVector &Operands,
                          MCStreamer &Out);

// Auto-generated instruction matching functions
#define GET_ASSEMBLER_HEADER
#include "TricoreGenAsmMatcher.inc"

  ParseStatus parseRegister(OperandVector &Operands);
  ParseStatus parseImmediate(OperandVector &Operands);
  ParseStatus parseMemOperand(OperandVector &Operands);

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
  enum MemoryKind { Offset, PostInc, PreInc };

private:
  enum KindTy {
    Token,
    Register,
    Immediate,
    Memory,
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
    unsigned RegNum;
    const MCExpr *Off;
    MemoryKind Kind;
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

  bool isToken() const override { return Kind == Token; }
  bool isImm() const override { return Kind == Immediate; }
  bool isReg() const override { return Kind == Register; }
  MCRegister getReg() const override {
    assert(isReg() || isMem());
    return isReg() ? Reg.RegNum : Mem.RegNum;
  }
  bool isMem() const override { return Kind == Memory; }

  SMLoc getStartLoc() const override { return StartLoc; }
  SMLoc getEndLoc() const override { return EndLoc; }

  void print(raw_ostream &OS) const override {
    auto RegName = [](MCRegister Reg) {
      if (Reg)
        return TricoreInstPrinter::getRegisterName(Reg);
      else
        return "noreg";
    };

    switch (Kind) {
    case KindTy::Immediate:
      OS << *getImm();
      break;
    case KindTy::Register:
      OS << "<register " << RegName(getReg()) << ">";
      break;
    case KindTy::Memory:
      OS << "<memory " << (Mem.Kind == PreInc ? "+" : "") << RegName(getReg())
         << (Mem.Kind == PostInc ? "+" : "") << *Mem.Off;
      break;
    case KindTy::Token:
      OS << "'" << getToken() << "'";
      break;
    }
  }

public:
  static std::unique_ptr<TricoreOperand> createToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<TricoreOperand>(KindTy::Token);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = S;
    return Op;
  }

  static std::unique_ptr<TricoreOperand> createReg(MCRegister Reg, SMLoc S,
                                                   SMLoc E) {
    auto Op = std::make_unique<TricoreOperand>(KindTy::Register);
    Op->Reg.RegNum = Reg;
    Op->Reg.Kind = rk_None;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<TricoreOperand> createImm(const MCExpr *Val, SMLoc S,
                                                   SMLoc E) {
    auto Op = std::make_unique<TricoreOperand>(KindTy::Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<TricoreOperand> createMemory(MCRegister Reg,
                                                      MemoryKind MK,
                                                      const MCExpr *Off,
                                                      SMLoc S, SMLoc E) {
    auto Op = std::make_unique<TricoreOperand>(KindTy::Memory);
    Op->Mem.RegNum = Reg;
    Op->Mem.Kind = MK;
    Op->Mem.Off = Off;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static bool classifySymbolRef(const MCExpr *Expr,
                                TricoreMCExpr::VariantKind &Kind) {
    Kind = TricoreMCExpr::VK_Tricore_None;

    if (const TricoreMCExpr *RE = dyn_cast<TricoreMCExpr>(Expr)) {
      Kind = RE->getKind();
      Expr = RE->getSubExpr();
    }

    MCValue Res;
    if (Expr->evaluateAsRelocatable(Res, nullptr, nullptr))
      return Res.getRefKind() == TricoreMCExpr::VK_Tricore_None;
    return false;
  }

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

  bool isCallTarget() const {
    int64_t Imm;
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    if (!isImm() || evaluateConstantImm(getImm(), Imm, VK))
      return false;

    if (Imm % 2 != 0)
      return false;

    return classifySymbolRef(getImm(), VK) &&
           VK == TricoreMCExpr::VK_Tricore_24REL;
  }

  template <int bits, int mode> bool isMemWithSimmOffset() const {
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    int64_t Imm;
    if (!isMem())
      return false;
    bool IsConstantImm = evaluateConstantImm(Mem.Off, Imm, VK);
    
    return IsConstantImm && isInt<bits>(Imm) && Mem.Kind == mode;
  }

  template <int bits> bool isDisp() const {
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      assert(false); // IsValid = TricoreAsmParser::classifySymbolRef(getImm(),
                     // VK);
    else
      IsValid = isShiftedInt<bits, 1>(Imm);
    return IsValid &&
           ((IsConstantImm && VK == TricoreMCExpr::VK_Tricore_None) ||
            VK == TricoreMCExpr::VK_Tricore_LO);
  }

  bool isSImm10() const {
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    int64_t Imm;
    bool IsValid;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    if (!IsConstantImm)
      assert(false); // IsValid = TricoreAsmParser::classifySymbolRef(getImm(),
                     // VK);
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
      assert(false); // IsValid = TricoreAsmParser::classifySymbolRef(getImm(),
                     // VK);
    else
      IsValid = isInt<12>(Imm);
    return IsValid &&
           ((IsConstantImm && VK == TricoreMCExpr::VK_Tricore_None) ||
            VK == TricoreMCExpr::VK_Tricore_LO);
  }

  template <unsigned N, int P = 0> bool isUImm() const {
    if (!isImm())
      return false;

    int64_t Imm;
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    return IsConstantImm && isUInt<N>(Imm - P) &&
           VK == TricoreMCExpr::VK_Tricore_None;
  }

  template <unsigned N, unsigned S = 0> bool isSImm() const {
    if (!isImm())
      return false;

    int64_t Imm;
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);
    return IsConstantImm && isShiftedInt<N, S>(Imm) &&
           VK == TricoreMCExpr::VK_Tricore_None;
  }

  bool isSImm4() const { return isSImm<4>(); }
  bool isSImm9() const { return isSImm<9>(); }

  bool isUImm4() const { return isUImm<4>(); }
  bool isUImm5() const { return isUImm<5>(); }
  bool isUImm8() const { return isUImm<8>(); }
  bool isUImm16() const { return isUImm<16>(); }

  bool isPos() const {
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    int64_t Imm;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);

    return IsConstantImm && Imm >= 0 && Imm < 32;
  }

  bool isWidth() const {
    TricoreMCExpr::VariantKind VK = TricoreMCExpr::VK_Tricore_None;
    int64_t Imm;
    if (!isImm())
      return false;
    bool IsConstantImm = evaluateConstantImm(getImm(), Imm, VK);

    return IsConstantImm && Imm > 0 && Imm <= 32;
  }

  StringRef getToken() const {
    assert(Kind == Token && "Invalid access!");
    return StringRef(Tok.Data, Tok.Length);
  }

  const MCExpr *getImm() const {
    assert((Kind == Immediate) && "Invalid access!");
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
  void addMemOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(Mem.RegNum));
    addExpr(Inst, Mem.Off);
    Inst.setFlags(Mem.Kind);
  }
};
} // namespace

#define GET_REGISTER_MATCHER
#define GET_SUBTARGET_FEATURE_NAME
#define GET_MATCHER_IMPLEMENTATION
#define GET_MNEMONIC_SPELL_CHECKER
#include "TricoreGenAsmMatcher.inc"

bool TricoreAsmParser::validateInstruction(MCInst &Inst,
                                           OperandVector &Operands) {
  return false;
}

bool TricoreAsmParser::processInstruction(MCInst &Inst, SMLoc IDLoc,
                                          OperandVector &Operands,
                                          MCStreamer &Out) {

  return false;
}

bool TricoreAsmParser::matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                               OperandVector &Operands,
                                               MCStreamer &Out,
                                               uint64_t &ErrorInfo,
                                               bool MatchingInlineAsm) {
  MCInst Inst;
  FeatureBitset MissingFeatures;

  auto Result = MatchInstructionImpl(Operands, Inst, ErrorInfo, MissingFeatures,
                                     MatchingInlineAsm);
  switch (Result) {
  default:
    break;
  case Match_Success:
    if (validateInstruction(Inst, Operands))
      return true;
    return processInstruction(Inst, IDLoc, Operands, Out);
  case Match_MissingFeature: {
    assert(MissingFeatures.any() && "Unknown missing features!");
    bool FirstFeature = true;
    std::string Msg = "instruction requires the following:";
    for (unsigned i = 0, e = MissingFeatures.size(); i != e; ++i) {
      if (MissingFeatures[i]) {
        Msg += FirstFeature ? " " : ", ";
        Msg += getSubtargetFeatureName(i);
        FirstFeature = false;
      }
    }
    return Error(IDLoc, Msg);
  }
  case Match_MnemonicFail: {
    FeatureBitset FBS = ComputeAvailableFeatures(getSTI().getFeatureBits());
    std::string Suggestion = TricoreMnemonicSpellCheck(
        ((TricoreOperand &)*Operands[0]).getToken(), FBS, 0);
    return Error(IDLoc, "unrecognized instruction mnemonic" + Suggestion);
  }
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0ULL) {
      if (ErrorInfo >= Operands.size())
        return Error(ErrorLoc, "too few operands for instruction");

      ErrorLoc = ((TricoreOperand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }
    return Error(ErrorLoc, "invalid operand for instruction");
  }
  }

  return false;
}

bool TricoreAsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                     SMLoc &EndLoc) {
  if (!tryParseRegister(Reg, StartLoc, EndLoc).isSuccess())
    return Error(StartLoc, "invalid register name");
  return false;
}

ParseStatus TricoreAsmParser::tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                               SMLoc &EndLoc) {
  const AsmToken &Tok = getParser().getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  StringRef Name = getLexer().getTok().getIdentifier();

  Reg = MatchRegisterName(Name);
  if (!Reg)
    Reg = MatchRegisterAltName(Name);
  if (!Reg)
    return ParseStatus::NoMatch;

  getParser().Lex(); // Eat identifier token.
  return ParseStatus::Success;
}

ParseStatus TricoreAsmParser::parseRegister(OperandVector &Operands) {
  StringRef Name;
  SMLoc S = getParser().getTok().getLoc();

  if (getParser().parseIdentifier(Name))
    return ParseStatus::NoMatch;

  MCRegister Reg = MatchRegisterName(Name);
  if (!Reg)
    Reg = MatchRegisterAltName(Name);
  if (!Reg)
    return ParseStatus::Failure;

  Operands.push_back(
      TricoreOperand::createReg(Reg, S, S.getFromPointer(Name.end())));

  return ParseStatus::Success;
}

ParseStatus TricoreAsmParser::parseImmediate(OperandVector &Operands) {
  SMLoc S = getParser().getTok().getLoc();
  SMLoc E;
  const MCExpr *Res;

  switch (getLexer().getKind()) {
  default:
    return ParseStatus::NoMatch;
  case AsmToken::LParen:
  case AsmToken::Dot:
  case AsmToken::Minus:
  case AsmToken::Plus:
  case AsmToken::Exclaim:
  case AsmToken::Tilde:
  case AsmToken::Integer:
  case AsmToken::String:
  case AsmToken::Identifier:
    if (getParser().parseExpression(Res, E))
      return ParseStatus::Failure;
    break;
  case AsmToken::Percent:
    // return parseOperandWithModifier(Operands);
    return ParseStatus::Failure;
  }

  E = getParser().getTok().getEndLoc();
  Operands.push_back(TricoreOperand::createImm(Res, S, E));
  return ParseStatus::Success;
}

ParseStatus TricoreAsmParser::parseMemOperand(OperandVector &Operands) {
  StringRef Name;
  MCRegister Reg;
  TricoreOperand::MemoryKind MemKind = TricoreOperand::Offset;
  const MCExpr *Off;
  SMLoc S = getParser().getTok().getLoc();
  SMLoc E;

  if (!getParser().parseOptionalToken(AsmToken::LBrac))
    return ParseStatus::NoMatch;

  if (getParser().parseOptionalToken(AsmToken::Plus))
    MemKind = TricoreOperand::PreInc;

  if (getParser().parseIdentifier(Name))
    return Error(getTok().getLoc(),
                 "expected valid identifier for memory register");

  Reg = MatchRegisterName(Name);
  if (!Reg)
    Reg = MatchRegisterAltName(Name);
  if (!Reg)
    return ParseStatus::Failure;

  if (getTok().is(AsmToken::Plus)) {

    if (MemKind != TricoreOperand::Offset) {
      return Error(getTok().getLoc(),
                   "operand can use only one increment modifier");
    }
    Lex();
    MemKind = TricoreOperand::PreInc;
  }

  if (!getTok().is(AsmToken::RBrac)) {
    return ParseStatus::Failure;
  }
  Lex();

  if (getParser().parseExpression(Off, E))
    return ParseStatus::Failure;

  Operands.push_back(TricoreOperand::createMemory(Reg, MemKind, Off, S, E));

  return ParseStatus::Success;
}

bool TricoreAsmParser::parseOperand(OperandVector &Operands,
                                    StringRef Mnemonic) {
  ParseStatus Result =
      MatchOperandParserImpl(Operands, Mnemonic, /*ParseForAllFeatures=*/true);
  if (Result.isSuccess())
    return false;
  if (Result.isFailure())
    return true;

  // Attempt to parse token as a register.
  if (parseRegister(Operands).isSuccess())
    return false;

  // Attempt to parse token as an immediate
  if (parseImmediate(Operands).isSuccess())
    return false;

  if (parseMemOperand(Operands).isSuccess())
    return false;

  Error(getParser().getTok().getLoc(), "unknown operand");
  return true;
}

bool TricoreAsmParser::parseInstruction(ParseInstructionInfo &Info,
                                        StringRef Name, SMLoc NameLoc,
                                        OperandVector &Operands) {
  // First operand is token for instruction
  Operands.push_back(TricoreOperand::createToken(Name, NameLoc));

  // If there are no more operands, then finish
  if (getLexer().is(AsmToken::EndOfStatement)) {
    getParser().Lex(); // Consume the EndOfStatement.
    return false;
  }

  // Parse first operand
  if (parseOperand(Operands, Name))
    return true;

  // Parse until end of statement, consuming commas between operands
  while (parseOptionalToken(AsmToken::Comma)) {
    // Parse next operand
    if (parseOperand(Operands, Name))
      return true;
  }

  if (getParser().parseEOL("unexpected token")) {
    getParser().eatToEndOfStatement();
    return true;
  }
  return false;
}

ParseStatus TricoreAsmParser::parseDirective(AsmToken DirectiveID) {

  // Let the MC layer to handle other directives.
  return ParseStatus::NoMatch;
}

ParseStatus TricoreAsmParser::parseCallTarget(OperandVector &Operands) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);

  switch (getLexer().getKind()) {
  default:
    return ParseStatus::NoMatch;
  case AsmToken::LParen:
  case AsmToken::Integer:
  case AsmToken::Identifier:
  case AsmToken::Dot:
    break;
  }

  const MCExpr *DestValue;
  if (getParser().parseExpression(DestValue))
    return ParseStatus::NoMatch;

  bool IsPic = getContext().getObjectFileInfo()->isPositionIndependent();
  TricoreMCExpr::VariantKind Kind = llvm::TricoreMCExpr::VK_Tricore_24REL;

  const MCExpr *DestExpr = TricoreMCExpr::create(Kind, DestValue, getContext());
  Operands.push_back(TricoreOperand::createImm(DestExpr, S, E));
  return ParseStatus::Success;
}

unsigned TricoreAsmParser::validateTargetOperandClass(MCParsedAsmOperand &AsmOp,
                                                      unsigned Kind) {
  TricoreOperand &Op = static_cast<TricoreOperand &>(AsmOp);

  return Match_InvalidOperand;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTricoreAsmParser() {
  RegisterMCAsmParser<TricoreAsmParser> A(getTheTricoreTarget());
}