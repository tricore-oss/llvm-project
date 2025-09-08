
def uimm8 : TricoreUImmLeafOp<8>;
def br_disp4 : TricoreBrDispImmOp<4>,
               ImmLeaf<i32, [{return isShiftedInt<4, 1>(Imm);}]>;
def br_disp8 : TricoreBrDispImmOp<8>,
               ImmLeaf<i32, [{return isShiftedInt<8, 1>(Imm);}]>;

class CBinOpImmInst<bits<8> Op1, string opcstring>
    : TCInstSRC<Op1, (outs), (ins DGPR:$a, simm4:$const4), opcstring,
                "d15, $a, $const4">;
class CBinOpInst<bits<8> Op1, string opcstring>
    : TCInstSRR<Op1, (outs), (ins DGPR:$a, DGPR:$b), opcstring, "d15, $a, $b">;
class CBinOpImmOutInst<bits<8> Op1, string opcstring>
    : TCInstSRC<Op1, (outs DGPR:$a), (ins simm4:$const4), opcstring,
                "$a, d15, $const4">;
class CBinOpOutInst<bits<8> Op1, string opcstring>
    : TCInstSRR<Op1, (outs DGPR:$a), (ins DGPR:$b), opcstring, "$a, d15,  $b">;
class CBinOpImmInOutInst<bits<8> Op1, string opcstring>
    : TCInstSRC<Op1, (outs DGPR:$c), (ins DGPR:$a, simm4:$const4), opcstring,
                "$a, $const4"> {
  let Constraints = "$c = $a";
}
class CBinOpInOutInst<bits<8> Op1, string opcstring>
    : TCInstSRR<Op1, (outs), (ins DGPR:$a, DGPR:$b), opcstring, "$a,  $b">;
class CBinOpImmOnlyInst<bits<8> Op1, string opcstring>
    : TCInstSC<Op1, (outs), (ins uimm8:$const8), opcstring, "d15, $const8">;

multiclass CBinOpImm<bits<8> Op1, string inststring, string opcstring> {
  def NAME : CBinOpImmInst<Op1, opcstring>;
  def : CompressPat<(!cast<Instruction>(inststring#rc) D15, DGPR:$a,
                        simm4:$const4),
                    (!cast<Instruction>(NAME) DGPR:$a, simm4:$const4)>;
}
multiclass CBinOp<bits<8> Op1, string inststring, string opcstring> {
  def NAME : CBinOpInst<Op1, opcstring>;
  def : CompressPat<(!cast<Instruction>(inststring#rr) D15, DGPR:$a, DGPR:$b),
                    (!cast<Instruction>(NAME) DGPR:$a, DGPR:$b)>;
}
multiclass CBinOpImmOut<bits<8> Op1, string inststring, string opcstring> {
  def NAME : CBinOpImmOutInst<Op1, opcstring>;
  def : CompressPat<(!cast<Instruction>(inststring#rc) DGPR:$a, D15,
                        simm4:$const4),
                    (!cast<Instruction>(NAME) DGPR:$a, simm4:$const4)>;
}
multiclass CBinOpOut<bits<8> Op1, string inststring, string opcstring> {
  def NAME : CBinOpOutInst<Op1, opcstring>;
  def : CompressPat<(!cast<Instruction>(inststring#rr) DGPR:$a, D15, DGPR:$b),
                    (!cast<Instruction>(NAME) DGPR:$a, DGPR:$b)>;
}
multiclass CBinOpImmInOut<bits<8> Op1, string inststring, string opcstring> {
  def NAME : CBinOpImmInOutInst<Op1, opcstring>;
  def : CompressPat<(!cast<Instruction>(inststring#rc) DGPR:$a, DGPR:$a,
                        simm4:$const4),
                    (!cast<Instruction>(NAME) DGPR:$a, simm4:$const4)>;
}
multiclass CBinOpInOut<bits<8> Op1, string inststring, string opcstring> {
  def NAME : CBinOpInOutInst<Op1, opcstring>;
  def : CompressPat<(!cast<Instruction>(inststring#rr) DGPR:$a, DGPR:$a,
                        DGPR:$b),
                    (!cast<Instruction>(NAME) DGPR:$a, DGPR:$b)>;
}
multiclass CBinOpImmOnly<bits<8> Op1, string inststring, string opcstring> {
  def NAME : CBinOpImmOnlyInst<Op1, opcstring>;
  def : CompressPat<(!cast<Instruction>(inststring#rc) D15, D15, uimm8:$const8),
                    (!cast<Instruction>(NAME) uimm8:$const8)>;
}

defm ADDsrc : CBinOpImm<0x9A, "ADD", "add">;
defm ADDsrc_out : CBinOpImmOut<0x92, "ADD", "add">;
defm ADDsrc_inout : CBinOpImmInOut<0xC2, "ADD", "add">;
defm ADDsrr : CBinOp<0x1A, "ADD", "add">;
defm ADDsrr_out : CBinOpOut<0x12, "ADD", "add">;
defm ADDsrr_inout : CBinOpInOut<0x42, "ADD", "add">;
// ADDS
defm ANDsc : CBinOpImmOnly<0x16, "AND", "and">;
defm ANDsrc : CBinOpInOut<0x26, "AND", "and">;
// EQ
// LT
// MUL
// NOT
// OR
// RSUB
// SH
// SHA
// SUB
// SUBS
// XOR

// Conditional instructions
let Constraints = "$c = $a" in {
  def CMOVsrc : TCInstSRC<0xAA, (outs DGPR:$a), (ins simm4:$const4, DGPR:$c),
                          "cmov", "$a, d15, $const4">;
  def CMOVNsrc : TCInstSRC<0xEA, (outs DGPR:$a), (ins simm4:$const4, DGPR:$c),
                           "cmov", "$a, d15, $const4">;
  def CMOVsrr : TCInstSRR<0x2A, (outs DGPR:$a), (ins DGPR:$b, DGPR:$c), "cmov",
                          "$a, d15, $b">;
  def CMOVNsrr : TCInstSRR<0x6A, (outs DGPR:$a), (ins DGPR:$b, DGPR:$c), "cmov",
                           "$a, d15, $b">;
}
// CADD
// CADDN
// CSUB
// CSUBN
def : CompressPat<(SELNrcr DGPR:$a, D15, DGPR:$a, simm4:$const4),
                  (CMOVsrc DGPR:$a, simm4:$const4)>;
def : CompressPat<(SELrcr DGPR:$a, D15, DGPR:$a, simm4:$const4),
                  (CMOVNsrc DGPR:$a, simm4:$const4)>;
def : CompressPat<(SELNrrr DGPR:$a, D15, DGPR:$a, DGPR:$b), (CMOVsrr DGPR:$a,
                                                                DGPR:$b)>;
def : CompressPat<(SELrrr DGPR:$a, D15, DGPR:$a, DGPR:$b), (CMOVNsrr DGPR:$a,
                                                               DGPR:$b)>;

// Address instructions
// ADD_A
// ADDSC_A
def SUB_Asc
    : TCInstSC<0x20, (outs), (ins uimm8:$const8), "sub.a", "a10, $const8"> {
  let Uses = [A10];
  let Defs = [A10];
}

// Move instructions
let isMoveImm = 1 in {
  def MOVsc
      : TCInstSC<0xDA, (outs), (ins uimm8:$const8), "mov", "d15, $const8">;
  def MOVsrc : TCInstSRC<0x82, (outs DGPR:$a), (ins simm4:$const4), "mov",
                         "$a, $const4">;
  def MOVEsrc : TCInstSRC<0xD2, (outs EGPR:$a), (ins simm4:$const4), "mov",
                          "$a, $const4">;
  def MOVAsrc : TCInstSRC<0xA0, (outs AGPR:$a), (ins uimm4:$const4), "mov.a",
                          "$a, $const4">;
}
def : CompressPat<(MOVrlc D15, uimm8:$const8), (MOVsc uimm8:$const8)>;
def : CompressPat<(MOVrlc DGPR:$a, simm4:$const4), (MOVsrc DGPR:$a,
                                                       simm4:$const4)>;
def : CompressPat<(MOVErlc EGPR:$a, simm4:$const4), (MOVEsrc EGPR:$a,
                                                        simm4:$const4)>;
// def : CompressPat<(LEAabs AGPR:$a, simm4:$const4), (MOVAsrc DGPR:$a,
//                                                        simm4:$const4)>;
let isMoveReg = 1 in {
  def MOVsrr : TCInstSRR<0x02, (outs DGPR:$a), (ins DGPR:$b), "mov", "$a, $b">;
  def MOVAsrr
      : TCInstSRR<0x60, (outs AGPR:$a), (ins DGPR:$b), "mov.a", "$a, $b">;
  def MOVAAsrr
      : TCInstSRR<0x40, (outs AGPR:$a), (ins AGPR:$b), "mov.aa", "$a, $b">;
  def MOVDsrr
      : TCInstSRR<0x80, (outs DGPR:$a), (ins AGPR:$b), "mov.d", "$a, $b">;
}
def : CompressPat<(MOVrr DGPR:$a, DGPR:$b), (MOVsrr DGPR:$a, DGPR:$b)>;
def : CompressPat<(MOVArr AGPR:$a, DGPR:$b), (MOVAsrr AGPR:$a, DGPR:$b)>;
def : CompressPat<(MOVAArr AGPR:$a, AGPR:$b), (MOVAAsrr AGPR:$a, AGPR:$b)>;
def : CompressPat<(MOVDrr DGPR:$a, AGPR:$b), (MOVDsrr DGPR:$a, AGPR:$b)>;

// Branch Instructions
multiclass CCall<bits<8> op1, bits<4> op2> {
  let isCall = 1, isBarrier = 1, mayStore = 1, mayLoad = 0,
      hasSideEffects = 1 in {
    def sb : TCInstSB<op1, (outs), (ins br_disp8:$disp8), !tolower(NAME),
                      "$disp8">;
    def Isr
        : TCInstSR<0xDC, op2, (outs), (ins AGPR:$a), !tolower(NAME#I), "$a">;
  }
  def : CompressPat<(!cast<Instruction>(NAME) br_disp8:$disp8),
                    (!cast<Instruction>(NAME#sb) br_disp8:$disp8)>;
  def : CompressPat<(!cast<Instruction>(NAME#I) AGPR:$a),
                    (!cast<Instruction>(NAME#Isr) AGPR:$a)>;
}
multiclass CBranch<bits<8> op1, bits<4> op2> {
  let isBranch = 1, isTerminator = 1, isBarrier = 1 in {
    def sb : TCInstSB<op1, (outs), (ins br_disp8:$disp8), !tolower(NAME),
                      "$disp8">;
    def Isr
        : TCInstSR<0xDC, op2, (outs), (ins AGPR:$a), !tolower(NAME#I), "$a">;
  }
  def : CompressPat<(!cast<Instruction>(NAME) br_disp8:$disp8),
                    (!cast<Instruction>(NAME#sb) br_disp8:$disp8)>;
  def : CompressPat<(!cast<Instruction>(NAME#I) AGPR:$a),
                    (!cast<Instruction>(NAME#Isr) AGPR:$a)>;
}
multiclass CBranchCompareZero<bits<8> op1, string inststring> {
  def sb : TCInstSB<op1, (outs), (ins br_disp8:$disp8), !tolower(NAME),
                    "d15, $disp8">;
  def sbr : TCInstSBR<!add(op1, 0x8), (outs), (ins DGPR:$b, br_disp4:$disp4),
                      !tolower(NAME), "$b, $disp4">;
  def : CompressPat<(!cast<Instruction>(inststring#brc) D15, 0,
                        br_disp8:$disp8),
                    (!cast<Instruction>(NAME#sb) br_disp8:$disp8)>;
  def : CompressPat<(!cast<Instruction>(inststring#brc) DGPR:$b, 0,
                        br_disp4:$disp4),
                    (!cast<Instruction>(NAME#sbr) DGPR:$b, br_disp4:$disp4)>;
}
multiclass CBranchComparePlus16<bits<8> op1> {
  let isBranch = 1, isTerminator = 1 in {
    def sbc : TCInstSBC<op1, (outs), (ins simm4:$const4, br_disp4:$disp4),
                        !tolower(NAME), "d15, $const4, $disp4">;
    /* TODO: br_disp4p16 */
    def sbc16 : TCInstSBC<!add(op1, 0x80), (outs),
                          (ins simm4:$const4, br_disp4:$disp4), !tolower(NAME),
                          "d15, $const4, $disp4">;
    def sbr : TCInstSBR<!add(op1, 0x20), (outs), (ins DGPR:$b, br_disp4:$disp4),
                        !tolower(NAME), "$b, $disp4">;
    def sbr16
        : TCInstSBR<!add(op1, 0xA0), (outs), (ins DGPR:$b, br_disp4:$disp4),
                    !tolower(NAME), "$b, $disp4">;
  }
  def : CompressPat<
            (!cast<Instruction>(NAME#brc) D15, simm4:$const4, br_disp4:$disp4),
            (!cast<Instruction>(NAME#sbc) simm4:$const4, br_disp4:$disp4)>;
  def : CompressPat<
            (!cast<Instruction>(NAME#brc) D15, simm4:$const4, br_disp4:$disp4),
            (!cast<Instruction>(NAME#sbc16) simm4:$const4, br_disp4:$disp4)>;
  def : CompressPat<(!cast<Instruction>(NAME#brc) DGPR:$b, 0, br_disp4:$disp4),
                    (!cast<Instruction>(NAME#sbr) DGPR:$b, br_disp4:$disp4)>;
  def : CompressPat<(!cast<Instruction>(NAME#brc) DGPR:$b, 0, br_disp4:$disp4),
                    (!cast<Instruction>(NAME#sbr16) DGPR:$b, br_disp4:$disp4)>;
}
multiclass CBranchCompare<bits<8> op1, string inststr, int imm = 0> {
  let isBranch = 1, isTerminator = 1 in {
    def sbr : TCInstSBR<op1, (outs), (ins DGPR:$b, br_disp4:$disp4),
                        !tolower(NAME), "$b, $disp4">;
  }
  def : CompressPat<(!cast<Instruction>(inststr#brc) DGPR:$b, imm,
                        br_disp4:$disp4),
                    (!cast<Instruction>(NAME#sbr) DGPR:$b, br_disp4:$disp4)>;
}

defm CALL : CCall<0x5C, 0x1>;
defm J : CBranch<0x3C, 0x0>;
defm JZ : CBranchCompareZero<0x6E, "JEQ">;
defm JNZ : CBranchCompareZero<0xEE, "JNE">;
defm JEQ : CBranchComparePlus16<0x1E>;
defm JGEZ : CBranchCompare<0xCE, "JGE">;
defm JGTZ : CBranchCompare<0x4E, "JGE", 1>;
defm JLEZ : CBranchCompare<0x8E, "JLT", 1>;
defm JLTZ : CBranchCompare<0x0E, "JLT">;
defm JNE : CBranchComparePlus16<0x5E>;
// JNZ_A
// JNZ_T
// JZ_A
// JZ_T
// LOOP

// Load store
//*

// System instruction
multiclass CSys<bits<4> op2, string inststring> {
  def sr : TCInstSR<0x00, op2, (outs), (ins),
                    !cast<TCInstCommon>(inststring).OpcodeStr, "">;
  def : CompressPat<(!cast<Instruction>(inststring)),
                    (!cast<Instruction>(NAME#sr))>;
}
// BISR
// defm DEBUGsr : CSys<0x0, "DEBUG">;
// defm FRETsr : CSys<0x7, "FRET">;
// defm NOPsr : CSYS<0x0, "NOP">;
// defm RET : CSys<0x9, "RET">;
// defm RFEsr : CSYS<0x8, "RFE">;
// TrapINV