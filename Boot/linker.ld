ENTRY(kmain)

SECTIONS
{
    . = 0xffffffff80000000;

    .text : ALIGN(0x1000)
    {
        *(.text .text.*)
    }

    .rodata : ALIGN(0x1000)
    {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(0x1000)
    {
        *(.data .data.*)
    }

    .bss : ALIGN(0x1000)
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    .init_array : ALIGN(0x1000)
    {
        KEEP(*(.init_array))
    }

    .fini_array : ALIGN(0x1000)
    {
        KEEP(*(.fini_array))
    }

    _kernel_end = .;
}
