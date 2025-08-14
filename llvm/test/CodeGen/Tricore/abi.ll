; RUN: llc -mtriple=tricore < %s | FileCheck %s

; CHECK-LABEL: test_abi_i32
; CHECK: mov d4, 1
; CHECK: call abi_i32
; CHECK: ret
declare void @abi_i32(i32 %a)
define void @test_abi_i32() nounwind {
  call void @abi_i32(i32 noundef 1)
  ret void
}

; CHECK-LABEL: test_abi_ret_i32
; CHECK: mov d2, d4
; CHECK: ret
define i32 @test_abi_ret_i32(i32 %a) nounwind {
  ret i32 %a
}

; CHECK-LABEL: test_abi_i64
; CHECK: sub.a a10, 0
; CHECK: ld.d e4, [a15]
; CHECK: call abi_i64
; CHECK: ret
declare void @abi_i64(i64 %a)
define void @test_abi_i64() nounwind {
  call void @abi_i64(i64 noundef 1)
  ret void
}

; CHECK-LABEL: test_abi_ret_i64
; CHECK: mov e2, d5, d4
; CHECK: ret
define i64 @test_abi_ret_i64(i64 %a) nounwind {
  ret i64 %a
}

; CHECK-LABEL: test_abi_i32i64
; CHECK: sub.a a10, 0
; CHECK: ld.d e6, [a15]
; CHECK: mov d4, 2
; CHECK: call abi_i32i64
; CHECK: ret
declare void @abi_i32i64(i32 %b, i64 %a)
define void @test_abi_i32i64() nounwind {
  call void @abi_i32i64(i32 noundef 2, i64 noundef 1)
  ret void
}

; CHECK-LABEL: test_abi_i32i64
; CHECK: sub.a a10, 0
; CHECK: ld.d e6, [a15]
define i64 @test_abi_arg_i32i64(i32 %a, i64 %b) nounwind {
  %y = sext i32 %a to i64
  %x = add i64 %b, %y
  ret i64 %x
}


; CHECK-LABEL: test_abi_i32i64i32
; CHECK: sub.a a10, 0
; CHECK: ld.d e6, [a15]
; CHECK: mov d4, 2
; CHECK: mov d5, 3
; CHECK: call abi_i32i64i32
; CHECK: ret
declare void @abi_i32i64i32(i32 %b, i64 %a, i32 %c)
define void @test_abi_i32i64i32() nounwind {
  call void @abi_i32i64i32(i32 noundef 2, i64 noundef 1, i32 noundef 3)
  ret void
}

; CHECK-LABEL: test_abi_i32i64i64
; CHECK: sub.a a10, 8
; CHECK: ld.d e6, [a15]
; CHECK: mov d4, 2
; CHECK: st.d [a10], e0
; CHECK: call abi_i32i64i64
; CHECK: ret
declare void @abi_i32i64i64(i32 %b, i64 %a, i64 %c)
define void @test_abi_i32i64i64() nounwind {
  call void @abi_i32i64i64(i32 noundef 2, i64 noundef 1, i64 noundef 3)
  ret void
}

; CHECK-LABEL: test_abi_4i32
; CHECK: sub.a a10, 0
; CHECK: mov d4, 0
; CHECK: mov d5, 1
; CHECK: mov d6, 2
; CHECK: mov d7, 3
; CHECK: call abi_4i32
; CHECK: ret
declare void @abi_4i32(i32 %a, i32 %b, i32 %c, i32 %d)
define void @test_abi_4i32() nounwind {
  call void @abi_4i32(i32 noundef 0, i32 noundef 1, i32 noundef 2, i32 noundef 3)
  ret void
}

; CHECK-LABEL: test_abi_5i32
; CHECK: sub.a a10, 8
; CHECK: mov d15, 4
; CHECK: mov d4, 0
; CHECK: mov d5, 1
; CHECK: mov d6, 2
; CHECK: mov d7, 3
; CHECK: st.w [a10], d15
; CHECK: call abi_5i32
; CHECK: ret
declare void @abi_5i32(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f)
define void @test_abi_5i32() nounwind {
  call void @abi_5i32(i32 noundef 0, i32 noundef 1, i32 noundef 2, i32 noundef 3, i32 noundef 4)
  ret void
}

; CHECK-LABEL: abi2
; CHECK: sub.a a10, 4
; CHECK: ld.w d4, [a10]
; CHECK: call test_struct_i32
; CHECK: ret
%Foo = type {
    i32
}
declare void @test_struct_i32(%Foo %a)

define void @abi2(i32 %a, i32 %b) nounwind {
  %x = alloca %Foo
  %y = load %Foo, %Foo* %x
  call void @test_struct_i32(%Foo %y)
  ret void
}

; CHECK-LABEL: struct_2i32
; CHECK: sub.a a10, 8
; CHECK: ld.w d5, [a10]4
; CHECK: ld.w d4, [a10]
; CHECK: call test_struct_2i32
; CHECK: ret
%S2I32 = type {
    i32,
    i32
}
declare void @test_struct_2i32(%S2I32 %a)

define void @struct_2i32(i32 %a, i32 %b) nounwind {
  %x = alloca %S2I32
  %y = load %S2I32, %S2I32* %x
  call void @test_struct_2i32(%S2I32 %y)
  ret void
}

; CHECK-LABEL: abi3
; CHECK: sub.a a10, 4
; CHECK: ld.w d4, [a10]
; CHECK: call test_struct_nested_i32
; CHECK: ret
%Foo1 = type {
    %Foo
}
declare void @test_struct_nested_i32(%Foo1 %a)

define void @abi3(i32 %a, i32 %b) nounwind {
  %x = alloca %Foo1
  %y = load %Foo1, %Foo1* %x
  call void @test_struct_nested_i32(%Foo1 %y)
  ret void
}
