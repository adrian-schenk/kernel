OUTPUT_FORMAT(binary)

SECTIONS
{
    . = LOAD_ADDR;

    .text :
    {
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss (NOLOAD) :
    {
        *(.bss*)
        *(COMMON)
    }

    /DISCARD/ :
    {
        *(.note*)
        *(.comment)
        *(.debug*)
    }
}