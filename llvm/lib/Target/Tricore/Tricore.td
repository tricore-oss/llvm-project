//===-- Tricore.td - Describe the Tricore Target Machine ---------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the Tricore target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"


//===----------------------------------------------------------------------===//
// Tricore Subtarget features                                                    //
//===----------------------------------------------------------------------===//
def FeatureFloat32
    : SubtargetFeature<"SignleFloat", "HasSingle", "true", "Implements Single Float">;

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "TricoreRegisterInfo.td"
include "TricoreSchedule.td"
include "TricoreInstrInfo.td"
include "TricoreCallingConv.td"
include "TricoreRegisterBanks.td"
include "TricoreCombine.td"

// Avoid forward declaration issues.
include "TricoreScheduleGeneric.td"

def TricoreInstrInfo : InstrInfo {
}

//===----------------------------------------------------------------------===//
// Tricore processors supported.
//===----------------------------------------------------------------------===//


def TC16 : ProcessorModel<"tc16", NoSchedModel, [FeatureFloat32]>;

def TricoreAsmParser : AsmParser {
}


def TricoreAsmWriter : AsmWriter {
  int PassSubtarget = 1;
}

def Tricore : Target {
  let InstructionSet = TricoreInstrInfo;
  let AssemblyWriters = [TricoreAsmWriter];
  let AssemblyParsers = [TricoreAsmParser];
}
