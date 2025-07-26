/*
 * op0[0] = 1 32bit, 0 16bit
 * op0[1]
 * op0[2] = 1 32bit, 0 16bit
 * op0[3]
 * op0[4] = 1 32bit, 0 16bit
 * op0[5]
 * op0[6] = 1 co-processor, 0 no
 * op0[7]
 */

// Format specifies the encoding used by the instruction. This is used by
// TricoreMCCodeEmitter to determine which form of fixup to use. These
// definitions must be kept in-sync with TricoreBaseInfo.h.
class InstFormat<bits<6> val> { bits<6> Value = val; }
def InstFormatPseudo : InstFormat<0>;
def InstFormatABS : InstFormat<1>;
def InstFormatABSB : InstFormat<2>;
def InstFormatB : InstFormat<3>;
def InstFormatBIT : InstFormat<4>;
def InstFormatBO : InstFormat<5>;
def InstFormatBOL : InstFormat<6>;
def InstFormatBRC : InstFormat<7>;
def InstFormatBRN : InstFormat<8>;
def InstFormatBRR : InstFormat<9>;
def InstFormatRC : InstFormat<10>;
def InstFormatRCPW : InstFormat<11>;
def InstFormatRCR : InstFormat<12>;
def InstFormatRCRR : InstFormat<13>;
def InstFormatRCRW : InstFormat<14>;
def InstFormatRLC : InstFormat<15>;
def InstFormatRR : InstFormat<16>;
def InstFormatRR1 : InstFormat<17>;
def InstFormatRR2 : InstFormat<18>;
def InstFormatRRPW : InstFormat<19>;
def InstFormatRRR : InstFormat<20>;
def InstFormatRRR1 : InstFormat<21>;
def InstFormatRRR2 : InstFormat<22>;
def InstFormatRRRR : InstFormat<23>;
def InstFormatRRRW : InstFormat<24>;
def InstFormatSYS : InstFormat<25>;
def InstFormatSB : InstFormat<26>;
def InstFormatSBC : InstFormat<27>;
def InstFormatSBR : InstFormat<28>;
def InstFormatSBRN : InstFormat<29>;
def InstFormatSC : InstFormat<30>;
def InstFormatSLR : InstFormat<31>;
def InstFormatSLRO : InstFormat<32>;
def InstFormatSR : InstFormat<33>;
def InstFormatSRC : InstFormat<34>;
def InstFormatSRO : InstFormat<35>;
def InstFormatSRR : InstFormat<36>;
def InstFormatSRRS : InstFormat<37>;
def InstFormatSSR : InstFormat<38>;
def InstFormatSSRO : InstFormat<39>;

// Load / store index mode.
class IndexMode<bits<2> val> {
  bits<2> Value = val;
}
def IndexModePost : IndexMode<0>;
def IndexModePre  : IndexMode<1>;
def IndexModeNone : IndexMode<2>;

class TCInstCommon<dag outs, dag ins, string opcodestr, string argstr,
                   list<dag> pattern, InstFormat format> : Instruction {
  let Namespace = "Tricore";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = opcodestr#!if(!empty(argstr), "", "\t"#argstr);
  let Pattern = pattern;

  let TSFlags{5 - 0} = format.Value;
}

class TCInst16<dag outs, dag ins, string opcodestr, string argstr,
               list<dag> pattern, InstFormat format>
    : TCInstCommon<outs, ins, opcodestr, argstr, pattern, format> {
  field bits<16> Inst;
  // SoftFail is a field the disassembler can use to provide a way for
  // instructions to not match without killing the whole decode process. It is
  // mainly used for ARM, but Tablegen expects this field to exist or it fails
  // to build the decode table.
  field bits<16> SoftFail = 0;
  let Size = 2;
}

class TCInst32<dag outs, dag ins, string opcodestr, string argstr,
               list<dag> pattern, InstFormat format>
    : TCInstCommon<outs, ins, opcodestr, argstr, pattern, format> {
  field bits<32> Inst;
  field bits<32> SoftFail = 0;
  let Size = 4;
}

class Pseudo<dag outs, dag ins, list<dag> pattern, string opcodestr = "",
             string argstr = "">
    : TCInstCommon<outs, ins, opcodestr, argstr, pattern, InstFormatPseudo> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

//------------------------------------------------------------------------------
// 32 Bit instructions
//------------------------------------------------------------------------------

class TCInstB<bits<8> op1, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatB> {
  bits<24> disp24;

  let TSFlags{6} = op1{7};
  let Inst{31 - 16} = disp24{15 - 0};
  let Inst{15 - 8} = disp24{7 - 0};
  let Inst{7 -0} = op1;
}

class TCInstBO<bits<8> op1, bits<4> op2, IndexMode mode, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatBO> {
  bits<4> a;
  bits<4> b;
  bits<10> off10;

  let Inst{31 - 28} = off10{9 - 6};
  let Inst{27 - 26} = mode.Value;
  let Inst{25 - 22} = op2;
  let Inst{21 - 16} = off10{5 - 0};
  let Inst{15 - 12} = b;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}

class TCInstBOL<bits<8> op1, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRR> {
  bits<4> a;
  bits<4> b;
  bits<16> off16;

  let Inst{31 - 28} = off16{9 - 6};
  let Inst{27 - 22} = off16{15 - 10};
  let Inst{21 - 16} = off16{5 - 0};
  let Inst{15 - 12} = b;
  let Inst{11 - 8} = a;
  let Inst{7 -0} = op1;
}

class TCInstBRC<bits<8> op1, bits<1> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatBRR> {
  bits<4> a;
  bits<4> const4;
  bits<15> disp15;

  let Inst{31} = op2;
  let Inst{30 - 16} = disp15;
  let Inst{15 - 12} = const4;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}

class TCInstBRR<bits<8> op1, bits<1> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatBRR> {
  bits<4> a;
  bits<4> b;
  bits<15> disp15;

  let Inst{31} = op2;
  let Inst{30 - 16} = disp15;
  let Inst{15 - 12} = b;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}

class TCInstRC<bits<8> op1, bits<7> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRC> {
  bits<4> a;
  bits<9> const9;
  bits<4> c;

  let Inst{31 - 28} = c;
  let Inst{27 - 21} = op2;
  let Inst{20 - 12} = const9;
  let Inst{11 -8} = a;
  let Inst{7 -0} = op1;
}

class TCInstRCPW<bits<8> op1, bits<2> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRCPW> {
  bits<4> a;
  bits<4> c;
  bits<4> const4;
  bits<5> pos;
  bits<5> width;

  let Inst{31 - 28} = c;
  let Inst{27 - 23} = pos;
  let Inst{22 - 21} = op2;
  let Inst{20 - 16} = width;
  let Inst{15 - 12} = const4;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}


class TCInstRCR<bits<8> op1, bits<3> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRCR> {
  bits<4> a;
  bits<9> const9;
  bits<4> c;
  bits<4> d;

  let Inst{31 - 28} = c;
  let Inst{27 - 24} = d;
  let Inst{23 - 21} = op2;
  let Inst{20 - 12} = const9;
  let Inst{11 - 8} = a;
  let Inst{7 -0} = op1;
}

class TCInstRCRR<bits<8> op1, bits<3> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRCRR> {
  bits<4> a;
  bits<4> c;
  bits<4> d;
  bits<4> const4;

  let Inst{31 - 28} = c;
  let Inst{27 - 24} = d;
  let Inst{23 - 21} = op2;
  let Inst{15 - 12} = const4;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}

class TCInstRCRW<bits<8> op1, bits<3> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRCRW> {
  bits<4> a;
  bits<4> c;
  bits<4> d;
  bits<4> const4;
  bits<5> width;

  let Inst{31 - 28} = c;
  let Inst{27 - 24} = d;
  let Inst{23 - 21} = op2;
  let Inst{20 - 16} = width;
  let Inst{15 - 12} = const4;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}

class TCInstRR<bits<8> op1, bits<8> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRR> {
  bits<4> a;
  bits<4> b;
  bits<4> c;

  let Inst{31 - 28} = c;
  let Inst{27 - 20} = op2;
  let Inst{15 -12} = b;
  let Inst{11 -8} = a;
  let Inst{7 -0} = op1;
}

class TCInstRRPW<bits<8> op1, bits<2> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRRPW> {
  bits<4> a;
  bits<4> b;
  bits<4> c;
  bits<5> pos;
  bits<5> width;

  let Inst{31 - 28} = c;
  let Inst{27 - 23} = pos;
  let Inst{22 - 21} = op2;
  let Inst{20 - 16} = width;
  let Inst{15 - 12} = b;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}

class TCInstRRR<bits<8> op1, bits<4> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRRR> {
  bits<4> a;
  bits<4> b;
  bits<4> c;
  bits<4> d;

  let Inst{31 - 28} = c;
  let Inst{27 - 24} = d;
  let Inst{23 - 20} = op2;
  let Inst{15 -12} = b;
  let Inst{11 -8} = a;
  let Inst{7 -0} = op1;
}

class TCInstRRRR<bits<8> op1, bits<3> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRRRR> {
  bits<4> a;
  bits<4> b;
  bits<4> c;
  bits<4> d;

  let Inst{31 - 28} = c;
  let Inst{27 - 24} = d;
  let Inst{23 - 21} = op2;
  let Inst{15 - 12} = b;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}


class TCInstRRRW<bits<8> op1, bits<3> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatRRRW> {
  bits<4> a;
  bits<4> b;
  bits<4> c;
  bits<4> d;
  bits<5> width;

  let Inst{31 - 28} = c;
  let Inst{27 - 24} = d;
  let Inst{23 - 21} = op2;
  let Inst{20 - 16} = width;
  let Inst{15 - 12} = b;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}


class TCInstSYS<bits<8> op1, bits<6> op2, dag outs, dag ins, list<dag> pattern,
                string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatSYS> {
  bits<4> s1;

  let Inst{27 -22} = op2;
  let Inst{11 -8} = s1;
  let Inst{7 -0} = op1;
}

class TCInstRLC<bits<8> op1, dag outs, dag ins, list<dag> pattern,
                string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatSYS> {
  bits<4> c;
  bits<4> a;
  bits<16> const16;

  let Inst{31 - 28} = c;
  let Inst{27 - 12} = const16;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}

class TCInstABS<bits<8> op1, dag outs, dag ins, list<dag> pattern,
                string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatABS> {
  bits<4> a;
  bits<18> imm;

  let Inst{31 - 28} = imm{9 - 6};
  let Inst{27 -26} = 0;
  let Inst{25 - 22} = imm{13 - 10};
  let Inst{21 - 16} = imm{5 -0};
  let Inst{15 - 12} = imm{17 -14};
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}

//------------------------------------------------------------------------------
// 16bit instructions
//------------------------------------------------------------------------------

class TCInstSBC<bits<8> op1, dag outs, dag ins,
                string opcodestr, string argstr = "">
    : TCInst16<outs, ins, opcodestr, argstr, [], InstFormatSBC> {
  bits<4> const4;
  bits<4> disp4;

  let Inst{15 - 12} = const4;
  let Inst{11 - 8} = disp4;
  let Inst{7 - 0} = op1;
}

class TCInstSBR<bits<8> op1, dag outs, dag ins,
                string opcodestr, string argstr = "">
    : TCInst16<outs, ins, opcodestr, argstr, [], InstFormatSBR> {
  bits<4> b;
  bits<4> disp4;

  let Inst{15 - 12} = b;
  let Inst{11 - 8} = disp4;
  let Inst{7 - 0} = op1;
}


class TCInstSC<bits<8> op1, dag outs, dag ins,
               string opcodestr, string argstr = "">
    : TCInst16<outs, ins, opcodestr, argstr, [], InstFormatSC> {
  bits<8> const8;

  let Inst{15 - 8} = const8;
  let Inst{7 - 0} = op1;
}

class TCInstSRC<bits<8> op1, dag outs, dag ins,
                string opcodestr, string argstr = "">
    : TCInst16<outs, ins, opcodestr, argstr, [], InstFormatSRC> {
  bits<4> a;
  bits<4> const4;

  let Inst{15 - 12} = const4;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}


class TCInstSRR<bits<8> op1, dag outs, dag ins,
                string opcodestr, string argstr = "">
    : TCInst16<outs, ins, opcodestr, argstr, [], InstFormatSRR> {
  bits<4> a;
  bits<4> b;

  let Inst{15 - 12} = b;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}
