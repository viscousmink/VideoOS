.section ".text.boot"

.global _start

_start:
    // 1. Core 0 check: read MPIDR_EL1
    mrs     x0, mpidr_el1
    and     x0, x0, #3          // Mask core ID bits (0-3)
    cbz     x0, master_core     // If Core 0, continue

park_core:
    // Put Cores 1, 2, and 3 to sleep
    wfe
    b       park_core

master_core:
    // 2. Load PL011 UART0 Data Register base address (0xFE201000) into x1
    movz    x1, #0x1000
    movk    x1, #0xfe20, lsl #16

    // Print 'H'
    mov     w2, #'H'
    str     w2, [x1]

    // Print 'i'
    mov     w2, #'i'
    str     w2, [x1]

    // Print '\n'
    mov     w2, #'\n'
    str     w2, [x1]

halt:
    // Infinite loop
    wfe
    b       halt