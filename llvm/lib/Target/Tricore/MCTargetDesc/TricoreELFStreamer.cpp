//===-------- TricoreELFStreamer.cpp - ELF Object Output ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TricoreELFStreamer.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/Casting.h"

using namespace llvm;

TricoreELFStreamer::TricoreELFStreamer(MCContext &Context,
                                 std::unique_ptr<MCAsmBackend> MAB,
                                 std::unique_ptr<MCObjectWriter> OW,
                                 std::unique_ptr<MCCodeEmitter> Emitter)
    : MCELFStreamer(Context, std::move(MAB), std::move(OW),
                    std::move(Emitter)) {
}

MCELFStreamer *
llvm::createTricoreELFStreamer(MCContext &Context,
                            std::unique_ptr<MCAsmBackend> MAB,
                            std::unique_ptr<MCObjectWriter> OW,
                            std::unique_ptr<MCCodeEmitter> Emitter) {
  return new TricoreELFStreamer(Context, std::move(MAB), std::move(OW),
                             std::move(Emitter));
}
