global gdt_load

gdt_load:
    ; rdi = pointer to GDTR

    lgdt [rdi]

    ; reload data segments
    mov ax, 0x10        ; kernel data selector
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; reload CS
    push 0x08            ; kernel code selector
    lea rax, [rel .reload_cs]
    push rax
    retfq

.reload_cs:
    ret

global tss_load

tss_load:
    mov ax, 0x28
    ltr ax
    ret