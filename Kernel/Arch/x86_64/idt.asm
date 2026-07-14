bits 64

global idt
global idtr

section .data
align 16

idtr:
	dw idt_end - idt - 1
	dq idt

; idt_gate dpl, type
%macro idt_gate_null 2
    dw 0                                       ; offset 15:0
    dw 0x08                                    ; kernel code selector
    db 0                                       ; IST
    db (1 << 7) | ((%1 & 3) << 5) | (%2 & 0xf) ; type/attributes
    dw 0                                       ; offset 31:16
    dd 0                                       ; offset 63:32
    dd 0                                       ; reserved
%endmacro

align 16

%macro idt_gate_intr 0
    idt_gate_null 0, 0xE
%endmacro

%macro idt_gate_trap 0
    idt_gate_null 0, 0xF
%endmacro

idt:
    ; Fault - instruction execution failed, can recover
    ; Trap - an error was detected after the instruction was executed
    ; Abort - send help I am dying
    idt_gate_intr ; 0x00 00,  Fault,      Error=No,           Divide Error
    idt_gate_trap ; 0x01 01,  Trap,       Error=No,           Debug Exception
    idt_gate_intr ; 0x02 02,  Interrupt,  Error=No,           NMI Interrupt
    idt_gate_trap ; 0x03 03,  Trap,       Error=No,           Breakpoint
    idt_gate_trap ; 0x04 04,  Trap,       Error=No,           Overflow
    idt_gate_intr ; 0x05 05,  Fault,      Error=No,           BOUND Range Exceeded
    idt_gate_intr ; 0x06 06,  Fault,      Error=No,           Invalid Opcode (Undefined Opcode)
    idt_gate_intr ; 0x07 07,  Fault,      Error=No,           Device Not Available (No Math Coprocessor)
    idt_gate_intr ; 0x08 08,  Abort,      Error=Yes (zero),   Double Fault
    idt_gate_intr ; 0x09 09,  Fault,      Error=No,           Coprocessor Segment Overrun (reserved)
    idt_gate_intr ; 0x0A 10,  Fault,      Error=Yes,          Invalid TSS
    idt_gate_intr ; 0x0B 11,  Fault,      Error=Yes,          Segment Not Present
    idt_gate_intr ; 0x0C 12,  Fault,      Error=Yes,          Stack-Segment Fault
    idt_gate_intr ; 0x0D 13,  Fault,      Error=Yes,          General Protection
    idt_gate_intr ; 0x0E 14,  Fault,      Error=Yes,          Page Fault
    idt_gate_intr ; 0x0F 15,  N/A,        Error=No,           Intel reserved. Do not use.
    idt_gate_intr ; 0x10 16,  Fault,      Error=No,           x87 FPU Floating-Point Error (Math Fault)
    idt_gate_intr ; 0x11 17,  Fault,      Error=Yes (zero),   Alignment Check
    idt_gate_intr ; 0x12 18,  Abort,      Error=No,           Machine Check
    idt_gate_intr ; 0x13 19,  Fault,      Error=No,           SIMD Floating-Point Exception
    idt_gate_intr ; 0x14 20,  Fault,      Error=No,           Virtualization Exception
    idt_gate_intr ; 0x15 21,  Fault,      Error=Yes,          Control Protection Exception
    idt_gate_intr ; 0x16 22,  Reserved
    idt_gate_intr ; 0x17 23,  Reserved
    idt_gate_intr ; 0x18 24,  Reserved
    idt_gate_intr ; 0x19 25,  Reserved
    idt_gate_intr ; 0x1a 26,  Reserved
    idt_gate_intr ; 0x1b 27,  Reserved
    idt_gate_intr ; 0x1c 28,  Reserved
    idt_gate_intr ; 0x1d 29,  Reserved
    idt_gate_intr ; 0x1e 30,  Reserved
    idt_gate_intr ; 0x1f 31,  Reserved

%rep 16
    idt_gate_intr ; 0x20-0x2f, APIC
%endrep
idt_end: