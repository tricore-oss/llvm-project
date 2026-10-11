; RUN: llc -mtriple=tricore -mcpu=tc162 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -O0 -verify-machineinstrs < %s | FileCheck %s

; Keep more scalar values live across a call than the preserved register bank
; can hold. Their spills must remain 32-bit, not use unrelated register pairs.
; CHECK-LABEL: word_spills:
; CHECK-NOT: st.d
; CHECK: st.w [a10]
; CHECK-NOT: st.d
; CHECK: call touch
; CHECK-NOT: ld.d
; CHECK: ld.w {{d[0-9]+}}, [a10]
; CHECK-NOT: ld.d
; CHECK: ret
define i32 @word_spills(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e,
                       i32 %f, i32 %g, i32 %h, i32 %i, i32 %j) {
  call void @touch()
  %ab = add i32 %a, %b
  %abc = add i32 %ab, %c
  %abcd = add i32 %abc, %d
  %abcde = add i32 %abcd, %e
  %abcdef = add i32 %abcde, %f
  %abcdefg = add i32 %abcdef, %g
  %abcdefgh = add i32 %abcdefg, %h
  %abcdefghi = add i32 %abcdefgh, %i
  %result = add i32 %abcdefghi, %j
  store volatile i32 %result, ptr @sink
  ret i32 %result
}

; Pointer spills must also keep their scalar register class. In particular,
; spilling address-register pairs using data-register instructions is invalid.
; CHECK-LABEL: pointer_spills:
; CHECK-NOT: st.d
; CHECK: call touch
; CHECK-NOT: ld.d
; CHECK: ret
define i32 @pointer_spills(ptr %a, ptr %b, ptr %c, ptr %d, ptr %e,
                          ptr %f, ptr %g, ptr %h, ptr %i, ptr %j) {
  call void @touch()
  %av = load volatile i32, ptr %a
  %bv = load volatile i32, ptr %b
  %cv = load volatile i32, ptr %c
  %dv = load volatile i32, ptr %d
  %ev = load volatile i32, ptr %e
  %fv = load volatile i32, ptr %f
  %gv = load volatile i32, ptr %g
  %hv = load volatile i32, ptr %h
  %iv = load volatile i32, ptr %i
  %jv = load volatile i32, ptr %j
  %ab = add i32 %av, %bv
  %abc = add i32 %ab, %cv
  %abcd = add i32 %abc, %dv
  %abcde = add i32 %abcd, %ev
  %abcdef = add i32 %abcde, %fv
  %abcdefg = add i32 %abcdef, %gv
  %abcdefgh = add i32 %abcdefg, %hv
  %abcdefghi = add i32 %abcdefgh, %iv
  %result = add i32 %abcdefghi, %jv
  ret i32 %result
}

@sink = external global i32
declare void @touch()
