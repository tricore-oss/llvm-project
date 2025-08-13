//===- IntrinsicsTricore.td - Defines Tricore intrinsics ---------*- tablegen
//-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the Tricore-specific intrinsics.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//

let TargetPrefix = "tricore" in { // All intrinsics start with "llvm.tricore.".

  def int_tricore_mfcr : ClangBuiltin<"__builtin_tricore_mfcr">,
                         DefaultAttrsIntrinsic<[llvm_i32_ty], [llvm_i32_ty],
                                               [IntrNoMem, IntrHasSideEffects,
                                                ImmArg<ArgIndex<0>>]>;
  def int_tricore_mtcr : ClangBuiltin<"__builtin_tricore_mtcr">,
                         DefaultAttrsIntrinsic<[], [llvm_i32_ty, llvm_i32_ty],
                                               [IntrNoMem, IntrHasSideEffects,
                                                ImmArg<ArgIndex<0>>]>;
  /* Bit manipulation */
  def int_tricore_insert
      : ClangBuiltin<"__builtin_tricore_insert">,
        DefaultAttrsIntrinsic<
            [llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty, llvm_i32_ty, llvm_i32_ty],
            [IntrNoMem]>;

  def int_tricore_enable : ClangBuiltin<"__builtin_tricore_enable">,
                           DefaultAttrsIntrinsic<[], [], [IntrHasSideEffects]>;
  def int_tricore_disable
      : ClangBuiltin<"__builtin_tricore_disable">,
        DefaultAttrsIntrinsic<[llvm_i32_ty], [], [IntrHasSideEffects]>;
  def int_tricore_restore
      : ClangBuiltin<"__builtin_tricore_restore">,
        DefaultAttrsIntrinsic<[], [llvm_i32_ty], [IntrHasSideEffects]>;
  /* System functions */
  def int_tricore_dsync : ClangBuiltin<"__builtin_tricore_dsync">,
                          DefaultAttrsIntrinsic<[], [], [IntrWriteMem]>;
  def int_tricore_isync : ClangBuiltin<"__builtin_tricore_isync">,
                          DefaultAttrsIntrinsic<[], [], [IntrHasSideEffects]>;
  def int_tricore_syscall : ClangBuiltin<"__builtin_tricore_syscall">,
                          DefaultAttrsIntrinsic<[], [], [IntrWriteMem, IntrNoMerge, IntrNoDuplicate]>;
}
