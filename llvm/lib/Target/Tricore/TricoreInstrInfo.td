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

class TricoreOp<ValueType vt> : Operand<vt> {
  let OperandNamespace = "TricoreOp";
}

class TricoreSImmOp<int bitsNum> : TricoreOp<i32> {
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

class TricoreSImmLeafOp<int bitsNum>
    : TricoreSImmOp<bitsNum>, ImmLeaf<i32, "return isInt<"#bitsNum#">(Imm);">;
// def simm4 : TricoreSImmLeafOp<4>;
// def simm5 : TricoreSImmLeafOp<5>;
// def simm8 : TricoreSImmLeafOp<8>;
// def simm9 : TricoreSImmLeafOp<9>;
def simm10 : TricoreSImmLeafOp<10>;
def simm16 : TricoreSImmLeafOp<16>;

class TricoreUImmLeafOp<int bitsNum>
    : TricoreUImmOp<bitsNum>, ImmLeaf<i32, "return isUInt<"#bitsNum#">(Imm);">;
def uimm4 : TricoreUImmLeafOp<4>;
def uimm8 : TricoreUImmLeafOp<8>;
def uimm16 : TricoreUImmLeafOp<16>;

def tuimm16 : TricoreUImmOp<16>, TImmLeaf<i32, "return isUInt<16>(Imm);">;

class TricoreDispImmLeafOp<int bitsNum>
    : TricoreDispImmOp<bitsNum>,
      ImmLeaf<i32, "return isShiftedInt<"#bitsNum#", 1>(Imm);">;

def disp24 : TricoreDispImmLeafOp<24>;

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
def MOV_RLC : TCInstRLC<0x3B, (outs DGPR:$c),
                        (ins simm16:$const16), [(set DGPR:$c, simm16:$const16)],
                        "mov", "$c, $const16"> {
  let a = 0;
}
def MOV_RR
    : TCInstRR<0x1F, 0x0B, (outs DGPR:$c), (ins DGPR:$b), [], "mov", "$c, $b"> {
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
    : TCInstSRR<0x80, (outs DGPR:$a), (ins AGPR:$b), [], "mov.d", "$a, $b">;

def MOVA_RR : TCInstRR<0x01, 0x63, (outs AGPR:$c), (ins DGPR:$b), [], "mov.a",
                       "$c, $b"> {
  let a = 0;
}
def MOVA_SRC : TCInstSRC<0xA0, (outs AGPR:$a),
                         (ins uimm4:$const4), [(set AGPR:$a, uimm4:$const4)],
                         "mov.a", "$a, $const4">;
def MOVA_SRR
    : TCInstSRR<0x60, (outs AGPR:$a), (ins DGPR:$b), [], "mov.a", "$a, $b">;

def MOVAA_RR : TCInstRR<0x01, 0x00, (outs AGPR:$c), (ins AGPR:$b), [], "mov.aa",
                        "$c, $b"> {
  let a = 0;
}
def MOVAA_SRR
    : TCInstSRR<0x40, (outs AGPR:$a), (ins AGPR:$b), [], "mov.aa", "$a, $b">;

def MOVHA_RLC : TCInstRLC<0x91, (outs AGPR:$c), (ins uimm16:$const16), [],
                          "movh.a", "$c, $const16"> {
  let a = 0;
}
def LEA_BOL : TCInstBOL<0xD9, (outs AGPR:$a), (ins AGPR:$b, uimm16:$off16), [],
                        "lea", "$a, $b, $off16">;

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

class Arit_RR<bits<8> op2, string opcodestr, SDNode Op, ValueType Ty>
    : TCInstRR<0x0B, op2, (outs DGPR:$c),
               (ins DGPR:$a, DGPR:$b), [(set Ty:$c, (Op Ty:$a, Ty:$b))],
               opcodestr, "$c, $a, $b">;
def ADD_RR : Arit_RR<0x0, "add", add, i32>;

def addih : TCInstRLC<0x9B, (outs DGPR:$c), (ins DGPR:$a, simm16:$const16), [],
                      "addih", "$c, $a, $const16">;
def addi : TCInstRLC<0x1B, (outs DGPR:$c), (ins DGPR:$a, simm16:$const16),
                     [(set DGPR:$c, (add DGPR:$a, simm16:$const16))], "addi",
                     "$c, $a, $const16">;

// Transforms frameindex -> tframeindex.
def to_tframeindex : SDNodeXForm<frameindex, [{
  return CurDAG->getTargetFrameIndex(N->getIndex(), N->getValueType(0));
}]>;

def : GICustomOperandRenderer<"renderFrameIndex">,
      GISDNodeXFormEquiv<to_tframeindex>;

let Constraints = "$c = $a" in {
  def SUBA_SC : TCInstSC<0x20, (outs AGPR:$c), (ins AGPR:$a, uimm8:$const8), [],
                         "sub.a", "$c, $const8"> {
    let Uses = [A10];
    let Defs = [A10];
  }
}
def : Pat<(sub frameindex:$fi, uimm8:$const8),
          (SUBA_SC(iPTR(to_tframeindex $fi)), uimm8:$const8)>;

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

def : Pat<(i32 imm:$imm), (addih(MOV_RLC(lo16 imm:$imm)), (hi16 imm:$imm))>;
def : Pat<(i32 uimm16:$imm), (MOV_RLC uimm16:$imm)>;
