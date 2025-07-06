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
// Tricore Subtarget features                                                 //
//===----------------------------------------------------------------------===//
def FeatureTC131 : SubtargetFeature<"tc131", "ArchType", "TC131", "Tricore 1.3.1 Instruction Set">;
def FeatureTC16 : SubtargetFeature<"tc16", "ArchType", "TC16", "Tricore 1.6 Instruction Set", [FeatureTC131]>;
def FeatureTC162 : SubtargetFeature<"tc162", "ArchType", "TC162", "Tricore 1.6.2 Instruction Set", [FeatureTC16]>;
def FeatureTC18 : SubtargetFeature<"tc18", "ArchType", "TC18", "Tricore 1.8 Instruction Set", [FeatureTC162]>;

def FeatureVirt : SubtargetFeature<"virt", "HasVirtualization", "true", "Virutalization Instructions">;
def FeatureCRC : SubtargetFeature<"crc", "HasCRC", "true", "Implements CRC Instructions">;

def FeatureSoftFloat : SubtargetFeature<"soft-float", "FPType", "FP_Soft", "Implements Soft Float">;
def FeatureSingleFloat : SubtargetFeature<"single-float", "FPType", "FP_Single", "Implements Single Float">;
def FeatureDoubleFloat : SubtargetFeature<"double-float", "FPType", "FP_Double", "Implements Double Float">;

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

def TC13 : ProcessorModel<"tc13", NoSchedModel, []>;
def TC131 : ProcessorModel<"tc131", NoSchedModel, [FeatureTC131]>;
def TC16 : ProcessorModel<"tc16", NoSchedModel, [FeatureTC16]>;
def TC162 : ProcessorModel<"tc162", NoSchedModel, [FeatureTC162]>;
def TC18 : ProcessorModel<"tc18", NoSchedModel, [FeatureTC18]>;

def TricoreAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
}

def TricoreAsmWriter : AsmWriter {
  int PassSubtarget = 1;
}

def Tricore : Target {
  let InstructionSet = TricoreInstrInfo;
  let AssemblyWriters = [TricoreAsmWriter];
  let AssemblyParsers = [TricoreAsmParser];
}
