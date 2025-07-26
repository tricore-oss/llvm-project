; RUN: llc -mtriple=tricore < %s | FileCheck %s

; CHECK-LABEL: add_r
; CHECK: add d2, d4, d5
define i32 @add_r(i32 %a, i32 %b) nounwind {
entry:
  %v = add i32 %a, %b
  ret i32 %v
}

; CHECK-LABEL: add_i
; CHECK: add d2, d4, 13
define i32 @add_i(i32 %a) nounwind {
  %v = add i32 %a, 13
  ret i32 %v
}

; CHECK-LABEL: add_i16
; CHECK: addi d2, d4, 32323
define i32 @add_i16(i32 %a) nounwind {
  %v = add i32 %a, 32323
  ret i32 %v
}