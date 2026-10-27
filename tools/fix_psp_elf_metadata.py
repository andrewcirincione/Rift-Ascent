"""Set the PSP module-info locator in an ET_EXEC's first PT_LOAD header.

PSP uses p_paddr as a file offset to .rodata.sceModuleInfo, not a physical
load address. Leaving it equal to p_vaddr invokes CFW's static-ELF fallback,
which can leave execSize unset and make ModuleMgr size its buffer to EOF,
including music/art in DATA.PSAR. The locator prevents that fallback.
"""
from pathlib import Path
import struct,sys
def fix(path):
 p=Path(path);data=bytearray(p.read_bytes());assert data[:4]==b'\0PBP'
 off=struct.unpack_from('<8I',data,8);base,end=off[6:8];elf=memoryview(data)[base:end]
 assert elf[:4]==b'\x7fELF' and struct.unpack_from('<H',elf,16)[0]==2
 shoff=struct.unpack_from('<I',elf,32)[0];shsize,n,strings=struct.unpack_from('<HHH',elf,46)
 sections=[struct.unpack_from('<10I',elf,shoff+i*shsize) for i in range(n)]
 names=bytes(elf[sections[strings][4]:sections[strings][4]+sections[strings][5]])
 module=next(x for x in sections if names[x[0]:].split(b'\0',1)[0]==b'.rodata.sceModuleInfo')
 assert module[5]==52 and module[4]+module[5]<=len(elf)
 assert struct.unpack_from('<H',elf,module[4])[0]&0x1000==0,'User-mode module required'
 phoff=struct.unpack_from('<I',elf,28)[0];phsize,nph=struct.unpack_from('<HH',elf,42)
 first=next(phoff+i*phsize for i in range(nph) if struct.unpack_from('<I',elf,phoff+i*phsize)[0]==1)
 old=struct.unpack_from('<I',elf,first+12)[0]
 struct.pack_into('<I',data,base+first+12,module[4]);p.write_bytes(data)
 print(f'PASS PSP first-load module-info locator: {old:#x} -> ELF file offset {module[4]:#x}')
if __name__=='__main__':fix(sys.argv[1])
