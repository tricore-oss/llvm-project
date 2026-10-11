; RUN: llc -mtriple=tricore -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -O0 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -verify-machineinstrs -stop-after=finalize-isel < %s | FileCheck %s --check-prefix=MIR

; A GCC caller supplies strlen's string in A4, and expects its length in D2.
; CHECK-LABEL: strlen:
; CHECK: mov.d {{d[0-9]+}}, a4
; CHECK: ret
; MIR-LABEL: name: strlen
; MIR: liveins: $a4
; MIR: RET implicit $d2
define i32 @strlen(ptr %s) nounwind {
entry:
  br label %loop
loop:
  %p = phi ptr [ %s, %entry ], [ %next, %loop ]
  %n = phi i32 [ 0, %entry ], [ %inc, %loop ]
  %c = load i8, ptr %p
  %next = getelementptr i8, ptr %p, i32 1
  %inc = add i32 %n, 1
  %done = icmp eq i8 %c, 0
  br i1 %done, label %exit, label %loop
exit:
  ret i32 %n
}

; CHECK-LABEL: call_strlen:
; CHECK: mov.a a4, {{d[0-9]+}}
; CHECK: call strlen
; CHECK: ret
define i32 @call_strlen(ptr %s) nounwind {
  %n = call i32 @strlen(ptr %s)
  ret i32 %n
}

; Pointer arguments do not consume integer argument registers.
; CHECK-LABEL: mixed_arguments:
; CHECK: mov.d {{d[0-9]+}}, a4
; CHECK: mov.d {{d[0-9]+}}, a5
; CHECK: ret
; MIR-LABEL: name: mixed_arguments
; MIR: liveins: $a4, $d4, $a5, $d5
define i32 @mixed_arguments(ptr %p, i32 %x, ptr %q, i32 %y) nounwind {
  %a = ptrtoint ptr %p to i32
  %b = ptrtoint ptr %q to i32
  %c = add i32 %a, %b
  %d = add i32 %c, %x
  %e = add i32 %d, %y
  ret i32 %e
}

declare void @consume_mixed(ptr, i32, ptr, i32)
; CHECK-LABEL: call_mixed:
; CHECK-DAG: mov.a a4, {{d[0-9]+}}
; CHECK-DAG: mov.a a5, {{d[0-9]+}}
; CHECK-DAG: mov d4, 11
; CHECK-DAG: mov d5, 22
; CHECK: call consume_mixed
; CHECK: ret
define void @call_mixed() nounwind {
  call void @consume_mixed(ptr inttoptr (i32 100 to ptr), i32 11,
                          ptr inttoptr (i32 200 to ptr), i32 22)
  ret void
}

; After A4-A7 are exhausted, pointers go on the stack, even if D4 is free.
; CHECK-LABEL: fifth_pointer:
; CHECK: ld.w {{d[0-9]+}}, [a10]
; CHECK: ret
define i32 @fifth_pointer(ptr %p, ptr %q, ptr %r, ptr %s, ptr %t) nounwind {
  %n = ptrtoint ptr %t to i32
  ret i32 %n
}

declare void @consume_five(ptr, ptr, ptr, ptr, ptr, i32)
; CHECK-LABEL: call_five:
; CHECK: lea a10, a10, -8
; CHECK: mov.d [[SP:d[0-9]+]], a10
; CHECK-DAG: mov.a [[STACK:a[0-9]+]], [[SP]]
; CHECK-DAG: mov.a a4, {{d[0-9]+}}
; CHECK-DAG: mov.a a5, {{d[0-9]+}}
; CHECK-DAG: mov.a a6, {{d[0-9]+}}
; CHECK-DAG: mov.a a7, {{d[0-9]+}}
; CHECK-DAG: mov d4, 66
; CHECK-DAG: st.w [[[STACK]]], {{d[0-9]+}}
; CHECK: call consume_five
; CHECK: ret
define void @call_five() nounwind {
  call void @consume_five(ptr inttoptr (i32 100 to ptr),
                         ptr inttoptr (i32 200 to ptr),
                         ptr inttoptr (i32 300 to ptr),
                         ptr inttoptr (i32 400 to ptr),
                         ptr inttoptr (i32 500 to ptr), i32 66)
  ret void
}

; CHECK-LABEL: return_pointer:
; CHECK: mov.a a2, {{d[0-9]+}}
; CHECK: ret
define ptr @return_pointer(i32 %address) nounwind {
  %p = inttoptr i32 %address to ptr
  ret ptr %p
}

declare ptr @get_pointer()
; CHECK-LABEL: call_pointer_return:
; CHECK: call get_pointer
; CHECK: mov.d {{d[0-9]+}}, a2
; CHECK: ret
define i32 @call_pointer_return() nounwind {
  %p = call ptr @get_pointer()
  %n = ptrtoint ptr %p to i32
  ret i32 %n
}

; Also verify the address-register live-in when a pointer is returned directly.
; CHECK-LABEL: pointer_identity:
; CHECK: mov.d {{d[0-9]+}}, a4
; CHECK: mov.a a2, {{d[0-9]+}}
; CHECK: ret
define ptr @pointer_identity(ptr %p) nounwind {
  ret ptr %p
}

; A pointer field in a small aggregate still uses a data register.
; CHECK-LABEL: aggregate_pointer:
; CHECK: mov d2, d4
; CHECK: ret
define i32 @aggregate_pointer({ ptr } %s) nounwind {
  %p = extractvalue { ptr } %s, 0
  %n = ptrtoint ptr %p to i32
  ret i32 %n
}

declare void @llvm.memcpy.p0.p0.i32(ptr, ptr, i32, i1 immarg)
; Compiler-generated libcalls must also pass pointers in address registers.
; CHECK-LABEL: copy_bytes:
; CHECK-DAG: mov.a a4, {{d[0-9]+}}
; CHECK-DAG: mov.a a5, {{d[0-9]+}}
; CHECK: call memcpy
; CHECK: ret
define void @copy_bytes(ptr %dst, ptr %src, i32 %n) nounwind {
  call void @llvm.memcpy.p0.p0.i32(ptr %dst, ptr %src, i32 %n, i1 false)
  ret void
}
