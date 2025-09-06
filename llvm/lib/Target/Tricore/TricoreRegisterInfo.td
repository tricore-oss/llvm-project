//===-- TricoreRegisterInfo.td - Tricore Register defs -----------*- tablegen
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the TRICORE register file
//===----------------------------------------------------------------------===//
let Namespace = "Tricore" in {
  def sub_even : SubRegIndex<32>;
  def sub_odd : SubRegIndex<32, 32>;
}

class TricoreReg<bits<16> Enc, string n, list<string> altNames = []>
    : Register<n, altNames> {
  let HWEncoding = Enc;
  let Namespace = "Tricore";
}

class TricoreRegWithSubRegs<bits<16> Enc, string n, list<Register> subregs>
    : RegisterWithSubRegs<n, subregs> {
  let HWEncoding = Enc;
  let Namespace = "Tricore";
  let SubRegIndices = [sub_even, sub_odd];
  let CoveredBySubRegs = 1;
}

class TricoreProcStateReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Tricore";
}

// Tricore CPU Registers.
class TricoreDataReg<bits<16> Enc, string n> : TricoreReg<Enc, n>;
class TricoreExtDataReg<bits<16> Enc, string n, list<Register> subregs>
    : TricoreRegWithSubRegs<Enc, n, subregs>;
class TricoreAddrReg<bits<16> Enc, string n, list<string> altNames = []>
    : TricoreReg<Enc, n, altNames>;
class TricoreExtAddrReg<bits<16> Enc, string n, list<Register> subregs>
    : TricoreRegWithSubRegs<Enc, n, subregs>;

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//

// Data Registers
def D0 : TricoreDataReg<0, "d0">, DwarfRegNum<[0]>;
def D1 : TricoreDataReg<1, "d1">, DwarfRegNum<[1]>;
def D2 : TricoreDataReg<2, "d2">, DwarfRegNum<[2]>;
def D3 : TricoreDataReg<3, "d3">, DwarfRegNum<[3]>;
def D4 : TricoreDataReg<4, "d4">, DwarfRegNum<[4]>;
def D5 : TricoreDataReg<5, "d5">, DwarfRegNum<[5]>;
def D6 : TricoreDataReg<6, "d6">, DwarfRegNum<[6]>;
def D7 : TricoreDataReg<7, "d7">, DwarfRegNum<[7]>;
def D8 : TricoreDataReg<8, "d8">, DwarfRegNum<[8]>;
def D9 : TricoreDataReg<9, "d9">, DwarfRegNum<[9]>;
def D10 : TricoreDataReg<10, "d10">, DwarfRegNum<[10]>;
def D11 : TricoreDataReg<11, "d11">, DwarfRegNum<[11]>;
def D12 : TricoreDataReg<12, "d12">, DwarfRegNum<[12]>;
def D13 : TricoreDataReg<13, "d13">, DwarfRegNum<[13]>;
def D14 : TricoreDataReg<14, "d14">, DwarfRegNum<[14]>;
def D15 : TricoreDataReg<15, "d15">, DwarfRegNum<[15]>;

// Ext Data registers
def E0 : TricoreExtDataReg<0, "e0", [D0, D1]>, DwarfRegNum<[0]>;
def E2 : TricoreExtDataReg<2, "e2", [D2, D3]>, DwarfRegNum<[2]>;
def E4 : TricoreExtDataReg<4, "e4", [D4, D5]>, DwarfRegNum<[4]>;
def E6 : TricoreExtDataReg<6, "e6", [D6, D7]>, DwarfRegNum<[6]>;
def E8 : TricoreExtDataReg<8, "e8", [D8, D9]>, DwarfRegNum<[8]>;
def E10 : TricoreExtDataReg<10, "e10", [D10, D11]>, DwarfRegNum<[10]>;
def E12 : TricoreExtDataReg<12, "e12", [D12, D13]>, DwarfRegNum<[12]>;
def E14 : TricoreExtDataReg<14, "e14", [D14, D15]>, DwarfRegNum<[14]>;

// Address registers
def A0 : TricoreDataReg<16, "a0">, DwarfRegNum<[16]>;
def A1 : TricoreAddrReg<17, "a1">, DwarfRegNum<[17]>;
def A2 : TricoreAddrReg<18, "a2">, DwarfRegNum<[18]>;
def A3 : TricoreAddrReg<19, "a3">, DwarfRegNum<[19]>;
def A4 : TricoreAddrReg<20, "a4">, DwarfRegNum<[20]>;
def A5 : TricoreAddrReg<21, "a5">, DwarfRegNum<[21]>;
def A6 : TricoreAddrReg<22, "a6">, DwarfRegNum<[22]>;
def A7 : TricoreAddrReg<23, "a7">, DwarfRegNum<[23]>;
def A8 : TricoreAddrReg<24, "a8">, DwarfRegNum<[24]>;
def A9 : TricoreAddrReg<25, "a9">, DwarfRegNum<[25]>;
def A10 : TricoreAddrReg<26, "a10", ["sp"]>, DwarfRegNum<[26]>;
def A11 : TricoreAddrReg<27, "a11", ["ra"]>, DwarfRegNum<[27]>;
def A12 : TricoreAddrReg<28, "a12">, DwarfRegNum<[28]>;
def A13 : TricoreAddrReg<29, "a13">, DwarfRegNum<[29]>;
def A14 : TricoreAddrReg<30, "a14">, DwarfRegNum<[30]>;
def A15 : TricoreAddrReg<31, "a15">, DwarfRegNum<[31]>;

// Ext Addr registers
def P0 : TricoreExtAddrReg<16, "p0", [A0, A1]>, DwarfRegNum<[16]>;
def P2 : TricoreExtAddrReg<18, "p2", [A2, A3]>, DwarfRegNum<[18]>;
def P4 : TricoreExtAddrReg<20, "p4", [A4, A5]>, DwarfRegNum<[20]>;
def P6 : TricoreExtAddrReg<22, "p6", [A6, A7]>, DwarfRegNum<[22]>;
def P8 : TricoreExtAddrReg<24, "p8", [A8, A9]>, DwarfRegNum<[24]>;
def P10 : TricoreExtAddrReg<26, "p10", [A10, A11]>, DwarfRegNum<[26]>;
def P12 : TricoreExtAddrReg<28, "p12", [A12, A13]>, DwarfRegNum<[28]>;
def P14 : TricoreExtAddrReg<30, "p14", [A14, A15]>, DwarfRegNum<[30]>;

// Programm state registers
def PC : TricoreProcStateReg<8, "pc">;
def PSW : TricoreProcStateReg<4, "psw">;
def PCXI : TricoreProcStateReg<0, "pcxi">;

def DGPR
    : RegisterClass<"Tricore", [i32, i16, i8, f32], 32,
                    (add D15, (sequence "D%u", 8, 14), (sequence "D%u", 0, 7))>;
def AGPR
    : RegisterClass<"Tricore", [i32], 32,
                    (add A15, (sequence "A%u", 12, 14), (sequence "A%u", 4, 7),
                        (sequence "A%u", 8, 11), A1, A0)>;
def EGPR : RegisterClass<"Tricore", [v2i32, i64, f64], 64,
                         (add(sequence "E%u", 8, 13, 2), E14,
                             (sequence "E%u", 0, 7, 2))>;
def PGPR : RegisterClass<"Tricore", [i64], 64, (sequence "P%u", 0, 15, 2)>;

def DGPR15 : RegisterClass<"Tricore", [i32], 32, (add D15)>;