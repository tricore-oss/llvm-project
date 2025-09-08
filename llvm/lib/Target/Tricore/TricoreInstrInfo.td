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

class TricoreUImmOp<int bitsNum, ValueType Vt = i32> : TricoreOp<Vt> {
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

class TricoreDispImmOp<int bitsNum> : Operand<i32> {
  let ParserMatchClass = DispImmAsmOperand<bitsNum>;
  let EncoderMethod = "getDispImmOpValue";
  let DecoderMethod = "decodeDispImmOperand<"#bitsNum#">";
  let PrintMethod = "printBranchOperand";
  let OperandType = "OPERAND_PCREL";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<}]#bitsNum#[{, 1>(Imm);
    return MCOp.isBareSymbolRef();
  }];
}

class TricoreBrDispImmOp<int bitsNum> : Operand<OtherVT> {
  let ParserMatchClass = DispImmAsmOperand<bitsNum>;
  let EncoderMethod = "getDispImmOpValue";
  let DecoderMethod = "decodeDispImmOperand<"#bitsNum#">";
  let PrintMethod = "printBranchOperand";
  let OperandType = "OPERAND_PCREL";
  let MCOperandPredicate = [{
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<}]#bitsNum#[{, 1>(Imm);
    return MCOp.isBareSymbolRef();
  }];
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
def uimm9 : TricoreUImmLeafOp<9>;
def uimm16 : TricoreUImmLeafOp<16>;

def uimm9_64 : TricoreUImmOp<9, i64>, ImmLeaf<i64, "return isUInt<9>(Imm);">;

def tuimm16 : TricoreUImmOp<16>, TImmLeaf<i32, "return isUInt<16>(Imm);">;

class TricoreDispImmLeafOp<int bitsNum>
    : TricoreDispImmOp<bitsNum>,
      ImmLeaf<i32, "return isShiftedInt<"#bitsNum#", 1>(Imm);">;

def disp24 : TricoreDispImmLeafOp<24>;
def br_disp15 : TricoreBrDispImmOp<15>;
def br_disp24 : TricoreBrDispImmOp<24>;

class MemAsmOperand<int bitsNum, IndexMode mode> : AsmOperandClass {
  let Name = "Mem"#bitsNum#"_"#mode.Value;
  let RenderMethod = "addMemOperands";
  let ParserMethod = "parseMemOperand";
  let PredicateMethod = "isMemWithSimmOffset<"#bitsNum#","#mode.Value#">";
  let DiagnosticType = "InvalidMem";
}

class MemBO<IndexMode mode> : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops AGPR, simm10);
  let ParserMatchClass = MemAsmOperand<10, mode>;
  let OperandType = "OPERAND_MEMORY";
}

def MemBOL : Operand<i32> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops AGPR, simm16);
  let ParserMatchClass = MemAsmOperand<16, IndexModeNone>;
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
  return CurDAG->getTargetConstant((uint32_t)(N->getZExtValue() + 1), SDLoc(N),
                                   MVT::i32);
}]>;
def negate : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((uint32_t)(0 - N->getZExtValue()), SDLoc(N),
                                   MVT::i32);
}]>;

def lo16AllZero : PatLeaf<(i32 imm), [{
  // Returns true if all low 16-bits are 0.
  return (((uint32_t)N->getZExtValue()) & 0xFFFFUL) == 0;
}],
                          hi16>;

def AddrRegImmBO : ComplexPattern<iPTR, 2, "SelectAddrRegImmBO">;
def AddrRegImmBOL : ComplexPattern<iPTR, 2, "SelectAddrRegImmBOL">;

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

def SDT_CMPF
    : SDTypeProfile<1, 2, [SDTCisInt<0>, SDTCisFP<1>, SDTCisSameAs<1, 2>]>;
def SDT_BIT
    : SDTypeProfile<1, 4,
                    [SDTCisInt<0>, SDTCisSameAs<0, 1>, SDTCisSameAs<0, 2>,
                     SDTCisSameAs<0, 3>, SDTCisSameAs<0, 4>]>;

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

def tricore_cmp_f : SDNode<"TricoreISD::CMP_F", SDT_CMPF>;

def tricore_and_t : SDNode<"TricoreISD::AND_T", SDT_BIT>;
def tricore_or_t : SDNode<"TricoreISD::OR_T", SDT_BIT>;
def tricore_nor_t : SDNode<"TricoreISD::NOR_T", SDT_BIT>;
def tricore_andn_t : SDNode<"TricoreISD::ANDN_T", SDT_BIT>;
def tricore_nand_t : SDNode<"TricoreISD::NAND_T", SDT_BIT>;
def tricore_orn_t : SDNode<"TricoreISD::ORN_T", SDT_BIT>;
def tricore_xnor_t : SDNode<"TricoreISD::XNOR_T", SDT_BIT>;
def tricore_xor_t : SDNode<"TricoreISD::XOR_T", SDT_BIT>;

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//
class InstRC<bits<8> op1, bits<7> op2, string opcodestr,
             list<dag> patterns = []>
    : TCInstRC<op1, op2, (outs DGPR:$c), (ins DGPR:$a, simm9:$const9), patterns,
               opcodestr, "$c, $a, $const9">;
class InstRR<bits<8> op1, bits<8> op2, string opcodestr,
             list<dag> patterns = []>
    : TCInstRR<op1, op2, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b), patterns,
               opcodestr, "$c, $a, $b">;

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

let isBarrier = 1, isReturn = 1, isTerminator = 1 in {
  def RET
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

//===----------------------------------------------------------------------===//
//  Load/Store instructions
//===----------------------------------------------------------------------===//

class LoadBO<int op2, IndexMode mode, string opcodestr, RegisterClass RC = DGPR>
    : TCInstBO<0x9, op2, mode, (outs RC:$a),
               (ins(MemBO<mode> $b, $off10):$addr), [], opcodestr, "$a, $addr">;
class LoadBOL<int op1, string opcodestr, RegisterClass RC = DGPR>
    : TCInstBOL<op1, (outs RC:$a), (ins(MemBOL $b, $off16):$addr), [],
                opcodestr, "$a, $addr">;

multiclass Load<int bo_op2, int bol_op1, string opcodestr,
                RegisterClass RC = DGPR> {
  let hasSideEffects = 0, mayLoad = 1, mayStore = 0 in {
    def post : LoadBO<bo_op2, IndexModePost, opcodestr, RC>;
    def pre : LoadBO<bo_op2, IndexModePre, opcodestr, RC>;
    def bo : LoadBO<bo_op2, IndexModeNone, opcodestr, RC>;
    if bol_op1 then {
      def bol : LoadBOL<bol_op1, opcodestr, RC>;
    }
  }
}

let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in {
  class Store_so<bits<4> op2, string opcodestr, RegisterClass RC = DGPR>
      : TCInstBO<0x89, op2, IndexModeNone, (outs),
                 (ins(MemBO<IndexModeNone> $b, $off10):$addr, RC:$a), [],
                 opcodestr, "$addr, $a">;
}

defm LDB : Load<0x0, 0x79, "ld.b">;
defm LDBU : Load<0x1, 0x39, "ld.bu">;
defm LDH : Load<0x2, 0xC9, "ld.h">;
defm LDHU : Load<0x3, 0xB9, "ld.hu">;
defm LDW : Load<0x4, 0x19, "ld.w">;
defm LDD : Load<0x5, 0x00, "ld.d", EGPR>;
defm LDA : Load<0x6, 0x99, "ld.a", AGPR>;
defm LDDA : Load<0x7, 0x00, "ld.d", PGPR>;
// defm LDDD : Load<0x9, 0x0, "ld.d", QGPR>;

// Load
multiclass LdPat<PatFrag LoadOp, string InstStr, ValueType vt = i32> {
  def : Pat<(vt(LoadOp AddrRegImmBOL:$addr)),
            (!cast<Instruction>(InstStr#bol) AddrRegImmBOL:$addr)>;
  def : Pat<(vt(LoadOp AddrRegImmBO:$addr)),
            (!cast<Instruction>(InstStr#bo) AddrRegImmBO:$addr)>;
}

defm : LdPat<extloadi1, "LDB">;
defm : LdPat<zextloadi1, "LDB">;
defm : LdPat<extloadi8, "LDB">;
defm : LdPat<sextloadi8, "LDB">;
defm : LdPat<zextloadi8, "LDBU">;
defm : LdPat<extloadi16, "LDH">;
defm : LdPat<sextloadi16, "LDH">;
defm : LdPat<zextloadi16, "LDHU">;
defm : LdPat<load, "LDW">;
defm : LdPat<load, "LDW", f32>;
def : Pat<(i64(load AddrRegImmBO:$addr)), (LDDbo AddrRegImmBO:$addr)>;
def : Pat<(f64(load AddrRegImmBO:$addr)), (LDDbo AddrRegImmBO:$addr)>;
def : Pat<(v2i32(load AddrRegImmBO:$addr)), (LDDbo AddrRegImmBO:$addr)>;
defm : LdPat<atomic_load_8, "LDB">;
defm : LdPat<atomic_load_16, "LDB">;
defm : LdPat<atomic_load_32, "LDW">;
def : Pat<(i64(atomic_load_64 AddrRegImmBO:$addr)), (LDDbo AddrRegImmBO:$addr)>;

// Store
class StoreBO<int op2, IndexMode mode, string opcodestr,
              RegisterClass RC = DGPR>
    : TCInstBO<0x89, op2, mode, (outs),
               (ins(MemBO<mode> $b, $off10):$addr, RC:$a), [], opcodestr,
               "$addr, $a">;
class StoreBOL<int op1, string opcodestr, RegisterClass RC = DGPR>
    : TCInstBOL<op1, (outs), (ins(MemBOL $b, $off16):$addr, RC:$a), [],
                opcodestr, "$addr, $a">;

multiclass Store<int bo_op2, int bol_op1, string opcodestr,
                 RegisterClass RC = DGPR> {
  let hasSideEffects = 0, mayLoad = 0, mayStore = 1 in {
    def post : StoreBO<bo_op2, IndexModePost, opcodestr, RC>;
    def pre : StoreBO<bo_op2, IndexModePre, opcodestr, RC>;
    def bo : StoreBO<bo_op2, IndexModeNone, opcodestr, RC>;
    if bol_op1 then {
      def bol : StoreBOL<bol_op1, opcodestr, RC>;
    }
  }
}

defm STB : Store<0x0, 0xE9, "st.b">;
defm STH : Store<0x2, 0xF9, "st.h">;
defm STW : Store<0x4, 0x59, "st.w">;
defm STD : Store<0x5, 0x0, "st.d", EGPR>;
defm STA : Store<0x6, 0xB5, "st.a", AGPR>;

multiclass StPat<PatFrag StoreOp, string InstStr, ValueType vt = i32> {
  def : Pat<(StoreOp(vt DGPR:$a), AddrRegImmBOL:$addr),
            (!cast<Instruction>(InstStr#bol) AddrRegImmBOL:$addr, DGPR:$a)>;
  def : Pat<(StoreOp(vt DGPR:$a), AddrRegImmBO:$addr),
            (!cast<Instruction>(InstStr#bo) AddrRegImmBO:$addr, DGPR:$a)>;
}

def : Pat<(store frameindex:$a, AddrRegImmBO:$addr), (STAbo AddrRegImmBO:$addr,
                                                         tframeindex:$a)>;
defm : StPat<truncstorei8, "STB">;
defm : StPat<truncstorei16, "STH">;
defm : StPat<store, "STW">;
defm : StPat<store, "STW", f32>;
def : Pat<(store(i64 EGPR:$a), AddrRegImmBO:$addr), (STDbo AddrRegImmBO:$addr,
                                                        EGPR:$a)>;
def : Pat<(store(f64 EGPR:$a), AddrRegImmBO:$addr), (STDbo AddrRegImmBO:$addr,
                                                        EGPR:$a)>;
def : Pat<(store(v2i32 EGPR:$a), AddrRegImmBO:$addr), (STDbo AddrRegImmBO:$addr,
                                                          EGPR:$a)>;

//===----------------------------------------------------------------------===//
// Move instructions
//===----------------------------------------------------------------------===//
let isMoveImm = 1 in {

  def MOVrlc
      : TCInstRLC<0x3B, (outs DGPR:$c),
                  (ins simm16:$const16), [(set DGPR:$c, simm16:$const16)],
                  "mov",
                  "$c, "
                  "$const16"> {
    let a = 0;
  }
  def MOVErlc
      : TCInstRLC<0xFB, (outs EGPR:$c),
                  (ins simm16_64:$const16), [(set EGPR:$c, simm16_64:$const16)],
                  "mov",
                  "$c, "
                  "$const16"> {
    let a = 0;
  }
  def MOVHArlc
      : TCInstRLC<0x91, (outs AGPR:$c), (ins uimm16:$const16), [], "movh.a",
                  "$c, "
                  "$const16"> {
    let a = 0;
  }
}

let isMoveReg = 1 in {

  def MOVrr : TCInstRR<0x1F, 0x0B, (outs DGPR:$c), (ins DGPR:$b), [], "mov",
                       "$c, $b"> {
    let a = 0;
  }
  def MOVEDrr : TCInstRR<0x0B, 0x80, (outs EGPR:$c), (ins DGPR:$b), [], "mov",
                         "$c, $b"> {
    let a = 0;
  }
  def MOVEDDrr
      : TCInstRR<0x0B, 0x81, (outs EGPR:$c), (ins DGPR:$a, DGPR:$b), [], "mov",
                 "$c, $a, "
                 "$b">;

  def MOVHrlc
      : TCInstRLC<0x7B, (outs DGPR:$c),
                  (ins simm16:$const16), [(set DGPR:$c, lo16AllZero:$const16)],
                  "movh",
                  "$c, "
                  "$const16"> {
    let a = 0;
  }

  def MOVDrr : TCInstRR<0x01, 0x4C, (outs DGPR:$c), (ins AGPR:$b), [], "mov.d",
                        "$c, $b"> {
    let a = 0;
  }
  def MOVD_SRR
      : TCInstSRR<0x80, (outs DGPR:$a), (ins AGPR:$b), "mov.d", "$a, $b">;

  def MOVArr : TCInstRR<0x01, 0x63, (outs AGPR:$c), (ins DGPR:$b), [], "mov.a",
                        "$c, $b"> {
    let a = 0;
  }
  def MOVA_SRC : TCInstSRC<0xA0, (outs AGPR:$a), (ins uimm4:$const4), "mov.a",
                           "$a, "
                           "$const4">;
  def MOVA_SRR
      : TCInstSRR<0x60, (outs AGPR:$a), (ins DGPR:$b), "mov.a", "$a, $b">;

  def MOVAArr : TCInstRR<0x01, 0x00, (outs AGPR:$c), (ins AGPR:$b), [],
                         "mov.aa", "$c, $b"> {
    let a = 0;
  }
  def MOVAA_SRR
      : TCInstSRR<0x40, (outs AGPR:$a), (ins AGPR:$b), "mov.aa", "$a, $b">;
}

def : Pat<(i64(sext(i32 DGPR:$b))), (MOVEDrr DGPR:$b)>;

//===--
// Address Instructions
//===--
def LEAbo : TCInstBO<0x49, 0x28, IndexModePost, (outs AGPR:$a),
                     (ins AGPR:$b, simm10:$off10), [], "lea",
                     "$a, $b, "
                     "$off10">;

def LEAbol
    : TCInstBOL<0xD9, (outs AGPR:$a), (ins AGPR:$b, simm16:$off16), [], "lea",
                "$a, $b, "
                "$off16">;

def copyFromSP : PatLeaf<(i32 AGPR:$src), [{
  return N->getOpcode() == ISD::CopyFromReg &&
         cast<RegisterSDNode>(N->getOperand(1))->getReg() == Tricore::A10;
}]>;
def : Pat<(add copyFromSP:$a, simm16:$off16), (LEAbol AGPR:$a, simm16:$off16)>;

def : Pat<(tricore_movha tglobaladdr:$in), (MOVHArlc tglobaladdr:$in)>;
def : Pat<(tricore_movha tblockaddress:$in), (MOVHArlc tblockaddress:$in)>;
def : Pat<(tricore_movha tjumptable:$in), (MOVHArlc tjumptable:$in)>;
def : Pat<(tricore_movha tconstpool:$in), (MOVHArlc tconstpool:$in)>;

def : Pat<(tricore_lea AGPR:$hi, tglobaladdr:$lo), (LEAbol AGPR:$hi,
                                                       tglobaladdr:$lo)>;
def : Pat<(tricore_lea AGPR:$hi, tblockaddress:$lo), (LEAbol AGPR:$hi,
                                                         tblockaddress:$lo)>;
def : Pat<(tricore_lea AGPR:$hi, tjumptable:$lo), (LEAbol AGPR:$hi,
                                                      tjumptable:$lo)>;
def : Pat<(tricore_lea AGPR:$hi, tconstpool:$lo), (LEAbol AGPR:$hi,
                                                      tconstpool:$lo)>;

//===----------------------------------------------------------------------===//
//  Add instructions
//===----------------------------------------------------------------------===//

class I32InstRRUnary<bits<8> Op1, bits<8> Op2, SDPatternOperator Op,
                     string Opcodestr>
    : TCInstRR<Op1, Op2, (outs DGPR:$c),
               (ins DGPR:$a), [(set DGPR:$c, (Op(i32 DGPR:$a)))], Opcodestr,
               "$c, $a"> {
  let b = 0;
}

multiclass arithmetic<int Op1, int Op2, SDNode Op, string opcodestr,
                      ValueType Ty = i32> {
  def rc : TCInstRC<!add(Op1, 0x80), Op2, (outs DGPR:$c),
                    (ins DGPR:$a, simm9:$const9),
                    [(set DGPR:$c, (Op(Ty DGPR:$a), simm9:$const9))], opcodestr,
                    "$c, $a, "
                    "$const9">;
  def rr : TCInstRR<Op1, Op2, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b),
                    [(set DGPR:$c, (Op(Ty DGPR:$a), (Ty DGPR:$b)))], opcodestr,
                    "$c, $a, "
                    "$b">;
}

multiclass arithmetic_notin<int Op1, int Op2, SDNode Op, string opcodestr,
                            ValueType Ty = i32> {
  def rc : TCInstRC<!add(Op1, 0x80), Op2, (outs DGPR:$c),
                    (ins DGPR:$a, simm9:$const9), [], opcodestr,
                    "$c, $a, "
                    "$const9">;
  def rr : TCInstRR<Op1, Op2, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b),
                    [(set Ty:$c, (Op Ty:$a, (ineg Ty:$b)))], opcodestr,
                    "$c, $a, "
                    "$b">;
  def : Pat<(Op DGPR:$a, (ineg simm9:$const9)),
            (!cast<Instruction>(NAME#rc)(Ty DGPR:$a), (negate simm9:$const9))>;
}

multiclass arithmetic_notout<int Op1, int Op2, SDNode Op, string opcodestr,
                             ValueType Ty = i32> {
  def rc
      : TCInstRC<
            !add(Op1, 0x80), Op2, (outs DGPR:$c), (ins DGPR:$a, simm9:$const9),
            [(set DGPR:$c, (not(Op(Ty DGPR:$a), simm9:$const9)))], opcodestr,
            "$c, $a, "
            "$const9">;
  def rr : TCInstRR<Op1, Op2, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b),
                    [(set DGPR:$c, (not(Op(Ty DGPR:$a), Ty:$b)))], opcodestr,
                    "$c, $a, "
                    "$b">;
}

multiclass arithmetic_cond<int op1, int op2, CondCode cond, string asmstring> {
  def rc
      : TCInstRC<
            !add(op1, 0x80), op2, (outs DGPR:$c), (ins DGPR:$a, simm9:$const9),
            [(set DGPR:$c, (i32(setcc(i32 DGPR:$a), simm9:$const9, cond)))],
            asmstring,
            "$c, $a, "
            "$const9">;
  def rr : TCInstRR<op1, op2, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b),
                    [(set DGPR:$c, (i32(setcc(i32 DGPR:$a), (i32 DGPR:$b),
                                       cond)))],
                    asmstring,
                    "$c, $a, "
                    "$b">;
}

multiclass cond_inferred<CondCode cond, Instruction inst, Instruction instb,
                         string asmstring> {
  def : Pat<(i32(setcc(i32 DGPR:$a), simm9:$const9, cond)),
            (inst DGPR:$a, (plus1 simm9:$const9))>;
  def : Pat<(i32(setcc(i32 DGPR:$a), DGPR:$b, cond)), (instb DGPR:$b, DGPR:$a)>;

  def rc : InstAlias<!strconcat(asmstring, "\t$c,$a, $const9"),
                  (inst DGPR:$c, DGPR:$a, simm9:$const9)>;
  def rr : InstAlias<!strconcat(asmstring, "\t$c, $a, $b"),
                  (instb DGPR:$c, DGPR:$b, DGPR:$a)>;
}

defm ADD : arithmetic<0x0B, 0x0, add, "add">;
defm ADDX : arithmetic<0x0B, 0x4, adde, "addx">;
defm ADDC : arithmetic<0x0B, 0x5, addc, "addc">;
defm EQ : arithmetic_cond<0x0B, 0x10, SETEQ, "eq">;
defm NE : arithmetic_cond<0x0B, 0x11, SETNE, "ne">;
defm LT : arithmetic_cond<0xB, 0x12, SETLT, "lt">;
defm LT_U : arithmetic_cond<0x0B, 0x13, SETULT, "lt.u">;
defm GE : arithmetic_cond<0xB, 0x14, SETGE, "ge">;
defm GE_U : arithmetic_cond<0xB, 0x15, SETUGE, "ge.u">;
defm ABSDIF : arithmetic<0x0B, 0xE, abds, "absdif">;
// TODO: abdu
defm GT : cond_inferred<SETGT, GErc, LTrr, "gt">;
defm GT_U : cond_inferred<SETUGT, GE_Urc, LT_Urr, "gt.u">;
defm LE : cond_inferred<SETLE, LTrc, GErr, "le">;
defm LE_U : cond_inferred<SETULE, LT_Urc, GE_Urr, "le.u">;

defm SH : arithmetic<0x0F, 0x0, shl, "sh">;
defm SHA : arithmetic<0xF, 0x1, shl, "sha">;
// shas 0x2
defm AND : arithmetic<0x0F, 0x8, and, "and">;
defm NAND : arithmetic_notout<0x0F, 0x9, and, "nand">;
defm OR : arithmetic<0x0F, 0xA, or, "or">;
defm NOR : arithmetic_notout<0x0F, 0xB, or, "nor">;
defm XOR : arithmetic<0x0F, 0xC, xor, "xor">;
defm XNOR : arithmetic_notout<0x0F, 0xD, xor, "xnor">;
defm ANDN : arithmetic_notin<0x0F, 0xE, and, "andn">;
defm ORN : arithmetic_notin<0x0F, 0xF, or, "orn">;

def ADDIH : TCInstRLC<0x9B, (outs DGPR:$c), (ins DGPR:$a, simm16:$const16), [],
                      "addih",
                      "$c, $a, "
                      "$const16">;
def ADDI : TCInstRLC<0x1B, (outs DGPR:$c), (ins DGPR:$a, simm16:$const16),
                     [(set DGPR:$c, (add DGPR:$a, simm16:$const16))], "addi",
                     "$c, $a, "
                     "$const16">;
def SUBrr
    : TCInstRR<
          0x0B, 0x08, (outs DGPR:$c),
          (ins DGPR:$a, DGPR:$b), [(set DGPR:$c, (sub(i32 DGPR:$a), DGPR:$b))],
          "sub", "$c, $a, $b">;
def SUBArr : TCInstRR<0x01, 0x02, (outs AGPR:$c), (ins AGPR:$a, AGPR:$b), [],
                      "sub.a", "$c, $a, $b">;
def RSUBrc
    : TCInstRC<0x8B, 0x08, (outs DGPR:$c), (ins simm9:$const9, DGPR:$a),
               [(set DGPR:$c, (sub simm9:$const9, (i32 DGPR:$a)))], "rsub",
               "$c, $const9, "
               "$a">;

let b = 0 in {
  def POPCNTW
      : TCInstRR<0x4B, 0x22, (outs DGPR:$c),
                 (ins DGPR:$a), [(set DGPR:$c, (i32(ctpop(i32 DGPR:$a))))],
                 "popcnt.w", "$c, $a">;
  def CLZ : TCInstRR<0x4F, 0x1B, (outs DGPR:$c),
                     (ins DGPR:$a), [(set DGPR:$c, (i32(ctlz(i32 DGPR:$a))))],
                     "clz", "$c, $a">;
}

def SHUFFLE
    : TCInstRC<0x8F, 0x07, (outs DGPR:$c), (ins DGPR:$a, uimm9:$const9), [],
               "shuffle", "$c, $a, $const9">;

def : Pat<(not(i32 DGPR:$a)), (NORrc DGPR:$a, (i32 0))>;
def : Pat<(srl(i32 DGPR:$a), simm9:$const9), (SHrc(i32 DGPR:$a),
                                                 (negate simm9:$const9))>;
def : Pat<(srl(i32 DGPR:$a), (i32 DGPR:$b)), (SHrr(i32 DGPR:$a),
                                                 (RSUBrc 0, (i32 DGPR:$b)))>;
def : Pat<(sra(i32 DGPR:$a), simm9:$const9), (SHArc(i32 DGPR:$a),
                                                 (negate simm9:$const9))>;
def : Pat<(sra(i32 DGPR:$a), (i32 DGPR:$b)), (SHArr(i32 DGPR:$a),
                                                 (RSUBrc 0, (i32 DGPR:$b)))>;
def : Pat<(bswap(i32 DGPR:$a)), (SHUFFLE(i32 DGPR:$a), 0x01B)>;

//===---
// Multiplication &
// Division
//===---
// Multiplication
def MULrc : TCInstRC<0x53, 0x01, (outs DGPR:$c),
                     (ins DGPR:$a, simm9:$const9), [], "mul",
                     "$c, $a, "
                     "$const9">;
def MULErc : TCInstRC<0x53, 0x03, (outs EGPR:$c),
                      (ins DGPR:$a, simm9_64:$const9), [], "mul",
                      "$c, $a, "
                      "$const9">;
def MULUrc : TCInstRC<0x53, 0x02, (outs EGPR:$c),
                      (ins DGPR:$a, simm9:$const9), [], "mul.u",
                      "$c, $a, "
                      "$const9">;
def MULrr : TCInstRR2<0x73, 0x0A, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b), [],
                      "mul", "$c, $a, $b">;
def MULErr : TCInstRR2<0x73, 0x6A, (outs EGPR:$c), (ins DGPR:$a, DGPR:$b), [],
                       "mul", "$c, $a, $b">;
def MULUrr : TCInstRR2<0x73, 0x68, (outs EGPR:$c), (ins DGPR:$a, DGPR:$b), [],
                       "mul.u", "$c, $a, $b">;
def DIV : TCInstRR<0x4B, 0x20, (outs EGPR:$c), (ins DGPR:$a, DGPR:$b), [],
                   "div", "$c, $a, $b">;
def DIVU : TCInstRR<0x4B, 0x21, (outs EGPR:$c), (ins DGPR:$a, DGPR:$b), [],
                    "div.u", "$c, $a, $b">;

def : Pat<(i32(mul DGPR:$a, simm9:$const9)), (MULrc DGPR:$a, simm9:$const9)>;
def : Pat<(i32(mul DGPR:$a, DGPR:$b)), (MULrr DGPR:$a, DGPR:$b)>;
def : Pat<(mulhu(i32 DGPR:$a), DGPR:$b),
          (i32(EXTRACT_SUBREG(i64(MULUrr DGPR:$a, DGPR:$b)), sub_odd))>;
def : Pat<(mulhs(i32 DGPR:$a), DGPR:$b),
          (i32(EXTRACT_SUBREG(i64(MULErr DGPR:$a, DGPR:$b)), sub_odd))>;
def : Pat<(sdiv(i32 DGPR:$a), DGPR:$b),
          (i32(EXTRACT_SUBREG(i64(DIV DGPR:$a, DGPR:$b)), sub_even))>;
def : Pat<(srem(i32 DGPR:$a), DGPR:$b),
          (i32(EXTRACT_SUBREG(i64(DIV DGPR:$a, DGPR:$b)), sub_odd))>;
def : Pat<(udiv(i32 DGPR:$a), DGPR:$b),
          (i32(EXTRACT_SUBREG(i64(DIVU DGPR:$a, DGPR:$b)), sub_even))>;
def : Pat<(urem(i32 DGPR:$a), DGPR:$b),
          (i32(EXTRACT_SUBREG(i64(DIVU DGPR:$a, DGPR:$b)), sub_odd))>;

// Transforms frameindex
// -> tframeindex.
def to_tframeindex : SDNodeXForm<frameindex, [{
  return CurDAG->getTargetFrameIndex(N->getIndex(), N->getValueType(0));
}]>;

def : GICustomOperandRenderer<"renderFrameIn"
                              "dex">,
      GISDNodeXFormEquiv<to_tframeindex>;

def : Pat<(frameindex:$fi), (LEAbol(iPTR(to_tframeindex $fi)), 0)>;
def : Pat<(sub frameindex:$fi, simm16:$const16),
          (LEAbol(iPTR(to_tframeindex $fi)), (negate simm16:$const16))>;

// Branch conditional
multiclass cond_branch_unsigned<CondCode cond, CondCode condu, int op1,
                                string asmstring> {
  let isBranch = 1, isTerminator = 1 in {
    def brc : TCInstBRC<!add(op1, 0x80), 0x00, (outs),
                        (ins DGPR:$a, simm4:$const4, br_disp15:$disp15),
                        [(brcc cond, (i32 DGPR:$a), simm4:$const4, bb:$disp15)],
                        asmstring,
                        "$a, "
                        "$const4, "
                        "$disp15">;
    def brr : TCInstBRR<
                  op1, 0x00, (outs), (ins DGPR:$a, DGPR:$b, br_disp15:$disp15),
                  [(brcc cond, (i32 DGPR:$a), DGPR:$b, bb:$disp15)], asmstring,
                  "$a, $b, "
                  "$disp15">;
    def Ubrc
        : TCInstBRC<!add(op1, 0x80), 0x01, (outs),
                    (ins DGPR:$a, simm4:$const4, br_disp15:$disp15),
                    [(brcc condu, (i32 DGPR:$a), simm4:$const4, bb:$disp15)],
                    !strconcat(asmstring, ".u"),
                    "$a, "
                    "$const4, "
                    "$disp15">;
    def Ubrr : TCInstBRR<op1, 0x01, (outs),
                         (ins DGPR:$a, DGPR:$b, br_disp15:$disp15),
                         [(brcc condu, (i32 DGPR:$a), DGPR:$b, bb:$disp15)],
                         !strconcat(asmstring, ".u"),
                         "$a, $b, "
                         "$disp15">;
  }
}

multiclass cond_branch<CondCode cond, int op1, int op2, string asmstring> {
  let isBranch = 1, isTerminator = 1 in {
    def brc : TCInstBRC<!add(op1, 0x80), op2, (outs),
                        (ins DGPR:$a, simm4:$const4, br_disp15:$disp15),
                        [(brcc cond, (i32 DGPR:$a), simm4:$const4, bb:$disp15)],
                        asmstring,
                        "$a, "
                        "$const4, "
                        "$disp15">;
    def brr
        : TCInstBRR<op1, op2, (outs), (ins DGPR:$a, DGPR:$b, br_disp15:$disp15),
                    [(brcc cond, (i32 DGPR:$a), DGPR:$b, bb:$disp15)],
                    asmstring,
                    "$a, $b, "
                    "$disp15">;
  }
  /* TODO: +16 disp4
   * instructions */
}

multiclass cond_branch_inferred<CondCode cond, Instruction inst,
                                Instruction instb, string asmstring> {
  def : Pat<(brcc cond, (i32 DGPR:$a), simm4:$const4, bb:$disp15),
            (inst DGPR:$a, (plus1 simm4:$const4), br_disp15:$disp15)>;
  def : Pat<(brcc cond, (i32 DGPR:$a), DGPR:$b, bb:$disp15),
            (instb DGPR:$b, DGPR:$a, br_disp15:$disp15)>;
  /* TODO: +1*/
  def : InstAlias<!strconcat(asmstring, " $a, "
                                        "$const4"
                                        ", "
                                        "$disp1"
                                        "5"),
                  (inst DGPR:$a, simm4:$const4, br_disp15:$disp15)>;
  def : InstAlias<!strconcat(asmstring, " $a, "
                                        "$b, "
                                        "$disp1"
                                        "5"),
                  (instb DGPR:$b, DGPR:$a, br_disp15:$disp15)>;
}

defm JLT : cond_branch_unsigned<SETLT, SETULT, 0x3F, "jlt">;
defm JGE : cond_branch_unsigned<SETGE, SETUGE, 0x7F, "jge">;
defm JGT : cond_branch_inferred<SETGT, JGEbrc, JLTbrr, "jgt">;
defm JGTU : cond_branch_inferred<SETUGT, JGEUbrc, JLTUbrr, "jgt.u">;
defm JLE : cond_branch_inferred<SETLE, JLTbrc, JGEbrr, "jle">;
defm JLEU : cond_branch_inferred<SETULE, JLTUbrc, JGEUbrr, "jle.u">;
defm JEQ : cond_branch<SETEQ, 0x5f, 0x0, "jeq">;
defm JNE : cond_branch<SETNE, 0x5f, 0x1, "jne">;

let isBranch = 1, isTerminator = 1 in {
  def JZ_T : TCInstBRN<0x6F, 0x0, (outs),
                       (ins DGPR:$a, pos:$n, br_disp15:$disp15), [], "jz.t",
                       "$a, $n, $disp15">;
  def JNZ_T : TCInstBRN<0x6F, 0x1, (outs),
                        (ins DGPR:$a, pos:$n, br_disp15:$disp15), [], "jz.t",
                        "$a, $n, $disp15">;
}
def : Pat<(brcond(assertzext(i32 DGPR:$a)), bb:$disp15),
          (JZ_T DGPR:$a, 0, br_disp15:$disp15)>;
def : Pat<(brcond(i32 DGPR:$a), bb:$disp15), (JNEbrc DGPR:$a, 0,
                                                 br_disp15:$disp15)>;

let isBranch = 1, isTerminator = 1, isBarrier = 1 in {
  def J : TCInstB<0x1D, (outs), (ins br_disp24:$disp24), [(br bb:$disp24)], "j",
                  "$disp24">;
  def JI : TCInstRR<0x2D, 0x03, (outs), (ins AGPR:$a), [(brind AGPR:$a)], "ji",
                    "$a"> {
    let b = 0;
    let c = 0;
    let isIndirectBranch = 1;
  }
}

let isCall = 1, isBarrier = 1, mayStore = 1, mayLoad = 0,
    hasSideEffects = 1 in {
  def CALL : TCInstB<0x6D, (outs), (ins disp24:$disp24), [], "call", "$disp24">;
  def CALLI : TCInstRR<0x2D, 0x00, (outs),
                       (ins AGPR:$a), [(tricore_call AGPR:$a)], "calli", "$a"> {
    let c = 0;
    let b = 0;
  }
}

let isCall = 1, isTerminator = 1, isReturn = 1, isBarrier = 1, Size = 4,
    isCodeGenOnly = 0 in {
  def PseudoTAIL : Pseudo<(outs), (ins disp24:$dst), [], "tail", "$dst">;
  def PseudoTAILI
      : Pseudo<(outs), (ins AGPR:$a), [(tricore_tail AGPR:$a)], "taili", "$a">,
        PseudoInstExpansion<(JI AGPR:$a)>;
}

def : Pat<(tricore_call tglobaladdr:$disp24), (CALL tglobaladdr:$disp24)>;
def : Pat<(tricore_call texternalsym:$disp24), (CALL texternalsym:$disp24)>;
def : Pat<(tricore_tail tglobaladdr:$disp24), (PseudoTAIL disp24:$disp24)>;
def : Pat<(tricore_tail texternalsym:$disp24), (PseudoTAIL disp24:$disp24)>;

// Select
multiclass selectcc_inst<int op1, int op2, CondCode cond, CondCode condu> {
  def rc : InstRC<!add(op1, 0x80), op2, !tolower(NAME)>;
  def rr : InstRR<op1, op2, !tolower(NAME)>;
  def _Urc : InstRC<!add(op1, 0x80), !add(op2, 1), !tolower(NAME#".u")>;
  def _Urr : InstRR<op1, !add(op2, 1), !tolower(NAME#".u")>;
  def _Hrc : InstRC<!add(op1, 0x80), !add(op2, 0x60), !tolower(NAME#".h")>;
  def _Hrr : InstRR<op1, !add(op2, 0x60), !tolower(NAME#".h")>;
  def _HUrc : InstRC<!add(op1, 0x80), !add(op2, 0x61), !tolower(NAME#".hu")>;
  def _HUrr : InstRR<op1, !add(op2, 0x61), !tolower(NAME#".hu")>;
  def _Brc : InstRC<!add(op1, 0x80), !add(op2, 0x40), !tolower(NAME#".b")>;
  def _Brr : InstRR<op1, !add(op2, 0x40), !tolower(NAME#".b")>;
  def _BUrc : InstRC<!add(op1, 0x80), !add(op2, 0x41), !tolower(NAME#".bu")>;
  def _BUrr : InstRR<op1, !add(op2, 0x41), !tolower(NAME#".bu")>;

  def : Pat<(selectcc(i32 DGPR:$a), simm9:$const9, DGPR:$a, simm9:$const9,
                cond),
            (!cast<Instruction>(NAME#rc) DGPR:$a, simm9:$const9)>;
  def : Pat<(selectcc(i32 DGPR:$a), (i32 DGPR:$b), DGPR:$a, DGPR:$b, cond),
            (!cast<Instruction>(NAME#rr) DGPR:$a, DGPR:$b)>;
  def : Pat<(selectcc(i32 DGPR:$a), simm9:$const9, DGPR:$a, simm9:$const9,
                condu),
            (!cast<Instruction>(NAME#_Urc) DGPR:$a, simm9:$const9)>;
  def : Pat<(selectcc(i32 DGPR:$a), (i32 DGPR:$b), DGPR:$a, DGPR:$b, condu),
            (!cast<Instruction>(NAME#_Urr) DGPR:$a, DGPR:$b)>;

  def : Pat<(vselect(setcc(v4i8 DGPR:$a), DGPR:$b, cond), (v4i8 DGPR:$a),
                DGPR:$b),
            (!cast<Instruction>(NAME#_Brr) DGPR:$a, DGPR:$b)>;
}

multiclass select_inst<int op2, CondCode cond> {
  def rcr : TCInstRCR<0xAB, op2, (outs DGPR:$c),
                      (ins DGPR:$d, DGPR:$a, simm9:$const9), [], !tolower(NAME),
                      "$c, $d, $a, $const9">;
  def rrr : TCInstRRR<0x2B, op2, (outs DGPR:$c),
                      (ins DGPR:$d, DGPR:$a, DGPR:$b), [], !tolower(NAME),
                      "$c, $d, $a, $b">;

  def : Pat<(selectcc(i32 DGPR:$d), 0, (i32 DGPR:$a), simm9:$const9, cond),
            (!cast<Instruction>(NAME#rcr) DGPR:$d, DGPR:$a, simm9:$const9)>;
  def : Pat<(selectcc(i32 DGPR:$d), 0, (i32 DGPR:$a), DGPR:$b, cond),
            (!cast<Instruction>(NAME#rrr) DGPR:$d, DGPR:$a, DGPR:$b)>;
}

defm SEL : select_inst<0x4, SETNE>;
defm SELN : select_inst<0x5, SETEQ>;
defm MIN : selectcc_inst<0x0B, 0x18, SETLT, SETULT>;
defm MAX : selectcc_inst<0x0B, 0x1A, SETGT, SETUGT>;

// Additional patterns for optimizations
def : Pat<(selectcc(i32 DGPR:$d), 0, simm9:$const9, (i32 DGPR:$a), SETEQ),
          (SELrcr DGPR:$d, DGPR:$a, simm9:$const9)>;
def : Pat<(selectcc(i32 DGPR:$d), 0, simm9:$const9, (i32 DGPR:$a), SETNE),
          (SELNrcr DGPR:$d, DGPR:$a, simm9:$const9)>;

multiclass selectcc_pat<string setcc, CondCode cc> {
  def : Pat<(selectcc(i32 DGPR:$a), simm9:$const9, (i32 DGPR:$c), simm9:$const9,
                cc),
            (SELrcr(!cast<Instruction>(setcc#rc) DGPR:$a, simm9:$const9),
                DGPR:$c, simm9:$const9)>;
  def : Pat<(selectcc(i32 DGPR:$a), simm9:$const9, (i32 DGPR:$c), (i32 DGPR:$d),
                cc),
            (SELrrr(!cast<Instruction>(setcc#rc) DGPR:$a, simm9:$const9),
                DGPR:$c, DGPR:$d)>;
  def : Pat<(selectcc(i32 DGPR:$a), (i32 DGPR:$b), (i32 DGPR:$c), simm9:$const9,
                cc),
            (SELrcr(!cast<Instruction>(setcc#rr) DGPR:$a, DGPR:$b), DGPR:$c,
                simm9:$const9)>;
  def : Pat<(selectcc(i32 DGPR:$a), (i32 DGPR:$b), (i32 DGPR:$c), (i32 DGPR:$d),
                cc),
            (SELrrr(!cast<Instruction>(setcc#rr) DGPR:$a, DGPR:$b), DGPR:$c,
                DGPR:$d)>;
}

defm : selectcc_pat<"EQ", SETEQ>;
defm : selectcc_pat<"NE", SETNE>;
defm : selectcc_pat<"GE", SETGE>;
defm : selectcc_pat<"GE_U", SETUGE>;
defm : selectcc_pat<"LT", SETLT>;
defm : selectcc_pat<"LT_U", SETULT>;

def : Pat<(i32(select(i32 DGPR:$d), (i32 DGPR:$a), (i32 DGPR:$b))),
          (SELrrr(i32 DGPR:$d), (i32 DGPR:$a), (i32 DGPR:$b))>;
def : Pat<(f32(select(i32 DGPR:$d), (f32 DGPR:$a), (f32 DGPR:$b))),
          (SELrrr(i32 DGPR:$d), (f32 DGPR:$a), (f32 DGPR:$b))>;

// Extract and Insert
let b = 0 in {
  def EXTRrrpw : TCInstRRPW<0x37, 0x2, (outs DGPR:$c),
                            (ins DGPR:$a, pos:$pos, width:$width),
                            [(set DGPR:$c, (tricore_extr(i32 DGPR:$a), pos:$pos,
                                               width:$width))],
                            "extr",
                            "$c, $a, "
                            "$pos, "
                            "$width">;
  def EXTRUrrpw : TCInstRRPW<0x37, 0x3, (outs DGPR:$c),
                             (ins DGPR:$a, pos:$pos, width:$width),
                             [(set DGPR:$c, (tricore_extru(i32 DGPR:$a),
                                                pos:$pos, width:$width))],
                             "extr.u",
                             "$c, $a, "
                             "$pos, "
                             "$width">;
}

def : Pat<(sext_inreg i32:$a, i1), (EXTRrrpw i32:$a, 0, 1)>;
def : Pat<(sext_inreg i32:$a, i8), (EXTRrrpw i32:$a, 0, 8)>;
def : Pat<(sext_inreg i32:$a, i16), (EXTRrrpw i32:$a, 0, 16)>;

def INSERTrcpw
    : TCInstRCPW<0xB7, 0x0, (outs DGPR:$c),
                 (ins DGPR:$a, uimm4:$const4, pos:$pos, width:$width),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), uimm4:$const4,
                                    pos:$pos, width:$width))],
                 "insert",
                 "$c, $a, "
                 "$const4, "
                 "$pos, "
                 "$width">;

def INSERTrcrr
    : TCInstRCRR<0x97, 0x0, (outs DGPR:$c),
                 (ins DGPR:$a, uimm4:$const4, EGPR:$d),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), uimm4:$const4,
                                    (i32(extractelt(v2i32 EGPR:$d), 0)),
                                    (i32(extractelt(v2i32 EGPR:$d), 1))))],
                 "insert",
                 "$c, $a, "
                 "$const4, $d">;

def INSERTrcrw
    : TCInstRCRW<0xD7, 0x0, (outs DGPR:$c),
                 (ins DGPR:$a, uimm4:$const4, DGPR:$d, width:$width),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), uimm4:$const4,
                                    (i32 DGPR:$d), width:$width))],
                 "insert",
                 "$c, $a, "
                 "$const4, $d, "
                 "$width">;

def INSERTrrpw
    : TCInstRRPW<0x37, 0x2, (outs DGPR:$c),
                 (ins DGPR:$a, DGPR:$b, pos:$pos, width:$width),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), (i32 DGPR:$b),
                                    pos:$pos, width:$width))],
                 "insert",
                 "$c, $a, $b, "
                 "$pos, "
                 "$width">;
def INSERTrrrr : TCInstRRRR<0x17, 0x0, (outs DGPR:$c),
                            (ins DGPR:$a, DGPR:$b, EGPR:$d), [], "insert",
                            "$c, $a, $b, "
                            "$d">;
def INSERTrrrw
    : TCInstRRRW<0x57, 0x0, (outs DGPR:$c),
                 (ins DGPR:$a, DGPR:$b, DGPR:$d, width:$width),
                 [(set DGPR:$c, (tricore_insert(i32 DGPR:$a), (i32 DGPR:$b),
                                    (i32 DGPR:$d), width:$width))],
                 "insert",
                 "$c, $a, $b, "
                 "$d, $width">;

def : Pat<(int_tricore_insert DGPR:$a, uimm4:$const4, DGPR:$d0, DGPR:$d1),
          (INSERTrcrr DGPR:$a, uimm4:$const4,
              (INSERT_SUBREG
                  (INSERT_SUBREG(v2i32(IMPLICIT_DEF)), DGPR:$d1, sub_odd),
                  DGPR:$d0, sub_even))>;
def : Pat<(int_tricore_insert DGPR:$a, DGPR:$b, DGPR:$d0, DGPR:$d1),
          (INSERTrrrr DGPR:$a, DGPR:$b,
              (INSERT_SUBREG
                  (INSERT_SUBREG(v2i32(IMPLICIT_DEF)), DGPR:$d1, sub_odd),
                  DGPR:$d0, sub_even))>;
//==--
// Bit
//==--
class BitInst<bits<8> op1, bits<2> op2, string opcodestr>
    : TCInstBIT<op1, op2, (outs DGPR:$c),
                (ins DGPR:$a, pos:$pos1, DGPR:$b, pos:$pos2), [], opcodestr,
                "$c, $a, $pos1, $b, $pos2">;
def AND_AND_T : BitInst<0x47, 0x0, "and.and.t">;
def AND_OR_T : BitInst<0x47, 0x1, "and.or.t">;
def AND_NOR_T : BitInst<0x47, 0x2, "and.nor.t">;
def AND_ANDN_T : BitInst<0x47, 0x3, "and.andn.t">;
def OR_AND_T : BitInst<0xC7, 0x0, "or.and.t">;
def OR_OR_T : BitInst<0xC7, 0x1, "or.or.t">;
def OR_NOR_T : BitInst<0xC7, 0x2, "or.nor.t">;
def OR_ANDN_T : BitInst<0xC7, 0x3, "or.andn.t">;
def SH_AND_T : BitInst<0x27, 0x0, "sh.and.t">;
def SH_OR_T : BitInst<0x27, 0x1, "sh.or.t">;
def SH_NOR_T : BitInst<0x27, 0x2, "sh.nor.t">;
def SH_ANDN_T : BitInst<0x27, 0x3, "sh.andn.t">;
def SH_NAND_T : BitInst<0xA7, 0x0, "sh.nand.t">;
def SH_ORN_T : BitInst<0xA7, 0x1, "sh.orn.t">;
def SH_XNOR_T : BitInst<0xA7, 0x2, "sh.xnor.t">;
def SH_XOR_T : BitInst<0xA7, 0x3, "sh.xor.t">;

// AND.EQ OR.EQ XOR.EQ
// AND.GE AND.GE.U ...
// AND.LT AND.LT.U ..
// AND.NE ..
class SimpleBitInst<bits<8> op1, bits<2> op2, SDPatternOperator Op,
                    string opcodestr>
    : TCInstBIT<op1, op2, (outs DGPR:$c),
                (ins DGPR:$a, pos:$pos1, DGPR:$b, pos:$pos2),
                [(set DGPR:$c, (Op(i32 DGPR:$a), pos:$pos1, (i32 DGPR:$b),
                                   pos:$pos2))],
                opcodestr, "$c, $a, $pos1, $b, $pos2">;

def AND_T : SimpleBitInst<0x87, 0x0, tricore_and_t, "and.t">;
def OR_T : SimpleBitInst<0x87, 0x1, tricore_or_t, "or.t">;
def NOR_T : SimpleBitInst<0x87, 0x2, tricore_nor_t, "nor.t">;
def ANDN_T : SimpleBitInst<0x87, 0x3, tricore_andn_t, "andn.t">;
def NAND_T : SimpleBitInst<0x07, 0x0, tricore_nand_t, "nand.t">;
def ORN_T : SimpleBitInst<0x07, 0x1, tricore_orn_t, "orn.t">;
def XNOR_T : SimpleBitInst<0x07, 0x2, tricore_xnor_t, "xnor.t">;
def XOR_T : SimpleBitInst<0x07, 0x3, tricore_xor_t, "xor.t">;

//===----------------------------------------------------------------------===//
// Float & Double
//===----------------------------------------------------------------------===//
class FPInst<bits<8> op1, bits<8> op2, SDPatternOperator Op, string asmstring>
    : TCInstRR<op1, op2, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b),
               [(set DGPR:$c, (Op(f32 DGPR:$a), (f32 DGPR:$b)))], asmstring,
               "$c, $a, $b">;
class FPInstUnary<bits<8> op1, bits<8> op2, SDPatternOperator Op,
                  string asmstring, ValueType vt = f32>
    : TCInstRR<op1, op2, (outs DGPR:$c),
               (ins DGPR:$a), [(set DGPR:$c, (vt(Op(f32 DGPR:$a))))], asmstring,
               "$c, $a"> {
  let b = 0;
}
class FPInstBinary<bits<8> op1, bits<4> op2, SDPatternOperator op,
                   string opcodestr>
    : TCInstRRR<op1, op2, (outs DGPR:$c), (ins DGPR:$d, DGPR:$a),
                [(set DGPR:$c, (op(f32 DGPR:$d), (f32 DGPR:$a)))], opcodestr,
                "$c, $d, $a"> {
  let b = 0;
}
class FPInstTernary<bits<8> op1, bits<4> op2, SDPatternOperator op_n1,
                    SDPatternOperator op_n2, string opcodestr>
    : TCInstRRR<op1, op2, (outs DGPR:$c), (ins DGPR:$d, DGPR:$a, DGPR:$b),
                [(set DGPR:$c, (op_n1(f32 DGPR:$d), (op_n2(f32 DGPR:$a),
                                                        (f32 DGPR:$b))))],
                opcodestr, "$c, $d, $a, $b">;

def ABSF : FPInstUnary<0x4B, 0x30, fabs, "abs.f">;
def ADDF : FPInstBinary<0x6B, 0x2, any_fadd, "add.f">;
def CMPF : TCInstRR<0x4B, 0x00, (outs DGPR:$c), (ins DGPR:$a, DGPR:$b),
                    [(set DGPR:$c, (i32(tricore_cmp_f(f32 DGPR:$a),
                                       (f32 DGPR:$b))))],
                    "cmp.f", "$c, $a, $b">;
def DIVF : FPInst<0x4B, 0x05, any_fdiv, "div.f">;
// def FTOI : FPInstUnary<0x4B, 0x10, asd, "FTOI">;
// def FTOIN
def FTOIZ : FPInstUnary<0x4B, 0x13, any_fp_to_sint, "ftoiz", i32>;
// FTOQ31
// FTOQ31Z
// FTOU
def FTOUZ : FPInstUnary<0x4B, 0x15, any_fp_to_uint, "ftouz", i32>;
// FTOHP
// HPTOF
def ITOF : I32InstRRUnary<0x4B, 0x14, any_sint_to_fp, "itof">;
def MADDF : FPInstTernary<0x6B, 0x6, any_fadd, any_fmul, "madd.f">;
def MAXF : FPInst<0x4B, 0x32, fmaximumnum, "max.f">;
def MINF : FPInst<0x4B, 0x33, fminimumnum, "min.f">;
def MSUBF : FPInstTernary<0x6B, 0x7, any_fsub, any_fmul, "msub.f">;
def MULF : FPInst<0x4B, 0x4, any_fmul, "mul.f">;
def NEGF : FPInstUnary<0x4B, 0x31, fneg, "neg.f">;
// Q31TOF
// QSEED.F
def SUBF : FPInstBinary<0x6B, 0x3, any_fsub, "sub.f">;
// UPDFL
def UTOF : I32InstRRUnary<0x4B, 0x16, any_uint_to_fp, "utof">;

def : Pat<(i32(bitconvert (f32 DGPR:$a))),(i32 DGPR:$a)>;
def : Pat<(f32(bitconvert (i32 DGPR:$a))),(f32 DGPR:$a)>;
def : Pat<(fcopysign(f32 DGPR:$a), (f32 DGPR:$b)),
          (f32(INSERTrrpw(f32 DGPR:$a), (EXTRUrrpw(f32 DGPR:$b), 31, 1), 31,
              1))>;

//===----------------------------------------------------------------------===//
// Intrinsics
//===----------------------------------------------------------------------===//

def MTCR : TCInstRLC<0xCD, (outs), (ins tuimm16:$const16, DGPR:$a),
                     [(int_tricore_mtcr tuimm16:$const16, DGPR:$a)], "mtcr",
                     "$const16, "
                     "$a"> {
  let c = 0;
}

//===----------------------------------------------------------------------===//
//  Pattern
//===----------------------------------------------------------------------===//

def : Pat<(i64 simm16_64:$imm), (MOVErlc simm16_64:$imm)>;
def : Pat<(i32 imm:$imm), (ADDIH(MOVrlc(lo16 imm:$imm)), (hi16 imm:$imm))>;
def : Pat<(i32 uimm16:$imm), (MOVrlc uimm16:$imm)>;

// extract_vector
def : Pat<(extractelt(v2i32 EGPR:$a), 0), (i32(EXTRACT_SUBREG EGPR:$a,
                                              sub_even))>;
def : Pat<(extractelt(v2i32 EGPR:$a), 1), (i32(EXTRACT_SUBREG EGPR:$a,
                                              sub_odd))>;
// Build vector
def : Pat<(build_vector(i32 0), (i32 0)), (v2i32(MOVErlc 0))>;
def : Pat<(build_vector(i32 0), (i32 DGPR:$a)), (INSERT_SUBREG(INSERT_SUBREG
                                                     (v2i32(IMPLICIT_DEF)),
                                                     (MOVrlc 0), sub_even),
                                                    (i32 DGPR:$a), sub_odd)>;
// build_vector
def : Pat<(build_vector(i32 DGPR:$a), (i32 DGPR:$b)),
          (INSERT_SUBREG
              (INSERT_SUBREG(v2i32(IMPLICIT_DEF)), (i32 DGPR:$a), sub_even),
              (i32 DGPR:$b), sub_odd)>;

include "TricoreInstrInfo16.td"