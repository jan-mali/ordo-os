global _start

extern main
extern exit

section .text.entry

_start:
    call main

    mov rdi, rax
    call exit

.hang:
    jmp .hang