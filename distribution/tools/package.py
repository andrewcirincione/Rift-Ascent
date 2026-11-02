from pathlib import Path
import struct,hashlib,subprocess,os,json
from resources import mapped
R=Path(__file__).resolve().parents[2];S=R/'qa/v186/source';D=R/'distribution/resources';E=R/'distribution/evidence';E.mkdir(exist_ok=True)
b=bytearray((S/'EBOOT.PBP').read_bytes());offsets=struct.unpack_from('<8I',b,8);elf=bytearray(b[offsets[6]:offsets[7]])
nm=subprocess.check_output([str(Path(os.environ['PSPDEV'])/'bin/psp-nm'),'-S',str(S/'rift_ascent_crt_free.elf')],text=True)
syms={p[-1]:(int(p[0],16),int(p[1],16)) for l in nm.splitlines() if len(p:=l.split())==4}
for name,file in [('ra_title_bg','title565.bin'),('rift_hero','hero565.bin')]:
 addr,size=syms[name];pixels=(D/file).read_bytes();assert len(pixels)==size;at=mapped(elf,addr,size);elf[at:at+size]=pixels
b[offsets[6]:offsets[7]]=elf;(S/'EBOOT.PBP').write_bytes(b)
subprocess.run(['python3',str(R/'tools/fix_psp_elf_metadata.py'),str(S/'EBOOT.PBP')],check=True)
subprocess.run(['python3',str(R/'tools/audit_psp_imports.py'),str(S/'EBOOT.PBP')],check=True)
b=(S/'EBOOT.PBP').read_bytes()+(D/'art.bank').read_bytes()+(D/'soundtrack.bank').read_bytes()
tmp=E/'EBOOT.PBP.tmp';tmp.write_bytes(b);tmp.replace(E/'EBOOT.PBP')
sha=hashlib.sha256(b).hexdigest();(E/'EBOOT.sha256').write_text(sha+'  EBOOT.PBP\n')
print('PACKAGED',len(b),sha)
