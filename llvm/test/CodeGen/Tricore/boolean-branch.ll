; RUN: llc -mtriple=tricore -mcpu=tc162 -verify-machineinstrs < %s | FileCheck %s --check-prefixes=CHECK,OPT
; RUN: llc -mtriple=tricore -mcpu=tc162 -O0 -verify-machineinstrs < %s | FileCheck %s
; RUN: llc -mtriple=tricore -mcpu=tc162 -filetype=obj < %s -o %t
; RUN: llvm-objdump -d %t | FileCheck %s --check-prefix=BYTES

; A zero-extended Boolean return value must branch on a set bit. In an
; async poll loop, branching on a clear bit treats Pending as Ready.
; CHECK-LABEL: poll_until_pending:
; CHECK: call poll_pending
; CHECK: jnz.t d2, 0,
; CHECK: call handle_ready
; BYTES-LABEL: <poll_until_pending>:
; BYTES: 6f 02 {{[0-9a-f][0-9a-f]}} 80
define void @poll_until_pending() {
entry:
  br label %poll
poll:
  %pending = call zeroext i1 @poll_pending()
  br i1 %pending, label %exit, label %ready
ready:
  call void @handle_ready()
  br label %poll
exit:
  ret void
}

declare zeroext i1 @poll_pending()
declare void @handle_ready()

@marker = global i32 0

; Explicit comparisons against a masked bit retain their own polarity.
; CHECK-LABEL: branch_when_clear:
; CHECK: jz.t d4, 3,
define void @branch_when_clear(i32 %value) {
  %bit = and i32 %value, 8
  %clear = icmp eq i32 %bit, 0
  br i1 %clear, label %exit, label %ready
ready:
  call void @handle_ready()
  store volatile i32 1, ptr @marker
  br label %exit
exit:
  ret void
}

; CHECK-LABEL: branch_when_set:
; CHECK: jnz.t d4, 3,
define void @branch_when_set(i32 %value) {
  %bit = and i32 %value, 8
  %set = icmp ne i32 %bit, 0
  br i1 %set, label %exit, label %ready
ready:
  call void @handle_ready()
  store volatile i32 1, ptr @marker
  br label %exit
exit:
  ret void
}

; Rust's C Boolean ABI returns i32 and truncates it to i1. The optimized
; backward JZ.T must retain a clear opcode bit next to its signed disp15.
; OPT-LABEL: poll_integer_until_pending:
; OPT: call poll_integer
; OPT: call handle_ready
; OPT: call poll_integer
; OPT: jz.t d2, 0,
; BYTES-LABEL: <poll_integer_until_pending>:
; BYTES: 6f 02 {{[0-9a-f][0-9a-f]}} 7f
define void @poll_integer_until_pending() {
entry:
  %first = call i32 @poll_integer()
  %first_pending = trunc i32 %first to i1
  br i1 %first_pending, label %exit, label %ready
ready:
  call void @handle_ready()
  %next = call i32 @poll_integer()
  %pending = trunc i32 %next to i1
  br i1 %pending, label %exit, label %ready
exit:
  ret void
}

declare i32 @poll_integer()

; The same disp15 fixup must preserve the comparison opcode on a
; backward integer branch, not change JEQ into JNE.
; OPT-LABEL: wait_while_equal:
; OPT: call poll_integer
; OPT: jeq d2, d15,
; BYTES-LABEL: <wait_while_equal>:
; BYTES: 5f f2 {{[0-9a-f][0-9a-f]}} 7f
define void @wait_while_equal() {
entry:
  br label %poll
poll:
  %value = call i32 @poll_integer()
  %zero = icmp eq i32 %value, 37
  br i1 %zero, label %poll, label %exit
exit:
  ret void
}
