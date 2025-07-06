//===--- Tricore.h - Tricore-specific Tool Helpers ------------------*- C++
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_TRICORE_H
#define LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_TRICORE_H

#include "clang/Driver/Driver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/Option.h"
#include "llvm/TargetParser/TricoreTargetParser.h"
#include <string>
#include <vector>

namespace clang {
namespace driver {
namespace tools {
namespace tricore {

enum class FloatABI {
  Invalid,
  Soft,
  Hard,
};

std::string getTricoreTargetCPU(const llvm::opt::ArgList &Args);
void getTricoreTargetFeatures(const Driver &D, const llvm::opt::ArgList &Args,
                              std::vector<llvm::StringRef> &Features);

FloatABI getTricoreFloatABI(const Driver &D, const llvm::opt::ArgList &Args);

} // end namespace tricore
} // end namespace tools
} // end namespace driver
} // end namespace clang

#endif // LLVM_CLANG_LIB_DRIVER_TOOLCHAINS_ARCH_TRICORE_H
