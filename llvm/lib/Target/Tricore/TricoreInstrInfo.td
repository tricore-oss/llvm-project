include "TricoreInstrFormat.td"

//===----------------------------------------------------------------------===//
// Instruction Pattern Stuff
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
def uimm8 : TricoreUImmLeafOp<8>;
def tuimm16 : TricoreUImmOp<16>, TImmLeaf<i32, "return isUInt<16>(Imm);">;


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

class MemOperand<RegisterClass regClass> : RegisterOperand<regClass> {
  let OperandType = "OPERAND_MEMORY";
}
def AGPRMem : MemOperand<AGPR>;

def AddrRegImm : ComplexPattern<iPTR, 2, "SelectAddrRegImm">;

// Target-independent type requirements, but with target-specific formats.
def SDT_CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_CallSeqEnd : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

// Target-independent nodes, but with target-specific formats.
def callseq_start : SDNode<"ISD::CALLSEQ_START",
                           SDT_CallSeqStart, [SDNPHasChain, SDNPOutGlue]>;
def callseq_end
    : SDNode<"ISD::CALLSEQ_END",
             SDT_CallSeqEnd, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

// Target-dependent nodes.
def tricore_call
    : SDNode<"TricoreISD::CALL",
             SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;
def tricore_ret_glue
    : SDNode<"TricoreISD::RET_GLUE",
             SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
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
  class Load_so<bits<6> op2, string opcodestr>
      : TCInstBO<0x09, op2, (outs DGPR:$a), (ins AGPRMem:$b, simm10:$imm), [],
                 opcodestr, "$a, [$b]$imm">;
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in {
  class Store_so<bits<6> op2, string opcodestr>
      : TCInstBO<0x89, op2, (outs), (ins DGPR:$a, AGPRMem:$b, simm10:$imm), [],
                 opcodestr, "[$b]$imm, $a">;
}

//===----------------------------------------------------------------------===//
//  Load/Store instructions
//===----------------------------------------------------------------------===//

def LDB_SO : Load_so<0x20, "ld.b">;
def LDH_SO : Load_so<0x22, "ld.h">;
def LDW_SO : Load_so<0x24, "ld.w">;
def LDD_SO : Load_so<0x25, "ld.d">;
def LDA_SO : Load_so<0x26, "ld.a">;

def STB_SO : Store_so<0x20, "st.b">;
def STH_SO : Store_so<0x22, "st.h">;
def STW_SO : Store_so<0x24, "st.w">;
def STD_SO : Store_so<0x25, "st.d">;
def STA_SO : Store_so<0x26, "st.a">;

//===----------------------------------------------------------------------===//
// Move instructions
//===----------------------------------------------------------------------===//
def mov_data : TCInstRLC<0x3B, (outs DGPR:$c),
                         (ins simm16:$imm), [(set DGPR:$c, simm16:$imm)], "mov",
                         "$c, $imm">;

def movh32 : TCInstRLC<0x7B, (outs DGPR:$c),
                       (ins simm16:$imm), [(set DGPR:$c, lo16AllZero:$imm)],
                       "movh", "$c, $imm">;

//===----------------------------------------------------------------------===//
//  Add instructions
//===----------------------------------------------------------------------===//

def addih : TCInstRLC<0x9B, (outs DGPR:$c), (ins DGPR:$a, simm16:$imm), [],
                      "addih", "$c, $a, $imm">;
def addi : TCInstRLC<0x1B, (outs DGPR:$c), (ins DGPR:$a, simm16:$imm),
                     [(set DGPR:$c, (add DGPR:$a, simm16:$imm))], "addi",
                     "$c, $a, $imm">;

// Transforms frameindex -> tframeindex.
def to_tframeindex : SDNodeXForm<frameindex, [{
  return CurDAG->getTargetFrameIndex(N->getIndex(), N->getValueType(0));
}]>;

def : GICustomOperandRenderer<"renderFrameIndex">,
      GISDNodeXFormEquiv<to_tframeindex>;

let Constraints = "$c = $a" in {
  def SUBA_SC : TCInstSC<0x20, (outs AGPR:$c), (ins AGPR:$a, uimm8:$const8),
                         [],
                         "sub.a", "$c, $const8">;
}
def : Pat<(sub frameindex:$fi, uimm8:$const8),
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

def : Pat<(i32 imm:$imm), (addih(mov_data(lo16 imm:$imm)), (hi16 imm:$imm))>;

// Load
class LdPat<PatFrag LoadOp, TCInstCommon Inst>
    : Pat<(i32(LoadOp(AddrRegImm(i32 AGPR:$b), simm10:$off10))),
          (Inst AGPR:$b, simm10:$off10)>;

def : LdPat<sextloadi8, LDB_SO>;
def : LdPat<load, LDW_SO>;

// Store
/// Stores

class StPat<PatFrag StoreOp, TCInstCommon Inst, RegisterClass StTy,
            ValueType vt>
    : Pat<(StoreOp(vt StTy:$a), (AddrRegImm(i32 AGPR:$b), simm10:$off10)),
          (Inst StTy:$a, AGPR:$b, simm10:$off10)>;

def : StPat<truncstorei8, STB_SO, DGPR, i32>;
def : StPat<truncstorei16, STH_SO, DGPR, i32>;
def : StPat<store, STW_SO, DGPR, i32>;