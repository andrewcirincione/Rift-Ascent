"""Keep the complete published 1.8.3 media bank and append the target crest."""
from pathlib import Path
from PIL import Image
import struct,zlib,hashlib,json
r=Path(__file__).resolve().parent;repo=r.parents[1]
b=(repo/'versions/1.8.3/EBOOT.PBP').read_bytes()
assert hashlib.sha256(b).hexdigest()=='b928b825642915746dfd85826a508a26b77004efd6ecdc6ea854a99f4fffd5f9'
assert b[-136:-128]==b'RIFTAU18';music=struct.unpack_from('<I',b,len(b)-8)[0];end=len(b)-136-music
length,crc=struct.unpack_from('<II',b,end-8);bank=bytearray(b[end-16-length:end-16]);assert zlib.crc32(bank)==crc
initial=bytes(bank);(r/'soundtrack.bank').write_bytes(b[end:])
p=r/'art/target-crest.png';im=Image.open(p).convert('RGBA');im=im.crop(im.getchannel('A').getbbox());im.thumbnail((208,48),Image.Resampling.LANCZOS)
canvas=Image.new('RGBA',(208,52));canvas.alpha_composite(im,((208-im.width)//2,(52-im.height)//2));data=canvas.tobytes();offset=len(bank);bank.extend(data)
desc=b'RIFTSIMG'+struct.pack('<IIII',offset,len(data),4,zlib.crc32(data))
header=(repo/'qa/v183/source/aesthetic_art.h').read_text();endline=header.rfind('#endif');header=header[:endline]+'\nstatic const unsigned char aesthetic_target184_data[24] __attribute__((aligned(4)))={'+','.join(map(str,desc))+'};\n#define aesthetic_target184 ((const uint32_t*)aesthetic_target184_data)\n#endif\n'
(r/'source/aesthetic_art.h').write_text(header)
record=dict(name='aesthetic_target184',file=p.name,width=208,height=52,bytes=len(data),offset=offset,crc32=f'{zlib.crc32(data):08x}',sha256=hashlib.sha256(data).hexdigest(),source_sha256=hashlib.sha256(p.read_bytes()).hexdigest())
old=json.loads((repo/'qa/v183/assets.json').read_text())
(r/'assets.bin').write_bytes(bank+b'RIFTART1'+struct.pack('<II',len(bank),zlib.crc32(bank)))
(r/'assets.json').write_text(json.dumps(dict(bytes=len(bank),sha256=hashlib.sha256(bank).hexdigest(),assets=old['assets']+[record],base='v1.8.3',base_bank_bytes=length,base_bank_sha256=hashlib.sha256(initial).hexdigest(),new_assets=[record],draft_missing=[]),indent=2)+'\n')
print(len(bank),'bank bytes; original art and all ten music streams preserved')
