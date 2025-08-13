//===- RISCV.cpp ----------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "InputFiles.h"
#include "OutputSections.h"
#include "Relocations.h"
#include "Symbols.h"
#include "SyntheticSections.h"
#include "Target.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/Support/ELFAttributes.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/TimeProfiler.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {

class Tricore final : public TargetInfo {
public:
  Tricore(Ctx &);
  uint32_t calcEFlags() const override;
  int64_t getImplicitAddend(const uint8_t *buf, RelType type) const override;
  void writeGotHeader(uint8_t *buf) const override;
  void writeGotPlt(uint8_t *buf, const Symbol &s) const override;
  void writeIgotPlt(uint8_t *buf, const Symbol &s) const override;
  void writePltHeader(uint8_t *buf) const override;
  void writePlt(uint8_t *buf, const Symbol &sym,
                uint64_t pltEntryAddr) const override;
  RelType getDynRel(RelType type) const override;
  RelExpr getRelExpr(RelType type, const Symbol &s,
                     const uint8_t *loc) const override;
  void relocate(uint8_t *loc, const Relocation &rel,
                uint64_t val) const override;
  void relocateAlloc(InputSectionBase &sec, uint8_t *buf) const override;
  bool relaxOnce(int pass) const override;
  void finalizeRelax(int passes) const override;
};

} // end anonymous namespace

namespace {
enum Op {};

enum Reg {};
} // namespace

static uint32_t hi16(uint32_t val) { return (val + 0x8000) >> 16; }
static uint32_t lo16(uint32_t val) { return val & 4095; }

// Extract bits v[begin:end], where range is inclusive, and begin must be < 63.
static uint32_t extractBits(uint64_t v, uint32_t begin, uint32_t end) {
  return (v & ((1ULL << (begin + 1)) - 1)) >> end;
}

static uint32_t setRLC(uint32_t insn, uint32_t imm) {
  return (insn & 0xffff) | (imm << 16);
}
static uint32_t setBOL(uint32_t insn, uint32_t imm) {
  return (insn & 0xFFFF) | (extractBits(imm, 0, 5) << 16) |
         (extractBits(imm, 10, 15) << 22) | (extractBits(imm, 6, 9) << 28);
}

Tricore::Tricore(Ctx &ctx) : TargetInfo(ctx) {
  copyRel = R_TRICORE_COPY;
  pltRel = R_TRICORE_JUMP_SLOT;
  relativeRel = R_TRICORE_RELATIVE;
  // iRelativeRel = R_TRICORE_IRELATIVE;

  symbolicRel = R_TRICORE_32ABS;
  // tlsModuleIndexRel = R_RISCV_TLS_DTPMOD32;
  // tlsOffsetRel = R_RISCV_TLS_DTPREL32;
  // tlsGotRel = R_RISCV_TLS_TPREL32;

  gotRel = symbolicRel;
  // tlsDescRel = R_RISCV_TLSDESC;

  // .got[0] = _DYNAMIC
  gotHeaderEntriesNum = 1;

  // .got.plt[0] = _dl_runtime_resolve, .got.plt[1] = link_map
  gotPltHeaderEntriesNum = 2;

  pltHeaderSize = 32;
  pltEntrySize = 16;
  ipltEntrySize = 16;
}

static uint32_t getEFlags(Ctx &ctx, InputFile *f) {
  return cast<ObjFile<ELF32LE>>(f)->getObj().getHeader().e_flags;
}

uint32_t Tricore::calcEFlags() const {
  // If there are only binary input files (from -b binary), use a
  // value of 0 for the ELF header flags.
  if (ctx.objectFiles.empty())
    return 0;

  uint32_t target = getEFlags(ctx, ctx.objectFiles.front());
  for (InputFile *f : ctx.objectFiles) {
    uint32_t eflags = getEFlags(ctx, f);
  }

  return target;
}

int64_t Tricore::getImplicitAddend(const uint8_t *buf, RelType type) const {
  switch (type) {
  default:
    InternalErr(ctx, buf) << "cannot read addend for relocation " << type;
    return 0;
  }
}

void Tricore::writeGotHeader(uint8_t *buf) const {
  if (ctx.arg.is64)
    write64le(buf, ctx.mainPart->dynamic->getVA());
  else
    write32le(buf, ctx.mainPart->dynamic->getVA());
}

void Tricore::writeGotPlt(uint8_t *buf, const Symbol &s) const {
  if (ctx.arg.is64)
    write64le(buf, ctx.in.plt->getVA());
  else
    write32le(buf, ctx.in.plt->getVA());
}

void Tricore::writeIgotPlt(uint8_t *buf, const Symbol &s) const {
  if (ctx.arg.writeAddends) {
    if (ctx.arg.is64)
      write64le(buf, s.getVA(ctx));
    else
      write32le(buf, s.getVA(ctx));
  }
}

void Tricore::writePltHeader(uint8_t *buf) const { assert(false); }

void Tricore::writePlt(uint8_t *buf, const Symbol &sym,
                       uint64_t pltEntryAddr) const {
  assert(false);
}

RelType Tricore::getDynRel(RelType type) const {
  return type == ctx.target->symbolicRel ? type
                                         : static_cast<RelType>(R_TRICORE_NONE);
}

RelExpr Tricore::getRelExpr(const RelType type, const Symbol &s,
                            const uint8_t *loc) const {
  switch (type) {
  case R_TRICORE_32ABS:
    return R_ABS;
  case R_TRICORE_32REL:
    return R_PC;
  case R_TRICORE_24ABS:
    return R_ABS;
  case R_TRICORE_24REL:
    return R_PC;
  case R_TRICORE_HI:
  case R_TRICORE_LO:
  case R_TRICORE_LO2:
    return R_ABS;
  default:
    Err(ctx) << getErrorLoc(ctx, loc) << "unknown relocation (" << type.v
             << ") against symbol " << &s;
    return R_NONE;
  }
}

void Tricore::relocate(uint8_t *loc, const Relocation &rel,
                       uint64_t val) const {
  const unsigned bits = ctx.arg.wordsize * 8;

  switch (rel.type) {
  case R_TRICORE_32ABS: {
    checkInt(ctx, loc, val, 32, rel);
    write32le(loc, (uint32_t)val);
    break;
  }
  case R_TRICORE_24REL: {
    uint32_t disp24 = val >> 1;
    uint32_t inst = (read32le(loc) & 0xFF);
    checkAlignment(ctx, loc, val, 2, rel);
    checkInt(ctx, loc, val >> 1, 24, rel);
    write32le(loc,
              inst | ((disp24 & 0xFFFF) << 16) | ((disp24 & 0xFF0000) >> 8));
    break;
  }

  case R_TRICORE_HI: {
    uint32_t off16 = hi16(val);
    checkInt(ctx, loc, val, 32, rel);
    write32le(loc, setRLC(read32le(loc), off16));
    break;
  }
  case R_TRICORE_LO: {
    uint32_t off16 = lo16(val);
    checkInt(ctx, loc, val, 32, rel);
    write32le(loc, setRLC(read32le(loc), off16));
    break;
  }
  case R_TRICORE_LO2: {
    uint32_t off16 = lo16(val);
    checkInt(ctx, loc, val, 32, rel);
    write32le(loc, setBOL(read32le(loc), off16));
    break;
  }
  default:
    llvm_unreachable("unknown relocation");
  }
}

static bool relaxable(ArrayRef<Relocation> relocs, size_t i) {
  return false; // i + 1 != relocs.size() && relocs[i + 1].type ==
                // R_RISCV_RELAX;
}

void Tricore::relocateAlloc(InputSectionBase &sec, uint8_t *buf) const {
  uint64_t secAddr = sec.getOutputSection()->addr;
  if (auto *s = dyn_cast<InputSection>(&sec))
    secAddr += s->outSecOff;
  else if (auto *ehIn = dyn_cast<EhInputSection>(&sec))
    secAddr += ehIn->getParent()->outSecOff;
  uint64_t tlsdescVal = 0;
  bool tlsdescRelax = false, isToLe = false;
  const ArrayRef<Relocation> relocs = sec.relocs();
  for (size_t i = 0, size = relocs.size(); i != size; ++i) {
    const Relocation &rel = relocs[i];
    uint8_t *loc = buf + rel.offset;
    uint64_t val = sec.getRelocTargetVA(ctx, rel, secAddr + rel.offset);

    relocate(loc, rel, val);
  }
}

static bool relax(Ctx &ctx, InputSection &sec) {
  assert(false);
  return false;
}

bool Tricore::relaxOnce(int pass) const {
  llvm::TimeTraceScope timeScope("Tricore relaxOnce");
  if (ctx.arg.relocatable)
    return false;

  return false;
}

void Tricore::finalizeRelax(int passes) const {
  llvm::TimeTraceScope timeScope("Finalize Tricore relaxation");
  Log(ctx) << "relaxation passes: " << passes;
}

void elf::setTricoreTargetInfo(Ctx &ctx) { ctx.target.reset(new Tricore(ctx)); }
