.intel_syntax noprefix
.global foo
foo:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov [rbp-4], edi
  mov [rbp-8], esi
  mov eax, 10
  lea rdi, [rbp-12]
  mov DWORD PTR [rdi], eax
  lea rdi, [rbp-4]
  mov eax, DWORD PTR [rdi]
  push rax
  lea rdi, [rbp-8]
  mov eax, DWORD PTR [rdi]
  push rax
  pop rdi
  pop rax
  add eax, edi
  push rax
  lea rdi, [rbp-12]
  mov eax, DWORD PTR [rdi]
  push rax
  pop rdi
  pop rax
  add eax, edi
  jmp .L_foo_epilogue
.L_foo_epilogue:
  leave
  ret
.global main
main:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov eax, 1
  lea rdi, [rbp-4]
  mov DWORD PTR [rdi], eax
  mov eax, 2
  lea rdi, [rbp-8]
  mov DWORD PTR [rdi], eax
  lea rdi, [rbp-4]
  mov eax, DWORD PTR [rdi]
  push rax
  lea rdi, [rbp-8]
  mov eax, DWORD PTR [rdi]
  push rax
  pop rsi
  pop rdi
  call foo
  jmp .L_main_epilogue
.L_main_epilogue:
  leave
  ret
