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
  /* Core registers */
  def mtcr : TricoreBuiltin<"void(_Constant unsigned int, unsigned int)">;
  def mfcr : TricoreBuiltin<"unsigned int(_Constant unsigned int)">;
  let Features = "TC18" in {
    def mtdcr : TricoreBuiltin<"void(_Constant unsigned int, unsigned long long int)">;
    def mfdcr : TricoreBuiltin<"unsigned long long int(_Constant unsigned int)">;
  }

  /* Bit manipulation */
  def insert : TricoreBuiltin<"unsigned int(unsigned int, unsigned int, unsigned int, _Constant unsigned int)">;
  def extr : TricoreBuiltin<"int(int, unsigned int, unsigned int)">;
  def extr_u : TricoreBuiltin<"unsigned int(unsigned int, unsigned int, unsigned int)">;
  def dextr_u : TricoreBuiltin<"unsigned int(unsigned long long int, unsigned int, unsigned int)">;
  def shuffle : TricoreBuiltin<"unsigned int(unsigned int, _Constant unsigned int)">;
  def imask : TricoreBuiltin<"long long int(unsigned int, unsigned int, _Constant unsigned int)">;

  /* Atomits */
  def cmpswap_w : TricoreBuiltin<"int(int *, int, int)">;
  def ldmst : TricoreBuiltin<"void(int *, int, int)">;
  def imask_ldmst : TricoreBuiltin<"void(int *, int, unsigned int, _Constant unsigned int)">;
  def swap_w : TricoreBuiltin<"void(int *, int)">;
  def swapmsk_w : TricoreBuiltin<"void(int*, int, unsigned int)">;
  def imask_swapmsk_w : TricoreBuiltin<"void(int*, int, unsigned int, _Constant unsigned int)">;

  /* Interrupts */
  def enable : TricoreBuiltin<"void()">;
  def disable : TricoreBuiltin<"void()">;
  def restore : TricoreBuiltin<"void(unsigned int)">;

  /* System operations */
  def dsync : TricoreBuiltin<"void()">;
  def isync : TricoreBuiltin<"void()">;
  def syscall : TricoreBuiltin<"void(_Constant unsigned int)">;
  def hvcall : TricoreBuiltin<"void(_Constant unsigned int)">;
  def nop : TricoreBuiltin<"void()">;
  def wait : TricoreBuiltin<"void()">;
}