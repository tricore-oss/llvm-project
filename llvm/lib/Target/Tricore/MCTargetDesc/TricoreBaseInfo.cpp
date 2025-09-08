#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCSubtargetInfo.h"

#include "TricoreBaseInfo.h"

using namespace llvm;

// Include the auto-generated portion of the compress emitter.
#define GEN_UNCOMPRESS_INSTR
#define GEN_COMPRESS_INSTR
#include "TricoreGenCompressInstEmitter.inc"

bool TricoreCI::compress(MCInst &OutInst, const MCInst &MI,
                         const MCSubtargetInfo &STI) {
  return compressInst(OutInst, MI, STI);
}

bool TricoreCI::uncompress(MCInst &OutInst, const MCInst &MI,
                           const MCSubtargetInfo &STI) {
  return uncompressInst(OutInst, MI, STI);
}