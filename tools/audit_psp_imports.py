#!/usr/bin/env python3
"""Audit PSP PBP/ELF import library table before real hardware tests."""
import sys
import struct
from pathlib import Path

BAD = ("ForKernel", "Kernel_Library", "sceKernelLibrary")
# Some kernel-designated export groups are *not* necessarily fatal; this
# rejects direct kernel-only imports from an app declared to run usermode.
def parse(path):
    b = Path(path).read_bytes()
    if b[:4] != b"\0PBP": raise ValueError("Not a PBP")
    o = struct.unpack_from("<8I", b, 8)
    e = b[o[6]:o[7]]
    if e[:4] != b"\x7fELF": raise ValueError("No ELF")
    shoff = struct.unpack_from("<I",e,32)[0]
    shnum=struct.unpack_from("<H",e,48)[0]
    shstrndx=struct.unpack_from("<H",e,50)[0]
    s=[struct.unpack_from("<10I",e,shoff+i*40) for i in range(shnum)]
    shstrings=e[s[shstrndx][4]:s[shstrndx][4]+s[shstrndx][5]]
    def name(off):return shstrings[off:].split(b"\0",1)[0].decode("ascii","replace")
    byname={name(x[0]):x for x in s}
    if ".lib.stub" not in byname:raise ValueError("Missing PSP import section")
    stub=byname[".lib.stub"]
    phoff=struct.unpack_from("<I",e,28)[0]
    phnum=struct.unpack_from("<H",e,44)[0]
    loads=[struct.unpack_from("<8I",e,phoff+32*i) for i in range(phnum)]
    loads=[x for x in loads if x[0]==1]
    def read_va(addr,length):
        for x in loads:
            off,v,size=x[1],x[2],x[4]
            if v<=addr and addr+length<=v+size:
                return e[off+addr-v:off+addr-v+length]
        return b""
    values=e[stub[4]:stub[4]+stub[5]]
    if len(values)%20:raise ValueError("Stub length is not multiple of 20")
    imports=[]
    for i in range(0,len(values),20):
        addr,flags,meta,nids,table=struct.unpack_from("<IIIII",values,i)
        lib=read_va(addr,128).split(b"\0",1)[0].decode("ascii","replace")
        if not lib:raise ValueError("Unresolvable import stub name")
        imports.append((lib,flags))
    return imports
if __name__=="__main__":
    for path in sys.argv[1:]:
        imports=parse(path)
        print(path)
        for name,flags in imports:
            print("  %s 0x%08x"%(name,flags))
        unsafe=[n for n,f in imports if any(tag in n for tag in BAD)]
        if unsafe:
            print("FAIL: Kernel-only import libraries found: "+", ".join(unsafe),file=sys.stderr)
            sys.exit(2)
        print("PASS: no kernel-only import libraries")
