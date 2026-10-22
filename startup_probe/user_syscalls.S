/* Minimal PSP 6.60 USER-mode firmware import stubs.
 * Only the four thread / load-exec syscalls used by the bare startup probe.
 * Based on publicly documented user-mode import names/NIDs.
 */
    .set noreorder
    .macro PSP_LIBRARY label, name, flags, count
    .section .rodata.stubmodulename, "a", @progbits
    .align 2
.Lname_\label:
    .asciz "\name"
    .align 2
    .section .lib.stub, "a", @progbits
    .align 2
    .word .Lname_\label
    .word \flags
    .word ((\count << 16) | 5)
    .word .Lnids_\label
    .word .Lstubs_\label
    .section .rodata.stubidtable, "a", @progbits
.Lnids_\label:
    .section .sceStub.text, "ax", @progbits
.Lstubs_\label:
    .endm

    .macro PSP_FUNCTION name, nid
    .section .rodata.stubidtable, "a", @progbits
    .word \nid
    .section .sceStub.text, "ax", @progbits
    .globl \name
\name:
    jr $ra
    nop
    .endm

    PSP_LIBRARY thread, "ThreadManForUser", 0x40090000, 3
    PSP_FUNCTION sceKernelCreateThread, 0x446D8DE6
    PSP_FUNCTION sceKernelStartThread,  0xF475845D
    PSP_FUNCTION sceKernelDelayThread,  0xCEADEB47

    PSP_LIBRARY loader, "LoadExecForUser", 0x40090000, 1
    PSP_FUNCTION sceKernelExitGame, 0x05572A5F
