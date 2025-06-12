//===- IntrinsicsTricore.td - Defines Tricore intrinsics ---------*- tablegen -*-===//
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


let TargetPrefix = "tricore" in {  // All intrinsics start with "llvm.tricore.".

  def int_tricore_mtcr : ClangBuiltin<"__builtin_tricore_mtcr">,
                     DefaultAttrsIntrinsic<[], [llvm_i32_ty, llvm_i32_ty],
                                           [IntrNoMem, IntrHasSideEffects, ImmArg<ArgIndex<0>>]>;

}
