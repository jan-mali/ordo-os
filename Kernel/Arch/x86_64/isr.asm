bits 64

extern idt, idtr, isr_handler
global init_isr_and_lidt

section .text

; load_isr index, handler
%macro load_isr 2
    lea rbx, [rel idt]

    mov eax, %1
    shl rax, 4
    add rbx, rax

    lea rax, [rel %2]

    mov word [rbx], ax

    shr rax, 16
    mov word [rbx+6], ax

    shr rax, 16
    mov dword [rbx+8], eax
%endmacro

init_isr_and_lidt:
    push rbx
%assign i 0
%rep 48
    ; 32 Exceptions + 16 PIC IRQs
    load_isr i, isr_handler_%+i
%assign i i+1
%endrep

    load_isr 127, isr_handler_127 ; Scheduler yield

    lidt [rel idtr]
    pop rbx
    ret

isr_handler_common:
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8

    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov rdi, rsp ; Pointer to the Registers struct
    call isr_handler

    test rax, rax
    jz .no_switch

    mov rsp, rax

.no_switch:
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp

    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    add rsp, 16
    iretq


%macro isr_handler_error 1
isr_handler_%1:
    push %1 ; IRQ Number
    jmp isr_handler_common
%endmacro

%macro isr_handler_noerr 1
isr_handler_%1:
    push 0 ; Fake error code
    push %1 ; IRQ Number
    jmp isr_handler_common
%endmacro

; Exceptions
isr_handler_noerr 0
isr_handler_noerr 1
isr_handler_noerr 2
isr_handler_noerr 3
isr_handler_noerr 4
isr_handler_noerr 5
isr_handler_noerr 6
isr_handler_noerr 7
isr_handler_error 8
isr_handler_noerr 9
isr_handler_error 10
isr_handler_error 11
isr_handler_error 12
isr_handler_error 13
isr_handler_error 14
isr_handler_noerr 15
isr_handler_noerr 16
isr_handler_error 17
isr_handler_noerr 18
isr_handler_noerr 19
isr_handler_noerr 20
isr_handler_error 21

; Reserved
isr_handler_noerr 22
isr_handler_noerr 23
isr_handler_noerr 24
isr_handler_noerr 25
isr_handler_noerr 26
isr_handler_noerr 27
isr_handler_noerr 28
isr_handler_noerr 29
isr_handler_noerr 30
isr_handler_noerr 31

; APIC Interrupts
%assign i 32
%rep 16
isr_handler_noerr i
%assign i i+1
%endrep

isr_handler_noerr 127 ; Scheduler yield
