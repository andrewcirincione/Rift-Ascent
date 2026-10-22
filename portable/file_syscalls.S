/* User-mode memory stick I/O (IoFileMgrForUser) — no newlib. */
.set noreorder
.section .rodata.stubmodulename, "a", @progbits
.balign 4
.Lfile_name:
.asciz "IoFileMgrForUser"
.balign 4
.section .lib.stub, "a", @progbits
.balign 4
.word .Lfile_name
.word 0x40090000
.word (4 << 16) | 5
.word .Lfile_nids
.word .Lfile_stubs
.section .rodata.sceNid, "a", @progbits
.Lfile_nids:
.word 0x109F50BC  /* sceIoOpen */
.word 0x6A638D83  /* sceIoRead */
.word 0x42EC03AC  /* sceIoWrite */
.word 0x810C4BC3  /* sceIoClose */
.section .sceStub.text, "ax", @progbits
.Lfile_stubs:
.globl sceIoOpen
sceIoOpen:
    jr $ra
    nop
.globl sceIoRead
sceIoRead:
    jr $ra
    nop
.globl sceIoWrite
sceIoWrite:
    jr $ra
    nop
.globl sceIoClose
sceIoClose:
    jr $ra
    nop
