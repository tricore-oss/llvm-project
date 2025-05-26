//===-- TricoreMCTargetDesc.h - Tricore Target Descriptions -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Tricore specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRICORE_MCTARGETDESC_TRICOREMCTARGETDESC_H
#define LLVM_LIB_TARGET_TRICORE_MCTARGETDESC_TRICOREMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCObjectWriter;
class MCRegisterInfo;
class MCStreamer;
class MCSubtargetInfo;
class MCTargetOptions;
class StringRef;
class Target;
class Triple;

MCCodeEmitter *createTricoreMCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx);

MCAsmBackend *createTricoreAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                   const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options);


/// Construct a Tricore ELF object writer.
std::unique_ptr<MCObjectTargetWriter>
createTricoreELFObjectWriter(uint8_t OSABI);

namespace TRICORE_MC {
void initLLVMToCVRegMapping(MCRegisterInfo *MRI);

StringRef selectTricoreCPU(const Triple &TT, StringRef CPU);
}

} // End llvm namespace

// Defines symbolic names for Tricore registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "TricoreGenRegisterInfo.inc"

// Defines symbolic names for the Tricore instructions.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "TricoreGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "TricoreGenSubtargetInfo.inc"

#endif
