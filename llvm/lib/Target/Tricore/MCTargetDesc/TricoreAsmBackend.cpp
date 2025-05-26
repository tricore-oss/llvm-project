//===-- TricoreAsmBackend.cpp - Tricore Asm Backend
//----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the TricoreAsmBackend class.
//
//===----------------------------------------------------------------------===//
//

#include "MCTargetDesc/TricoreAsmBackend.h"
#include "MCTargetDesc/TricoreFixupKinds.h"
#include "MCTargetDesc/TricoreMCTargetDesc.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class ELFTricoreAsmBackend : public TricoreAsmBackend {
  Triple::OSType OSType;

public:
  ELFTricoreAsmBackend(const MCSubtargetInfo &STI, Triple::OSType OSType)
      : TricoreAsmBackend(STI), OSType(OSType) {}

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override {
    assert(false); // TODO: Implement this function
  }

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
    return createTricoreELFObjectWriter(OSABI);
  }
};

std::optional<MCFixupKind>
TricoreAsmBackend::getFixupKind(StringRef Name) const {
  assert(false);
}

const MCFixupKindInfo &
TricoreAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  assert(false);
}

bool TricoreAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                     const MCSubtargetInfo *STI) const {
  if (Count % 2) {
    OS.write("\0", 1);
    Count -= 1;
  }

  for (; Count >= 2; Count -= 2)
    OS.write("\0\0", 2);

  return true;
}

MCAsmBackend *llvm::createTricoreAsmBackend(const Target &T,
                                            const MCSubtargetInfo &STI,
                                            const MCRegisterInfo &MRI,
                                            const MCTargetOptions &Options) {
  return new ELFTricoreAsmBackend(STI, STI.getTargetTriple().getOS());
}
