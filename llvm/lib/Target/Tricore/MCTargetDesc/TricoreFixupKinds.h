//===-- TricoreFixupKinds.h - Tricore Specific Fixup Entries ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRICORE_MCTARGETDESC_TRICOREFIXUPKINDS_H
#define LLVM_LIB_TARGET_TRICORE_MCTARGETDESC_TRICOREFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace Tricore {
  // Although most of the current fixup types reflect a unique relocation
  // one can have multiple fixup types for a given relocation and thus need
  // to be uniquely named.
  //
  // This table *must* be in the same order of
  // MCFixupKindInfo Infos[Tricore::NumTargetFixupKinds]
  // in TricoreAsmBackend.cpp.
  //
  enum Fixups {
    fixup_tricore_lo = FirstTargetFixupKind,
    fixup_tricore_hi,
    fixup_tricore_rel24,
    fixup_tricore_abs24,

    fixup_tricore_branch15,
    // Marker
    fixup_tricore_invalid,
    NumTargetFixupKinds = fixup_tricore_invalid - FirstTargetFixupKind
  };
} // namespace Tricore
} // namespace llvm


#endif
