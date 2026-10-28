"""Reassemble the exact stereo soundtrack from the checked-in encoded tracks."""
from pathlib import Path
import hashlib, json, struct, zlib

root = Path(__file__).resolve().parent / 'audio'
manifest = json.loads((root / 'manifest.json').read_text())
bank = bytearray()
footer = bytearray(b'RIFTAU18')
assert len(manifest['tracks']) == 10
for index, track in enumerate(manifest['tracks']):
    data = (root / (track['name'] + '.ima')).read_bytes()
    assert track['index'] == index
    assert len(data) == track['frames']
    assert hashlib.sha256(data).hexdigest() == track['encoded_sha256']
    footer += struct.pack('<III', len(bank), len(data), track['rate'])
    bank += data
footer += struct.pack('<II', len(bank), zlib.crc32(bank))
assert len(footer) == 136
(root / 'soundtrack.bank').write_bytes(bank + footer)
print('Verified and packed 10 stereo tracks:', len(bank), 'bytes')
