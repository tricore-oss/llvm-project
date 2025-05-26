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
class InstFormat<bits<5> val> { bits<5> Value = val; }
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
def InstFormatSC : InstFormat<26>;

class TCInstCommon<dag outs, dag ins, string opcodestr, string argstr,
                   list<dag> pattern, InstFormat format> : Instruction {
  let Namespace = "Tricore";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = opcodestr#!if(!empty(argstr), "", "\t"#argstr);
  let Pattern = pattern;

  let TSFlags{4 -0} = format.Value;
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
  bits<16> imm;

  let Inst{31 - 28} = c;
  let Inst{27 - 12} = imm;
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

class TCInstBO<bits<8> op1, bits<6> op2, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst32<outs, ins, opcodestr, argstr, pattern, InstFormatABS> {
  bits<4> a;
  bits<4> b;
  bits<10> imm;

  let Inst{31 - 28} = imm{9 - 6};
  let Inst{27 - 22} = op2;
  let Inst{21 - 16} = imm{5 - 0};
  let Inst{15 - 12} = b;
  let Inst{11 - 8} = a;
  let Inst{7 - 0} = op1;
}

class TCInstSC<bits<8> op1, dag outs, dag ins, list<dag> pattern,
               string opcodestr, string argstr = "">
    : TCInst16<outs, ins, opcodestr, argstr, pattern, InstFormatSC> {
  bits<8> const8;

  let Inst{15 - 8} = const8;
  let Inst{7 - 0} = op1;
}
