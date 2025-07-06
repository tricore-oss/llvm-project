include "TricoreInstrFormat.td"

//===----------------------------------------------------------------------===//
// Operands
//===----------------------------------------------------------------------===//
class ImmAsmOperand<string prefix, int width, string suffix> : AsmOperandClass {
  let Name = prefix#"Imm"#width#suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}
class SImmAsmOperand<int width, string suffix = "">
    : ImmAsmOperand<"S", width, suffix> {}

class UImmAsmOperand<int width, string suffix = "">
    : ImmAsmOperand<"U", width, suffix> {}

class DispImmAsmOperand<int width> : AsmOperandClass {
  let Name = "Disp"#width;
  let RenderMethod = "addImmOperands";
  let ParserMethod = "parseCallTarget";
  let PredicateMethod = "isDisp<"#width#">";
  let DiagnosticType = !strconcat("Invalid", Name);
}

def WidthAsmOperand : AsmOperandClass {
  let Name = "Width";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isWidth";
  let DiagnosticType = !strconcat("Invalid", Name);
}

def PosAsmOperand : AsmOperandClass {
  let Name = "Pos";
  let RenderMethod = "addImmOperands";
  let PredicateMethod = "isPos";
  let DiagnosticType = !strconcat("Invalid", Name);
}

class TricoreOp<ValueType vt> : Operand<vt> {
  let OperandNamespace = "TricoreOp";
}

class TricoreSImmOp<int bitsNum, ValueType Vt = i32> : TricoreOp<Vt> {
  let ParserMatchClass = SImmAsmOperand<bitsNum>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<"#bitsNum#">";
  let OperandType = "OPERAND_SIMM"#bitsNum;
  let MCOperandPredicate = [{
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isInt<}]#bitsNum#[{>(Imm);
  }];
}

class TricoreUImmOp<int bitsNum> : TricoreOp<i32> {
  let ParserMatchClass = UImmAsmOperand<bitsNum>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<"#bitsNum#">";
  let OperandType = "OPERAND_UIMM"#bitsNum;
  let MCOperandPredicate = [{
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isUInt<}]#bitsNum#[{>(Imm);
  }];
}

class TricoreDispImmOp<int bitsNum> : TricoreOp<i32> {
  let ParserMatchClass = DispImmAsmOperand<bitsNum>;
  let EncoderMethod = "getDispImmOpValue";
  let DecoderMethod = "decodeDispImmOperand<"#bitsNum#">";
  let OperandType = "OPERAND_DISP"#bitsNum;
  let MCOperandPredicate = [{
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isShiftedInt<}]#bitsNum#[{, 1>(Imm);
  }];
}

class TricoreBrDispImmOp<int bitsNum> : Operand<OtherVT> {
  let EncoderMethod = "getDispImmOpValue";
  let DecoderMethod = "decodeDispImmOperand<"#bitsNum#">";
}

class TricoreWidthOp : TricoreOp<i32> {
  let ParserMatchClass = WidthAsmOperand;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<5>";
  let OperandType = "OPERAND_WIDTH";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return imm > 0 && imm <= 32;
  }];
}
def width : TricoreWidthOp, ImmLeaf<i32, "return Imm > 0 && Imm <= 32;">;

class TricorePosOp : TricoreOp<i32> {
  let ParserMatchClass = PosAsmOperand;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<5>";
  let OperandType = "OPERAND_WIDTH";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return imm >= 0 && imm < 32;
  }];
}
def pos : TricorePosOp, ImmLeaf<i32, "return Imm >= 0 && Imm < 32;">;

class TricoreSImmLeafOp<int bitsNum>
    : TricoreSImmOp<bitsNum>, ImmLeaf<i32, "return isInt<"#bitsNum#">(Imm);">;
def simm4 : TricoreSImmLeafOp<4>;
// def simm5 : TricoreSImmLeafOp<5>;
// def simm8 : TricoreSImmLeafOp<8>;
def simm9 : TricoreSImmLeafOp<9>;
def simm10 : TricoreSImmLeafOp<10>;
def simm16 : TricoreSImmLeafOp<16>;

def simm9_64 : TricoreSImmOp<9, i64>, ImmLeaf<i64, "return isInt<9>(Imm);">;
def simm16_64 : TricoreSImmOp<16, i64>, ImmLeaf<i64, "return isInt<16>(Imm);">;

class TricoreUImmLeafOp<int bitsNum>
    : TricoreUImmOp<bitsNum>, ImmLeaf<i32, "return isUInt<"#bitsNum#">(Imm);">;
def uimm4 : TricoreUImmLeafOp<4>;
def uimm5 : TricoreUImmLeafOp<5>;
def uimm8 : TricoreUImmLeafOp<8>;
def uimm16 : TricoreUImmLeafOp<16>;

def tuimm16 : TricoreUImmOp<16>, TImmLeaf<i32, "return isUInt<16>(Imm);">;

class TricoreDispImmLeafOp<int bitsNum>
    : TricoreDispImmOp<bitsNum>,
      ImmLeaf<i32, "return isShiftedInt<"#bitsNum#", 1>(Imm);">;

def disp24 : TricoreDispImmLeafOp<24>;
def br_disp4 : TricoreBrDispImmOp<15>;
def br_disp15 : TricoreBrDispImmOp<15>;
def br_disp24 : TricoreBrDispImmOp<24>;

def MemBOAsmOperand : AsmOperandClass {
  let Name = "MemBO";
  let RenderMethod = "addMemOperands";
  let ParserMethod = "parseMemOperand";
  let PredicateMethod = "isMemWithSimmOffset<10>";
  let DiagnosticType = "InvalidMemBO";
}

def MemBO : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops AGPR, simm10);
  let ParserMatchClass = MemBOAsmOperand;
  let OperandType = "OPERAND_MEMORY";
}

//===----------------------------------------------------------------------===//
// Instruction Pattern Stuff
//===----------------------------------------------------------------------===//

/// Split a 32-bit immediate into two 16 bit parts.
def hi16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((uint32_t)N->getZExtValue() >> 16, SDLoc(N),
                                   MVT::i32);
}]>;

def lo16 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((uint32_t)N->getZExtValue() & 0xFFFF, SDLoc(N),
                                   MVT::i32);
}]>;
def plus1 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((uint32_t)N->getZExtValue() + 1, SDLoc(N),
                                   MVT::i32);
}]>;
def negate : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((uint32_t)N->getZExtValue() * -1, SDLoc(N),
                                   MVT::i32);
}]>;

def lo16AllZero : PatLeaf<(i32 imm), [{
  // Returns true if all low 16-bits are 0.
  return (((uint32_t)N->getZExtValue()) & 0xFFFFUL) == 0;
}],
                          hi16>;

def AddrRegImm : ComplexPattern<iPTR, 2, "SelectAddrRegImm">;

// Target-independent type requirements, but with target-specific formats.
def SDT_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_CallSeqEnd : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

def SDT_TricoreCall : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;

def SDT_EXTR
    : SDTypeProfile<
          1, 3, [SDTCisSameAs<0, 1>, SDTCisInt<2>, SDTCisInt<0>, SDTCisInt<3>]>;
def SDT_INSERT : SDTypeProfile<1, 4,
                               [SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>,
                                SDTCisInt<4>, SDTCisInt<0>, SDTCisInt<3>]>;

// Target-independent nodes, but with target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START",
                           SDT_CallSeqStart, [SDNPHasChain, SDNPOutGlue]>;
def callseq_end
    : SDNode<"ISD::CALLSEQ_END",
             SDT_CallSeqEnd, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

// Target-dependent nodes.
def tricore_call : SDNode<"TricoreISD::CALL", SDT_TricoreCall,
                          [SDNPMayStore, SDNPHasChain, SDNPOptInGlue,
                           SDNPOutGlue, SDNPVariadic]>;
def tricore_tail
    : SDNode<"TricoreISD::TAIL", SDT_TricoreCall,
             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;
def tricore_ret_glue
    : SDNode<"TricoreISD::RET_GLUE",
             SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def tricore_lea : SDNode<"TricoreISD::LEA", SDTIntBinOp>;
def tricore_movha : SDNode<"TricoreISD::MOVHA", SDTIntUnaryOp>;

def tricore_extr : SDNode<"TricoreISD::EXTR", SDT_EXTR>;
def tricore_extru : SDNode<"TricoreISD::EXTRU", SDT_EXTR>;
def tricore_insert : SDNode<"TricoreISD::INSERT", SDT_INSERT>;

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

let isBarrier = 1, isReturn = 1, isTerminator = 1 in {
  def ret32
      : TCInstSYS<0x0D, 0x06, (outs), (ins), [(tricore_ret_glue)], "ret"> {
    let s1 = 0;
  }
}

let Defs = [A10], Uses = [A10] in {
  def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                [(callseq_start timm:$amt1, timm:$amt2)]>;
  def ADJCALLSTACKUP : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(callseq_end timm:$amt1, timm:$amt2)]>;
}

let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in {
  class Load_so<bits<6> op2, string opcodestr, RegisterClass RC = DGPR>
      : TCInstBO<0x09, op2, (outs RC:$a), (ins(MemBO $b, $imm):$addr), [],
                 opcodestr, "$a, $addr">;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in {
  class Store_so<bits<6> op2, string opcodestr, RegisterClass RC = DGPR>
      : TCInstBO<0x89, op2, (outs), (ins RC:$a, (MemBO $b, $imm):$addr), [],
                 opcodestr, "$addr, $a">;
}

//===--
// Context
//===
let isCall = 1, isBarrier = 1, mayStore = 1, mayLoad = 0,
    hasSideEffects = 0 in {

  def CALL_B
      : TCInstB<0x6D, (outs), (ins disp24:$disp24), [], "call", "$disp24">;
}

def : Pat<(tricore_call tglobaladdr:$disp24), (CALL_B tglobaladdr:$disp24)>;
def : Pat<(tricore_call texternalsym:$disp24), (CALL_B texternalsym:$disp24)>;

//===----------------------------------------------------------------------===//
//  Load/Store instructions
//===----------------------------------------------------------------------===//

def LDB_SO : Load_so<0x20, "ld.b">;
def LDH_SO : Load_so<0x22, "ld.h">;
def LDW_SO : Load_so<0x24, "ld.w">;
def LDD_SO : Load_so<0x25, "ld.d">;
def LDA_SO : Load_so<0x26, "ld.a", AGPR>;

def STB_SO : Store_so<0x20, "st.b">;
def STH_SO : Store_so<0x22, "st.h">;
def STW_SO : Store_so<0x24, "st.w">;
def STD_SO : Store_so<0x25, "st.d">;
def STA_SO : Store_so<0x26, "st.a", AGPR>;

// Load
class LdPat<PatFrag LoadOp, TCInstCommon Inst, ValueType vt = i32>
    : Pat<(vt(LoadOp AddrRegImm:$addr)), (Inst AddrRegImm:$addr)>;

def : LdPat<sextloadi8, LDB_SO>;
def : LdPat<sextloadi16, LDH_SO>;
def : LdPat<load, LDW_SO>;

class StPat<PatFrag StoreOp, TCInstCommon Inst, ValueType vt = i32,
            RegisterClass RC = DGPR>
    : Pat<(StoreOp(vt RC:$a), AddrRegImm:$addr), (Inst vt:$a,
                                                     AddrRegImm:$addr)>;

def : StPat<truncstorei8, STB_SO>;
def : StPat<truncstorei16, STH_SO>;
def : StPat<store, STW_SO>;

//===----------------------------------------------------------------------===//
// Move instructions
//===----------------------------------------------------------------------===//
let isMoveImm = 1 in {

  def MOV_RLC
      : TCInstRLC<0x3B, (outs DGPR:$c),
                  (ins simm16:$const16), [(set DGPR:$c, simm16:$const16)],
                  "mov", "$c, $const16"> {
    let a = 0;
  }
  def MOVE_RLC
      : TCInstRLC<0xFB, (outs EGPR:$c),
                  (ins simm16_64:$const16), [(set EGPR:$c, simm16_64:$const16)],
                  "mov", "$c, $const16"> {
    let a = 0;
  }
  def MOVHA_RLC : TCInstRLC<0x91, (outs AGPR:$c), (ins uimm16:$const16), [],
                            "movh.a", "$c, $const16"> {
    let a = 0;
  }
  def LEA_BOL
      : TCInstBOL<0xD9, (outs AGPR:$a), (ins AGPR:$b, uimm16:$off16), [], "lea",
                  "$a, $b, $off16">;
}

let isMoveReg = 1 in {

  def MOV_RR : TCInstRR<0x1F, 0x0B, (outs DGPR:$c), (ins DGPR:$b), [], "mov",
                        "$c, $b"> {
    let a = 0;
  }

  def MOVH_RLC
      : TCInstRLC<0x7B, (outs DGPR:$c),
                  (ins simm16:$const16), [(set DGPR:$c, lo16AllZero:$const16)],
                  "movh", "$c, $const16"> {
    let a = 0;
  }

  def MOVD_RR : TCInstRR<0x01, 0x4C, (outs DGPR:$c), (ins AGPR:$b), [], "mov.d",
                         "$c, $b"> {
    let a = 0;
  }
  def MOVD_SRR
      : TCInstSRR<0x80, (outs DGPR:$a), (ins AGPR:$b), "mov.d", "$a, $b">;

  def MOVA_RR : TCInstRR<0x01, 0x63, (outs AGPR:$c), (ins DGPR:$b), [], "mov.a",
                         "$c, $b"> {
    let a = 0;
  }
  def MOVA_SRC : TCInstSRC<0xA0, (outs AGPR:$a), (ins uimm4:$const4), "mov.a",
                           "$a, $const4">;
  def MOVA_SRR
      : TCInstSRR<0x60, (outs AGPR:$a), (ins DGPR:$b), "mov.a", "$a, $b">;

  def MOVAA_RR : TCInstRR<0x01, 0x00, (outs AGPR:$c), (ins AGPR:$b), [],
                          "mov.aa", "$c, $b"> {
    let a = 0;
  }
  def MOVAA_SRR
      : TCInstSRR<0x40, (outs AGPR:$a), (ins AGPR:$b), "mov.aa", "$a, $b">;
}

def : Pat<(tricore_movha tglobaladdr:$in), (MOVHA_RLC tglobaladdr:$in)>;
def : Pat<(tricore_movha tblockaddress:$in), (MOVHA_RLC tblockaddress:$in)>;
def : Pat<(tricore_movha tjumptable:$in), (MOVHA_RLC tjumptable:$in)>;
def : Pat<(tricore_movha tconstpool:$in), (MOVHA_RLC tconstpool:$in)>;

def : Pat<(tricore_lea AGPR:$hi, tglobaladdr:$lo), (LEA_BOL AGPR:$hi,
                                                       tglobaladdr:$lo)>;
def : Pat<(tricore_lea AGPR:$hi, tblockaddress:$lo), (LEA_BOL AGPR:$hi,
                                                         tblockaddress:$lo)>;
def : Pat<(tricore_lea AGPR:$hi, tjumptable:$lo), (LEA_BOL AGPR:$hi,
                                                      tjumptable:$lo)>;
def : Pat<(tricore_lea AGPR:$hi, tconstpool:$lo), (LEA_BOL AGPR:$hi,
                                                      tconstpool:$lo)>;

//===----------------------------------------------------------------------===//
//  Add instructions
//===----------------------------------------------------------------------===//

multiclass arithmetic<int Op1, int Op2, SDNode Op, string opcodestr,
                      ValueType Ty = i32> {
  def _RC : TCInstRC<!add(Op1, 0x80), Op2, (outs DGPR:$c),
                     (ins DGPR:$a, simm9:$const9),
                     [(set DGPR:$c, (Op(Ty DGPR:$a), simm9:$const9))],
                     opcodestr, "$c, $a, $const9">;
  def _RR : TCInstRR<Op1, Op2, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b),
                     [(set DGPR:$c, (Op(Ty DGPR:$a), (Ty DGPR:$b)))], opcodestr,
                     "$c, $a, $b">;
}

multiclass arithmetic_notin<int Op1, int Op2, SDNode Op, string opcodestr,
                            ValueType Ty = i32> {
  def _RC : TCInstRC<!add(Op1, 0x80), Op2, (outs DGPR:$c),
                     (ins DGPR:$a, simm9:$const9), [], opcodestr,
                     "$c, $a, $const9">;
  def _RR : TCInstRR<
                Op1, Op2, (outs DGPR:$c),
                (ins DGPR:$a, DGPR:$b), [(set Ty:$c, (Op Ty:$a, (ineg Ty:$b)))],
                opcodestr, "$c, $a, $b">;
  def : Pat<(Op DGPR:$a, (ineg simm9:$const9)),
            (!cast<Instruction>(NAME#_RC)(Ty DGPR:$a), (negate simm9:$const9))>;
}

multiclass arithmetic_notout<int Op1, int Op2, SDNode Op, string opcodestr,
                             ValueType Ty = i32> {
  def _RC : TCInstRC<!add(Op1, 0x80), Op2, (outs DGPR:$c),
                     (ins DGPR:$a, simm9:$const9),
                     [(set Ty:$c, (not(Op Ty:$a, Ty:$const9)))], opcodestr,
                     "$c, $a, $const9">;
  def _RR
      : TCInstRR<Op1, Op2, (outs DGPR:$c),
                 (ins DGPR:$a, DGPR:$b), [(set Ty:$c, (not(Op Ty:$a, Ty:$b)))],
                 opcodestr, "$c, $a, $b">;
}

multiclass arithmetic_cond<int op1, int op2, CondCode cond, string asmstring> {
  def _RC
      : TCInstRC<
            !add(op1, 0x80), op2, (outs DGPR:$c), (ins DGPR:$a, simm9:$const9),
            [(set DGPR:$c, (i32(setcc(i32 DGPR:$a), simm9:$const9, cond)))],
            asmstring, "$c, $a, $const9">;
  def _RR : TCInstRR<op1, op2, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b),
                     [(set DGPR:$c, (i32(setcc(i32 DGPR:$a), (i32 DGPR:$b),
                                        cond)))],
                     asmstring, "$c, $a, $b">;
}

multiclass cond_inferred<CondCode cond, Instruction inst, Instruction instb,
                         string asmstring> {
  def : Pat<(i32(setcc(i32 DGPR:$a), simm9:$const9, cond)),
            (inst DGPR:$a, (plus1 simm9:$const9))>;
  def : Pat<(i32(setcc(i32 DGPR:$a), DGPR:$b, cond)), (instb DGPR:$b, DGPR:$a)>;
}

defm ADD : arithmetic<0x0B, 0x0, add, "add">;
defm ADDX : arithmetic<0x0B, 0x4, adde, "addx">;
defm ADDC : arithmetic<0x0B, 0x5, addc, "addc">;
defm EQ : arithmetic_cond<0x0B, 0x10, SETEQ, "eq">;
defm NE : arithmetic_cond<0x0B, 0x11, SETNE, "ne">;
defm LT : arithmetic_cond<0xB, 0x12, SETLT, "lt">;
defm LTU : arithmetic_cond<0x0B, 0x13, SETULT, "lt.u">;
defm GE : arithmetic_cond<0xB, 0x14, SETGE, "ge">;
defm GEU : arithmetic_cond<0xB, 0x15, SETUGE, "ge.u">;
defm ABSDIF : arithmetic<0x0B, 0xE, abds, "absdif">;
// TODO: abdu
defm GT : cond_inferred<SETGT, GE_RC, LT_RR, "gt">;
defm GTU : cond_inferred<SETUGT, GEU_RC, LTU_RR, "gt.u">;
defm LE : cond_inferred<SETLE, LT_RC, GE_RR, "le">;
defm LEU : cond_inferred<SETULE, LTU_RC, GEU_RR, "le.u">;

defm SH : arithmetic<0x0F, 0x0, shl, "sh">;
// sha 0x1
// shas 0x2
defm AND : arithmetic<0x0F, 0x8, and, "and">;
defm NAND : arithmetic_notout<0x0F, 0x9, and, "nand">;
defm OR : arithmetic<0x0F, 0xA, or, "or">;
defm NOR : arithmetic_notout<0x0F, 0xB, or, "nor">;
defm XOR : arithmetic<0x0F, 0xC, xor, "xor">;
defm XNOR : arithmetic_notout<0x0F, 0xD, xor, "xnor">;
defm ANDN : arithmetic_notin<0x0F, 0xE, and, "andn">;
defm ORN : arithmetic_notin<0x0F, 0xF, or, "orn">;
// TODO Pat: shr, srl, not

def : Pat<(not(i32 DGPR:$a)), (NOR_RC DGPR:$a, 0)>;
def : Pat<(add(i64 EGPR:$a), (i64 EGPR:$b)),
          (INSERT_SUBREG(INSERT_SUBREG(i64(IMPLICIT_DEF)),
               (i32(ADDX_RR(EXTRACT_SUBREG EGPR:$a, sub_even),
                   (EXTRACT_SUBREG EGPR:$b, sub_even))),
               sub_even),
              (i32(ADDC_RR(i32(EXTRACT_SUBREG EGPR:$a, sub_odd)),
                  (EXTRACT_SUBREG EGPR:$b, sub_odd))),
              sub_odd

          )>;
def : Pat<(add(i64 EGPR:$a), simm9_64:$const9),
          (i64(INSERT_SUBREG(INSERT_SUBREG(i64(IMPLICIT_DEF)),
               (i32(ADDX_RC(i32(EXTRACT_SUBREG EGPR:$a, sub_even)),
                   (lo16 simm9_64:$const9))),
               sub_even),
              (i32(ADDC_RC(i32(EXTRACT_SUBREG EGPR:$a, sub_odd)),
                  (i32 0))),
              sub_odd

          ))>;
// def : Pat<(i64(add EGPR:$a, i64:$const9)),
//           (i64(SUBREG_TO_REG (i32 0),(i32(ADDC_RR(i32(EXTRACT_SUBREG(i64
//           EGPR:$a), sub_odd)),
//                    (i32(ADDX_RC(i32(EXTRACT_SUBREG(i64 EGPR:$a), sub_even)),
//                        (i32 i64:$const9))))),
//               sub_even))>;

def ADDIH : TCInstRLC<0x9B, (outs DGPR:$c), (ins DGPR:$a, simm16:$const16), [],
                      "addih", "$c, $a, $const16">;
def ADDI : TCInstRLC<0x1B, (outs DGPR:$c), (ins DGPR:$a, simm16:$const16),
                     [(set DGPR:$c, (add DGPR:$a, simm16:$const16))], "addi",
                     "$c, $a, $const16">;

// Transforms frameindex -> tframeindex.
def to_tframeindex : SDNodeXForm<frameindex, [{
  return CurDAG->getTargetFrameIndex(N->getIndex(), N->getValueType(0));
}]>;

def : GICustomOperandRenderer<"renderFrameIndex">,
      GISDNodeXFormEquiv<to_tframeindex>;

let Constraints = "$c = $a" in {
  def SUBA_SC : TCInstSC<0x20, (outs AGPR:$c), (ins AGPR:$a, uimm8:$const8),
                         "sub.a", "$c, $const8"> {
    let Uses = [A10];
    let Defs = [A10];
  }
}
def : Pat<(sub frameindex:$fi, uimm8:$const8),
          (SUBA_SC(iPTR(to_tframeindex $fi)), uimm8:$const8)>;

// Select

def SEL_RCR
    : TCInstRCR<0xAB, 0x4, (outs DGPR:$c),
                (ins DGPR:$d, DGPR:$a, simm9:$const9),
                [(set DGPR:$c, (select(i32 DGPR:$d), DGPR:$a, simm9:$const9))],
                "sel", "$c, $d, $a, $const9">;
def SEL_RRR
    : TCInstRRR<0x2B, 0x4, (outs DGPR:$c), (ins DGPR:$d, DGPR:$a, DGPR:$b),
                [(set DGPR:$c, (i32(select(i32 DGPR:$d), DGPR:$a, DGPR:$b)))],
                "sel", "$c, $d, $a, $b">;
// Branch conditional
multiclass cond_branch_unsigned<CondCode cond, CondCode condu, int op1,
                                string asmstring> {
  let isBranch = 1, isTerminator = 1 in {
    def _BRC
        : TCInstBRC<!add(op1, 0x80), 0x00, (outs),
                    (ins DGPR:$a, simm4:$const4, br_disp15:$disp15),
                    [(brcc cond, (i32 DGPR:$a), simm4:$const4, bb:$disp15)],
                    asmstring, "$a, $const4, $disp15">;
    def _BRR : TCInstBRR<op1, 0x00, (outs),
                         (ins DGPR:$a, DGPR:$b, br_disp15:$disp15),
                         [(brcc cond, (i32 DGPR:$a), DGPR:$b, bb:$disp15)],
                         asmstring, "$a, $b, $disp15">;
    def U_BRC
        : TCInstBRC<!add(op1, 0x80), 0x01, (outs),
                    (ins DGPR:$a, simm4:$const4, br_disp15:$disp15),
                    [(brcc condu, (i32 DGPR:$a), simm4:$const4, bb:$disp15)],
                    !strconcat(asmstring, ".u"), "$a, $const4, $disp15">;
    def U_BRR : TCInstBRR<op1, 0x01, (outs),
                          (ins DGPR:$a, DGPR:$b, br_disp15:$disp15),
                          [(brcc condu, (i32 DGPR:$a), DGPR:$b, bb:$disp15)],
                          !strconcat(asmstring, ".u"), "$a, $b, $disp15">;
  }
}

multiclass cond_branch<CondCode cond, int op1, int op2, int op1_16,
                       string asmstring> {
  let isBranch = 1, isTerminator = 1 in {
    def _BRC
        : TCInstBRC<!add(op1, 0x80), op2, (outs),
                    (ins DGPR:$a, simm4:$const4, br_disp15:$disp15),
                    [(brcc cond, (i32 DGPR:$a), simm4:$const4, bb:$disp15)],
                    asmstring, "$a, $const4, $disp15">;
    def _BRR
        : TCInstBRR<op1, op2, (outs), (ins DGPR:$a, DGPR:$b, br_disp15:$disp15),
                    [(brcc cond, (i32 DGPR:$a), DGPR:$b, bb:$disp15)],
                    asmstring, "$a, $b, $disp15">;

    def _SBC : TCInstSBC<op1_16, (outs), (ins simm4:$const4, br_disp4:$disp4),
                         asmstring, "d15, $const4, $disp4">;
    def _SBR
        : TCInstSBR<!add(op1_16, 0x20), (outs), (ins DGPR:$b, br_disp4:$disp4),
                    asmstring, "d15, $b, $disp4">;
  }
  def : CompressPat<(NAME#_BRC D15, simm4:$const4, br_disp4:$disp4),
                    (NAME#_SBC simm4:$const4, br_disp4:$disp4)>;
  def : CompressPat<(NAME#_BRR D15, DGPR:$b, br_disp4:$disp4),
                    (NAME#_SBR DGPR:$b, br_disp4:$disp4)>;
  /* TODO: +16 disp4 instructions */
}

defm JLT : cond_branch_unsigned<SETLT, SETULT, 0x3F, "jlt">;
defm JGE : cond_branch_unsigned<SETGE, SETUGE, 0x7F, "jge">;
defm JEQ : cond_branch<SETEQ, 0x5f, 0x0, 0x1E, "jeq">;
defm JNE : cond_branch<SETNE, 0x5f, 0x1, 0x5E, "jne">;

let isBranch = 1, isTerminator = 1, isBarrier = 1 in {
  def J_B : TCInstB<0x1D, (outs), (ins br_disp24:$disp24), [(br bb:$disp24)],
                    "j", "$disp24">;
}

// Extract and Insert
let b = 0 in {

  def EXTR_RRPW : TCInstRRPW<0x37, 0x2, (outs DGPR:$c),
                             (ins DGPR:$a, pos:$pos, width:$width),
                             [(set DGPR:$c, (tricore_extr(i32 DGPR:$a),
                                                pos:$pos, width:$width))],
                             "extr", "$c, $a, $pos, $width">;
  def EXTRU_RRPW : TCInstRRPW<0x37, 0x3, (outs DGPR:$c),
                              (ins DGPR:$a, pos:$pos, width:$width),
                              [(set DGPR:$c, (tricore_extru(i32 DGPR:$a),
                                                 pos:$pos, width:$width))],
                              "extr.u", "$c, $a, $pos, $width">;
}

def INSERT_RCPW
    : TCInstRCPW<0xB7, 0x0, (outs DGPR:$c),
                 (ins DGPR:$a, uimm4:$const4, pos:$pos, width:$width),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), uimm4:$const4,
                                    pos:$pos, width:$width))],
                 "insert", "$c, $a, $const4, $pos, $width">;

def INSERT_RCRR
    : TCInstRCRR<0x97, 0x0, (outs DGPR:$c),
                 (ins DGPR:$a, uimm4:$const4, EGPR:$d),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), uimm4:$const4,
                                    (i32(extractelt(v2i32 EGPR:$d), 0)),
                                    (i32(extractelt(v2i32 EGPR:$d), 1))))],
                 "insert", "$c, $a, $const4, $d">;

def INSERT_RCRW
    : TCInstRCRW<0xD7, 0x0, (outs DGPR:$c),
                 (ins DGPR:$a, uimm4:$const4, DGPR:$d, width:$width),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), uimm4:$const4,
                                    (i32 DGPR:$d), width:$width))],
                 "insert", "$c, $a, $const4, $d, $width">;

def INSERT_RRPW
    : TCInstRRPW<0x37, 0x2, (outs DGPR:$c),
                 (ins DGPR:$a, DGPR:$b, pos:$pos, width:$width),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), (i32 DGPR:$b),
                                    pos:$pos, width:$width))],
                 "insert", "$c, $a, $b, $pos, $width">;
def INSERT_RRRR
    : TCInstRRRR<0x17, 0x0, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b, EGPR:$d), [],
                 "insert", "$c, $a, $b, $d">;
def INSERT_RRRW
    : TCInstRRRW<0x57, 0x0, (outs DGPR:$c),
                 (ins DGPR:$a, DGPR:$b, DGPR:$d, width:$width),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), (i32 DGPR:$b),
                                    (i32 DGPR:$d), width:$width))],
                 "insert", "$c, $a, $b, $d, $width">;

def : Pat<(int_tricore_insert DGPR:$a, uimm4:$const4, DGPR:$d0, DGPR:$d1),
          (INSERT_RCRR DGPR:$a, uimm4:$const4,
              (INSERT_SUBREG
                  (INSERT_SUBREG(v2i32(IMPLICIT_DEF)), DGPR:$d1, sub_odd),
                  DGPR:$d0, sub_even))>;
def : Pat<(int_tricore_insert DGPR:$a, DGPR:$b, DGPR:$d0, DGPR:$d1),
          (INSERT_RRRR DGPR:$a, DGPR:$b,
              (INSERT_SUBREG
                  (INSERT_SUBREG(v2i32(IMPLICIT_DEF)), DGPR:$d1, sub_odd),
                  DGPR:$d0, sub_even))>;

// Multiplication
def MUL_RC
    : TCInstRC<0x53, 0x01, (outs DGPR:$c), (ins DGPR:$a, simm9:$const9), [],
               "mul", "$c, $a, $const9">;
def : Pat<(i32(mul DGPR:$a, simm9:$const9)), (MUL_RC DGPR:$a, simm9:$const9)>;

//===----------------------------------------------------------------------===//
// Intrinsics
//===----------------------------------------------------------------------===//

def MTCR : TCInstRLC<0xCD, (outs), (ins tuimm16:$const16, DGPR:$a),
                     [(int_tricore_mtcr tuimm16:$const16, DGPR:$a)], "mtcr",
                     "$const16, $a"> {
  let c = 0;
}

//===----------------------------------------------------------------------===//
//  Pattern
//===----------------------------------------------------------------------===//

def : Pat<(i64 simm16_64:$imm), (MOVE_RLC simm16_64:$imm)>;
def : Pat<(i32 imm:$imm), (ADDIH(MOV_RLC(lo16 imm:$imm)), (hi16 imm:$imm))>;
def : Pat<(i32 uimm16:$imm), (MOV_RLC uimm16:$imm)>;
