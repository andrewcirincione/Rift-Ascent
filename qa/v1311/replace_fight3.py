"""Replace only RiftFight3 in a complete 1.3.11 EBOOT; preserve its executable and other audio."""
from pathlib import Path
import argparse, hashlib, json, runpy, struct, zlib

def unpack(blob):
    assert blob[:4] == b'\0PBP' and blob[-56:-48] == b'RIFTM311'
    size, crc = struct.unpack('<II', blob[-8:])
    start = len(blob) - 56 - size
    assert start > 40
    payload = blob[start:-56]
    assert zlib.crc32(payload) == crc
    parts, counts = [], []
    cursor = 0
    for i in range(5):
        off, count = struct.unpack_from('<II', blob, len(blob)-48+i*8)
        assert off == cursor and count > 0
        part = payload[off:off+(count+1)//2]
        assert len(part) == (count+1)//2
        parts.append(part); counts.append(count); cursor += len(part)
    assert cursor == len(payload)
    return blob[:start], parts, counts

def replace(baseline, recording, output):
    root = Path(__file__).resolve().parents[2]
    codec = runpy.run_path(str(root/'tools/embed_churn.py'))
    original = baseline.read_bytes()
    executable, parts, counts = unpack(original)
    samples = codec['decode_wav'](recording)
    parts[4] = codec['encode'](samples); counts[4] = len(samples)
    payload = b''.join(parts); entries = []; cursor = 0
    for part, count in zip(parts, counts):
        entries.append(struct.pack('<II', cursor, count)); cursor += len(part)
    updated = executable + payload + b'RIFTM311' + b''.join(entries) + struct.pack('<II', len(payload), zlib.crc32(payload))
    actual, streams, actual_counts = unpack(updated)
    old_executable, old_streams, old_counts = unpack(original)
    assert actual == old_executable and streams[:4] == old_streams[:4] and actual_counts[:4] == old_counts[:4]
    assert streams[4] != old_streams[4]
    # Decode the entire replacement and verify both signal and tracking error.
    import numpy as np
    decoded = np.asarray(codec['decode'](streams[4], actual_counts[4]), dtype=np.float64)
    source = np.asarray(samples, dtype=np.float64)
    assert len(decoded) == len(source) and len(source) > 11025
    rms = float(np.sqrt(np.mean(decoded**2)))
    correlation = float(np.corrcoef(source, decoded)[0,1])
    assert rms > 100 and correlation > .98
    output.write_bytes(updated)
    report = {'version':'1.3.11', 'replacement':'RiftFight3', 'source':recording.name,
              'source_sha256':hashlib.sha256(recording.read_bytes()).hexdigest(),
              'seconds':len(samples)/11025, 'samples':len(samples), 'rms':rms, 'correlation':correlation,
              'executable_and_splash_unchanged':True, 'other_four_tracks_unchanged':True,
              'eboot_sha256':hashlib.sha256(updated).hexdigest()}
    (output.parent/'fight3-audit.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report, indent=2))

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('baseline', type=Path); p.add_argument('recording', type=Path); p.add_argument('output', type=Path)
    a = p.parse_args(); replace(a.baseline, a.recording, a.output)
