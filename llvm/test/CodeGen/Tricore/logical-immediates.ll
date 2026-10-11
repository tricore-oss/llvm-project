; RUN: llc -mtriple=tricore -mcpu=tc162 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -O0 -verify-machineinstrs < %s | FileCheck %s

; memset must align using arithmetic negation, not bitwise complement.
; CHECK-LABEL: alignment_delta:
; CHECK-NOT: andn
; CHECK: rsub
; CHECK: and {{.*}}, 3
; CHECK: ret
define i32 @alignment_delta(i32 %address) {
  %negative = sub i32 0, %address
  %delta = and i32 %negative, 3
  ret i32 %delta
}

; CHECK-LABEL: bulk_length:
; CHECK-NOT: and {{.*}}, -4
; CHECK-NOT: and {{.*}}, 508
; CHECK: mov {{.*}}, -4
; CHECK: and {{.*}}, d{{[0-9]+}}
; CHECK: ret
define i32 @bulk_length(i32 %length) {
  %bulk = and i32 %length, -4
  ret i32 %bulk
}

; CHECK-LABEL: and_511:
; CHECK: and {{.*}}, 511
; CHECK: ret
define i32 @and_511(i32 %x) {
  %r = and i32 %x, 511
  ret i32 %r
}

; CHECK-LABEL: or_511:
; CHECK: or {{.*}}, 511
; CHECK: ret
define i32 @or_511(i32 %x) {
  %r = or i32 %x, 511
  ret i32 %r
}

; CHECK-LABEL: xor_511:
; CHECK: xor {{.*}}, 511
; CHECK: ret
define i32 @xor_511(i32 %x) {
  %r = xor i32 %x, 511
  ret i32 %r
}

; CHECK-LABEL: complement_and:
; CHECK: andn
; CHECK: ret
define i32 @complement_and(i32 %x, i32 %y) {
  %inverted = xor i32 %y, -1
  %r = and i32 %x, %inverted
  ret i32 %r
}

; CHECK-LABEL: negate_or:
; CHECK-NOT: orn
; CHECK: rsub
; CHECK: or
; CHECK: ret
define i32 @negate_or(i32 %x, i32 %y) {
  %negative = sub i32 0, %y
  %r = or i32 %x, %negative
  ret i32 %r
}

; CHECK-LABEL: complement_or:
; CHECK: orn
; CHECK: ret
define i32 @complement_or(i32 %x, i32 %y) {
  %inverted = xor i32 %y, -1
  %r = or i32 %x, %inverted
  ret i32 %r
}

