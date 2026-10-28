"""Exercise the actual cache through every original texture and invalid input."""
from pathlib import Path
import ctypes as C,struct,zlib,json,hashlib,random,subprocess
q=Path(__file__).parent;root=q.parents[1];lib=root.parent/'runtime/asset-test.so'
subprocess.run(['cc','-O2','-shared','-fPIC','-DRIFT_STREAMED_ART','-DRIFT_ASSET_TEST',str(root/'qa/v171r2/source/asset_stream.c'),'-o',str(lib)],check=True)
dll=C.CDLL(str(lib));dll.rift_asset_resolve.argtypes=[C.c_void_p,C.c_uint];dll.rift_asset_resolve.restype=C.c_void_p;dll.rift_assets_set_file.argtypes=[C.c_char_p]
pbp=root/'qa/v171r2/evidence/EBOOT.PBP';path=str(pbp).encode();dll.rift_assets_set_file(path)
raw=pbp.read_bytes();music=struct.unpack_from('<I',raw,len(raw)-8)[0];end=len(raw)-56-music
assert raw[end-16:end-8]==b'RIFTART1';size,crc=struct.unpack_from('<II',raw,end-8);bank=raw[end-16-size:end-16]
meta=json.loads((q/'assets.json').read_text());assert hashlib.sha256(bank).hexdigest()==meta['sha256'] and zlib.crc32(bank)==crc
# Force a stale file handle after one cached texture. A miss must reopen it.
dll.rift_assets_open()
a=meta['assets'][0];desc=C.create_string_buffer(b'RIFTSIMG'+struct.pack('<4I',a['offset'],a['bytes'],a['pixel_bytes'],a['crc32']));assert dll.rift_asset_resolve(desc,a['bytes'])
dll.rift_assets_test_break_io()
a=meta['assets'][1];desc=C.create_string_buffer(b'RIFTSIMG'+struct.pack('<4I',a['offset'],a['bytes'],a['pixel_bytes'],a['crc32']));p=dll.rift_asset_resolve(desc,a['bytes']);assert p and hashlib.sha256(C.string_at(p,a['bytes'])).hexdigest()==a['sha256']
assert C.c_uint.in_dll(dll,'rift_asset_recoveries').value==1
assert C.c_uint.in_dll(dll,'rift_asset_errors').value==0
print('PASS stale artwork file handle automatically reopens; CRC verified, no errors')
order=meta['assets'];random_order=order.copy();random.Random(1602).shuffle(random_order);checks=0
for entries in (order,list(reversed(order)),random_order):
 for a in entries:
  desc=C.create_string_buffer(b'RIFTSIMG'+struct.pack('<4I',a['offset'],a['bytes'],a['pixel_bytes'],a['crc32']))
  p=dll.rift_asset_resolve(desc,a['bytes']);assert p,a['name']
  actual=C.string_at(p,a['bytes']);assert hashlib.sha256(actual).hexdigest()==a['sha256'],a['name'];checks+=1
  assert dll.rift_asset_resolve(desc,a['bytes'])==p,'cache hit changed address'
assert C.c_uint.in_dll(dll,'rift_asset_errors').value==0
for desc,size in [(b'RIFTSIMG'+struct.pack('<4I',len(bank)+1,100,4,0),100),(b'RIFTSIMG'+struct.pack('<4I',0,100,4,0),80),(b'RIFTSIMG'+struct.pack('<4I',0,100,4,0),100)]:
 assert not dll.rift_asset_resolve(C.create_string_buffer(desc),size)
assert C.c_uint.in_dll(dll,'rift_asset_errors').value==3
dll.rift_assets_close()
print(f'PASS {checks} exact texture-byte comparisons plus cache hits, wraparound/eviction and 3 malformed-descriptor checks; {len(bank)} resource bytes, no palette/resolution/color changes')
