//==- BuiltinsTricore.td - Tricore Builtin function database ---*- tablegen -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the Tricore specific builtin function database.
//
//===----------------------------------------------------------------------===//

include "clang/Basic/BuiltinsBase.td"

class TricoreBuiltin<string prototype, string features = ""> : TargetBuiltin {
  let Spellings = ["__builtin_tricore_" # NAME];
  let Prototype = prototype;
  let Features = features;
}

let Attributes = [NoThrow, Const] in {
  def mtcr : TricoreBuiltin<"void(unsigned int, unsigned int)">;
  def insert : TricoreBuiltin<"unsigned int(unsigned int, unsigned int, unsigned int, unsigned int)">;
}