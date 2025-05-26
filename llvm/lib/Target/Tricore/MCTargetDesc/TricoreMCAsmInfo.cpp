//===- TricoreMCAsmInfo.cpp - Tricore asm properties --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the TricoreMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "TricoreMCAsmInfo.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/TargetParser/Triple.h"

using namespace llvm;

void TricoreELFMCAsmInfo::anchor() {}

TricoreELFMCAsmInfo::TricoreELFMCAsmInfo(const Triple &TheTriple) {
  
  IsLittleEndian = true;
  CodePointerSize = CalleeSaveStackSlotSize = 8;

  Data16bitsDirective = "\t.half\t";
  Data32bitsDirective = "\t.word\t";
  // .xword is only supported by V9.
  Data64bitsDirective = "\t.xword\t";
  ZeroDirective = "\t.skip\t";
  CommentString = "!";
  SupportsDebugInformation = true;

  ExceptionsType = ExceptionHandling::DwarfCFI;

  UsesELFSectionDirectiveForBSS = true;
}