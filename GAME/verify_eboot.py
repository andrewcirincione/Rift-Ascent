#!/usr/bin/env python3
"""Check a PSPSDK-packaged EBOOT.PBP, not execution or hardware compatibility.

The PSPSDK Makefile produces an ordinary ELF as DATA.PSP when BUILD_PRX is unset.
This script intentionally rejects PRX packages for this specific build configuration.
"""
from pathlib import Path
import hashlib
import struct
import sys


class InvalidEboot(ValueError):
    pass


def require(ok, message):
    if not ok:
        raise InvalidEboot(message)


def u16(data, offset):
    require(offset >= 0 and offset + 2 <= len(data), 'Truncated u16')
    return struct.unpack_from('<H', data, offset)[0]


def u32(data, offset):
    require(offset >= 0 and offset + 4 <= len(data), 'Truncated u32')
    return struct.unpack_from('<I', data, offset)[0]


def check(path):
    path = Path(path)
    data = path.read_bytes()
    require(len(data) >= 256, 'PBP is too short')
    require(data[:4] == b'\0PBP', 'Not a PBP')
    require(u32(data, 4) == 0x10000, 'Unexpected PBP version')
    offsets = list(struct.unpack_from('<8I', data, 8)) + [len(data)]
    require(offsets[0] == 40, 'Invalid SFO start')
    require(all(40 <= a <= b <= len(data) for a, b in zip(offsets, offsets[1:])), 'Invalid PBP offsets')
    require(offsets[1] - offsets[0] >= 20, 'Missing PARAM.SFO')
    require(data[offsets[0]:offsets[0]+4] == b'\0PSF', 'Invalid SFO signature')
    require(data[offsets[1]:offsets[1]+8] == b'\x89PNG\r\n\x1a\n', 'Missing ICON0.PNG')
    exe = data[offsets[6]:offsets[7]]
    require(len(exe) >= 256 and exe[:4] == b'\x7fELF', 'Missing ELF in DATA.PSP')
    require(exe[4:7] == b'\x01\x01\x01', 'ELF must be 32-bit little endian')
    require(u16(exe, 16) in (2, 3), 'ELF type is not executable/shared')
    require(u16(exe, 18) == 8, 'ELF architecture is not MIPS')
    require(u32(exe, 24) != 0, 'ELF entry point is zero')
    phoff = u32(exe, 28)
    ehsize, phentsize, phnum = struct.unpack_from('<HHH', exe, 40)
    require(ehsize >= 52 and phentsize >= 32 and 1 <= phnum <= 32, 'Invalid ELF headers')
    require(phoff + phentsize * phnum <= len(exe), 'ELF program headers truncated')
    segments = 0
    static_bytes = 0
    low_address = 0xffffffff
    high_address = 0
    entry = u32(exe, 24)
    entry_is_mapped_executable = False
    first_load_physical = None
    for i in range(phnum):
        off = phoff + phentsize * i
        kind, seg_off, vaddr, physical, fsize, msize, flags, align = struct.unpack_from('<8I', exe, off)
        if kind == 1:
            if first_load_physical is None:
                first_load_physical = physical
            segments += 1
            require(seg_off + fsize <= len(exe) and msize >= fsize, 'Bad load segment')
            require(align == 0 or (align & (align - 1)) == 0, 'Invalid PT_LOAD alignment')
            require(align <= 1 or (vaddr % align) == (seg_off % align), 'Misaligned PT_LOAD')
            if (flags & 1) and vaddr <= entry < vaddr + msize:
                entry_is_mapped_executable = True
            static_bytes += msize
            low_address = min(low_address, vaddr)
            high_address = max(high_address, vaddr + msize)
    require(segments > 0, 'No loadable ELF segments')
    require(entry_is_mapped_executable, 'Entry point is not inside an executable PT_LOAD segment')
    shoff = u32(exe, 32)
    shentsize, shnum, shstr = struct.unpack_from('<HHH', exe, 46)
    require(shentsize == 40 and shstr < shnum and shoff + shnum * shentsize <= len(exe), 'Invalid section table')
    sections = [struct.unpack_from('<10I', exe, shoff + i * shentsize) for i in range(shnum)]
    strings = sections[shstr]
    names = exe[strings[4]:strings[4] + strings[5]]
    module = next((s for s in sections if names[s[0]:].split(b'\0', 1)[0] == b'.rodata.sceModuleInfo'), None)
    require(module is not None and module[5] == 52, 'Missing PSP module info')
    require(first_load_physical == module[4], 'PSP first PT_LOAD must locate module info by ELF file offset; avoids CFW whole-file allocation')
    span = high_address - low_address
    require(0x08800000 <= low_address < high_address <= 0x0a000000, 'Load segments exceed the 32 MB PSP user partition')
    # 1.6 originally passed a 24 MiB SUM check with almost no loader/thread
    # headroom. Audit address SPAN too and keep a conservative startup budget.
    require(span <= 14 * 1024 * 1024, 'PSP resident span exceeds the 14 MiB hardware headroom budget')
    require(span + len(exe) <= 20 * 1024 * 1024, 'PSP executable plus resident span exceeds the 20 MiB startup budget')
    return {'size': len(data), 'elf_size': len(exe), 'segments': segments,
            'static_memory': static_bytes, 'resident_span': span,
            'conservative_startup_bytes': span + len(exe),
            'sha256': hashlib.sha256(data).hexdigest()}


def main(argv):
    if len(argv) != 2:
        print('Usage: verify_eboot.py PATH_TO_EBOOT.PBP', file=sys.stderr)
        return 2
    try:
        info = check(argv[1])
    except (InvalidEboot, OSError, struct.error) as exc:
        print('FAIL:', exc, file=sys.stderr)
        return 1
    print('PBP/ELF structural checks PASS (not a PSP runtime test)')
    for key, value in info.items():
        print(f'{key}: {value}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv))
