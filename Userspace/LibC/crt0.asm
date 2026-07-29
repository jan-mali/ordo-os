global _start

extern main
extern exit

section .text.entry

_start:
    mov rax, 0xCAFEBABE

    call main

    mov rdi, rax
    call exit

.hang:
    jmp .hang