; RUN: llc -mtriple=tricore -mcpu=tc162 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -O0 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -filetype=obj < %s -o %t
; RUN: llvm-objdump -s --section=.text %t | FileCheck %s --check-prefix=BYTES
; RUN: llc -mtriple=tricore -mcpu=tc162 -O0 -filetype=obj < %s -o %t
; RUN: llvm-objdump -s --section=.text %t | FileCheck %s --check-prefix=BYTES

; LEA must not encode as SWAP.W (49 aa 38 f0), which overwrites the
; caller's stack memory with D10 before adjusting SP.
; CHECK-LABEL: small_frame:
; CHECK: lea a10, a10, -8
; BYTES: 49aa38fa
define void @small_frame() {
  %slot = alloca [8 x i8], align 8
  call void @use_slot(ptr %slot)
  ret void
}

declare void @use_slot(ptr)
