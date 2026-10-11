; RUN: llc -mtriple=tricore -mcpu=tc162 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -O0 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -filetype=obj < %s -o %t
; RUN: llvm-objdump -s --section=.text %t | FileCheck %s --check-prefix=BYTES

; The invalid compressed CALLI used DC with op2=1. Only JI has a DC form.
; CHECK-LABEL: indirect_call:
; CHECK: calli a{{[0-9]+}}
; CHECK: ret
; BYTES: 2d0f0000
define void @indirect_call(ptr %function, ptr %context) {
  call void %function(ptr %context)
  ret void
}
