/* User-mode HOME exit callback and time seed for firmware 6.60.
 * User-mode imports. Kept separate from startup syscalls for audit clarity. */
.set noreorder
.section .rodata.stubmodulename, "a", @progbits
.balign 4
.Lthreadname:
.asciz "ThreadManForUser"
.balign 4
.Lexitname:
.asciz "LoadExecForUser"
.balign 4

.section .lib.stub, "a", @progbits
.balign 4
.word .Lthreadname
.word 0x40090000
.word (3 << 16) | 5
.word .Lthreadnids
.word .Lthreadstubs
.word .Lexitname
.word 0x40090000
.word (1 << 16) | 5
.word .Lexitnids
.word .Lexitstubs

.section .rodata.sceNid, "a", @progbits
.Lthreadnids:
.word 0xE81CAF8F  /* sceKernelCreateCallback */
.word 0x82826F70  /* sceKernelSleepThreadCB */
.word 0x369ED59D  /* sceKernelGetSystemTimeLow */
.Lexitnids:
.word 0x4AC57943  /* sceKernelRegisterExitCallback */

.section .sceStub.text, "ax", @progbits
.Lthreadstubs:
.globl sceKernelCreateCallback
sceKernelCreateCallback:
    jr $ra
    nop
.globl sceKernelSleepThreadCB
sceKernelSleepThreadCB:
    jr $ra
    nop
.globl sceKernelGetSystemTimeLow
sceKernelGetSystemTimeLow:
    jr $ra
    nop
.Lexitstubs:
.globl sceKernelRegisterExitCallback
sceKernelRegisterExitCallback:
    jr $ra
    nop
