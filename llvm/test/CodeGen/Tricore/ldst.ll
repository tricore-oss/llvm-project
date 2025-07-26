; RUN: llc -mtriple=tricore < %s | FileCheck %s

; CHECK-LABEL: load32
; CHECK: ld.w d2, [a4]4000
; CHECK: ret
define i32 @load32(ptr %bp) nounwind {
entry:
  %gep = getelementptr i32, ptr %bp, i32 1000
  %v = load i32, ptr %gep, align 4
  ret i32 %v
}

; CHECK-LABEL: loadu16
; CHECK: ld.hu d2, [a4]2000
; CHECK: ret
define i32 @loadu16(ptr %bp) nounwind {
entry:
  %gep = getelementptr i16, ptr %bp, i32 1000
  %0 = load i16, ptr %gep, align 2
  %1 = zext i16 %0 to i32
  ret i32 %1
}

; CHECK-LABEL: load16
; CHECK: ld.h d2, [a4]2000
; CHECK: ret
define i32 @load16(ptr %bp) nounwind {
entry:
  %gep = getelementptr i16, ptr %bp, i32 1000
  %0 = load i16, ptr %gep, align 2
  %1 = sext i16 %0 to i32
  ret i32 %1
}

; CHECK-LABEL: loadu8
; CHECK: ld.bu d2, [a4]1000
; CHECK: ret
define i32 @loadu8(ptr %bp) nounwind {
entry:
  %gep = getelementptr i8, ptr %bp, i32 1000
  %0 = load i8, ptr %gep, align 1
  %1 = zext i8 %0 to i32
  ret i32 %1
}

; CHECK-LABEL: load8
; CHECK: ld.b d2, [a4]1000
; CHECK: ret
define i32 @load8(ptr %bp) nounwind {
entry:
  %gep = getelementptr i8, ptr %bp, i32 1000
  %0 = load i8, ptr %gep, align 1
  %1 = sext i8 %0 to i32
  ret i32 %1
}
