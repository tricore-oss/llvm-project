; RUN: llc -mtriple=tricore -mcpu=tc162 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -O0 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -filetype=obj < %s -o %t
; RUN: llvm-objdump -s --section=.text %t | FileCheck %s --check-prefix=BYTES
; RUN: llc -mtriple=tricore -mcpu=tc162 -O0 -filetype=obj < %s -o %t
; RUN: llvm-objdump -s --section=.text %t | FileCheck %s --check-prefix=BYTES

; Integer DIV/DIV.U require bits 17:16 = 1. Textual assembly alone does
; not detect the missing selector in the object encoding.
; CHECK-LABEL: signed_quotient:
; CHECK: div e{{[02468]|1[024]}}, d{{[0-9]+}}, d{{[0-9]+}}
; BYTES: 4b{{[0-9a-f][0-9a-f]}}01{{[0-9a-f]}}2
define i32 @signed_quotient(i32 %a, i32 %b) align 16 {
  %result = sdiv i32 %a, %b
  ret i32 %result
}

; CHECK-LABEL: signed_remainder:
; CHECK: div e{{[02468]|1[024]}}, d{{[0-9]+}}, d{{[0-9]+}}
; BYTES: 4b{{[0-9a-f][0-9a-f]}}01{{[0-9a-f]}}2
define i32 @signed_remainder(i32 %a, i32 %b) align 16 {
  %result = srem i32 %a, %b
  ret i32 %result
}

; CHECK-LABEL: unsigned_quotient:
; CHECK: div.u e{{[02468]|1[024]}}, d{{[0-9]+}}, d{{[0-9]+}}
; BYTES: 4b{{[0-9a-f][0-9a-f]}}11{{[0-9a-f]}}2
define i32 @unsigned_quotient(i32 %a, i32 %b) align 16 {
  %result = udiv i32 %a, %b
  ret i32 %result
}

; CHECK-LABEL: unsigned_remainder:
; CHECK: div.u e{{[02468]|1[024]}}, d{{[0-9]+}}, d{{[0-9]+}}
; BYTES: 4b{{[0-9a-f][0-9a-f]}}11{{[0-9a-f]}}2
define i32 @unsigned_remainder(i32 %a, i32 %b) align 16 {
  %result = urem i32 %a, %b
  ret i32 %result
}
