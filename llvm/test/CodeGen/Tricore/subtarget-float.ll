; RUN: llc -mtriple=tricore -mcpu=tc162 -mattr=+soft-float -verify-machineinstrs < %s | FileCheck %s --check-prefix=SOFT
; RUN: llc -mtriple=tricore -mcpu=tc162 -mattr=+single-float -verify-machineinstrs < %s | FileCheck %s --check-prefix=HARD

; Feature state must agree with the legalization tables built by the subtarget.
; SOFT-LABEL: add_float:
; SOFT: call __addsf3
; SOFT: ret
; HARD-LABEL: add_float:
; HARD: add.f
; HARD-NOT: call
; HARD: ret
define float @add_float(float %x, float %y) {
  %sum = fadd float %x, %y
  ret float %sum
}
