#!/usr/bin/env python3
"""Repackage PSPSDK -N PBP with one 4KiB-aligned RWE PT_LOAD for PSP loader compatibility.
Does not rewrite linked code or virtual addresses. Strict validation aborts on layout changes."""
import struct
import sys
from pathlib import Path

def fix(data):
    if data[:4] != b'\x00PBP': raise ValueError("Invalid PBP")
    offsets = list(struct.unpack_from('<8I', data, 8))
    elf = bytearray(data[offsets[6]:offsets[7]])
    if elf[:6] != b'\x7fELF\x01\x01': raise ValueError("Expected MIPS ELF32 LE")
    if struct.unpack_from('<H', elf, 16)[0] != 2 or struct.unpack_from('<H', elf, 18)[0] != 8:
        raise ValueError("Expected PSP MIPS ET_EXEC")
    phoff = struct.unpack_from('<I', elf, 28)[0]
    shoff = struct.unpack_from('<I', elf, 32)[0]
    phentsize = struct.unpack_from('<H', elf, 42)[0]
    phnum = struct.unpack_from('<H', elf, 44)[0]
    shentsize = struct.unpack_from('<H', elf, 46)[0]
    shnum = struct.unpack_from('<H', elf, 48)[0]
    if phoff != 52 or phentsize != 32 or phnum != 2 or shentsize != 40:
        raise ValueError("Expected exactly two PHDRs (MIPS ABI + PT_LOAD)")
    abi = struct.unpack_from('<8I', elf, phoff)
    load = struct.unpack_from('<8I', elf, phoff + phentsize)
    if abi[0] != 0x70000003 or load[0] != 1 or load[6] != 7:
        raise ValueError("Expected ABIFLAGS and RWX PT_LOAD from linker -N")
    oldoff, vaddr = load[1], load[2]
    if oldoff < phoff + 2*phentsize or oldoff > 0x800 or (vaddr & 4095):
        raise ValueError("Unanticipated original segment placement")
    if oldoff + load[4] > len(elf):
        raise ValueError("Segment exceeds original file")
    newoff = 4096
    delta = newoff - oldoff
    output = bytearray(elf[:oldoff]) + bytearray(delta) + elf[oldoff:]
    new_shoff = shoff + delta if shoff >= oldoff else shoff
    struct.pack_into('<I', output, 32, new_shoff)
    struct.pack_into('<H', output, 44, 1)
    for i in range(shnum):
        pos = new_shoff + i*shentsize
        if pos + shentsize > len(output): raise ValueError("Invalid section table")
        sect_off = struct.unpack_from('<I', output, pos + 16)[0]
        if sect_off >= oldoff:
            struct.pack_into('<I', output, pos+16, sect_off+delta)
    struct.pack_into('<8I', output, phoff, 1, newoff, load[2], load[3],
                     load[4], load[5], load[6], 4096)
    pbp = bytearray(data[:offsets[6]]) + output + data[offsets[7]:]
    struct.pack_into('<I', pbp, 8 + 7*4, offsets[7]+delta)
    return bytes(pbp)

def check(data):
    offs=struct.unpack_from('<8I', data, 8)
    elf=data[offs[6]:offs[7]]
    phoff=struct.unpack_from('<I',elf,28)[0]
    phnum=struct.unpack_from('<H',elf,44)[0]
    p=struct.unpack_from('<8I',elf,phoff)
    assert phnum==1 and p[0]==1 and p[6]==7 and p[7]==4096
    assert (p[2]-p[1])%4096==0 and p[1]+p[4]<=len(elf)
    print('single LOAD', 'fileoff',hex(p[1]),'virtual',hex(p[2]),
          'file size',p[4],'mem size',p[5], 'entry',hex(struct.unpack_from('<I',elf,24)[0]))

if __name__=='__main__':
    src,dst=map(Path,sys.argv[1:3])
    fixed=fix(src.read_bytes());check(fixed);dst.write_bytes(fixed)
