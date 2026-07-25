SECTIONS
{
    . = 0x80000;     /* RPi 4 64-bit boot loading address */
    .text : {
        KEEP(*(.text.boot))
        *(.text)
    }
}