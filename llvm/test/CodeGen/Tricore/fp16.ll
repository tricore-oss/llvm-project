; RUN: llc -mtriple=tricore -mcpu=tc162 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -O0 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -mattr=+single-float -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -mattr=+single-float -O0 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -mattr=+double-float -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -mattr=+double-float -O0 -verify-machineinstrs < %s | FileCheck %s

; Half precision conversions need software helpers on TriCore.
; CHECK-LABEL: round_trip:
; CHECK: call __gnu_f2h_ieee
; CHECK: call __gnu_h2f_ieee
; CHECK: ret
define float @round_trip(float %x) {
  %h = fptrunc float %x to half
  %f = fpext half %h to float
  ret float %f
}

; CHECK-LABEL: extend_half:
; CHECK: call __gnu_h2f_ieee
; CHECK: ret
define float @extend_half(i16 %bits) {
  %h = bitcast i16 %bits to half
  %f = fpext half %h to float
  ret float %f
}

; CHECK-LABEL: truncate_half:
; CHECK: call __gnu_f2h_ieee
; CHECK: ret
define i16 @truncate_half(float %x) {
  %h = fptrunc float %x to half
  %bits = bitcast half %h to i16
  ret i16 %bits
}

; CHECK-LABEL: store_half:
; CHECK: call __gnu_f2h_ieee
; CHECK: st.h
; CHECK: ret
define void @store_half(ptr %p, float %x) {
  %h = fptrunc float %x to half
  store half %h, ptr %p, align 2
  ret void
}

; CHECK-LABEL: load_half:
; CHECK: ld.hu
; CHECK: call __gnu_h2f_ieee
; CHECK: ret
define float @load_half(ptr %p) {
  %h = load half, ptr %p, align 2
  %f = fpext half %h to float
  ret float %f
}
