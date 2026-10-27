"""Validate the tested release and package GitHub download assets without rebuilding it."""
from pathlib import Path
import argparse, hashlib, json, re, shutil, struct, subprocess, zipfile, zlib

ROOT = Path(__file__).resolve().parents[1]

def repo_file(name):
    path = (ROOT / name).resolve()
    assert path.is_relative_to(ROOT) and path.is_file(), name
    return path

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--validate-only', action='store_true')
    args = parser.parse_args()
    meta = json.loads((ROOT/'release/current.json').read_text())
    version, tag = meta['version'], meta['tag']
    assert re.fullmatch(r'\d+\.\d+(?:\.\d+)?[a-z]?', version)
    assert re.fullmatch('v'+re.escape(version)+r'(?:-r[2-9][0-9]*)?', tag)
    pbp = repo_file(meta['eboot']); data = pbp.read_bytes()
    assert sha(pbp) == meta['eboot_sha256']
    offsets = struct.unpack_from('<8I', data, 8)
    assert data[:4] == b'\0PBP' and tuple(sorted(offsets)) == offsets
    assert version.encode() in data[offsets[0]:offsets[1]]
    assert data[offsets[5]:offsets[5]+4] == b'RIFF', 'Missing home-screen music'
    assert data[-56:-48] == b'RIFTM311', 'Missing complete five-track soundtrack'
    size, crc = struct.unpack('<II', data[-8:])
    payload = data[-56-size:-56]
    assert len(payload) == size and zlib.crc32(payload) == crc
    cursor = 0
    for i in range(5):
        offset, samples = struct.unpack_from('<II', data, len(data)-48+i*8)
        assert offset == cursor and samples > 11025
        cursor += (samples+1)//2
    assert cursor == size
    evidence = repo_file(meta['validation']).read_text()
    assert meta['eboot_sha256'] in evidence and 'PASS final release EBOOT' in evidence
    for source in (ROOT/meta['canonical_source']).iterdir():
        assert source.is_file()
        for folder in ['GAME/src', 'portable_build']:
            if folder == 'GAME/src' and source.name == 'psp_start.c':
                continue
            assert source.read_bytes() == repo_file(f'{folder}/{source.name}').read_bytes(), source.name
    notes = repo_file(meta['notes']); screenshot = repo_file(meta['screenshot'])
    assert screenshot.read_bytes().startswith(b'\x89PNG\r\n\x1a\n')
    for script in ['GAME/verify_eboot.py', 'tools/audit_psp_imports.py']:
        subprocess.run(['python3', str(ROOT/script), str(pbp)], cwd=ROOT, check=True)
    print('PASS exact tested EBOOT, source copies, five music streams and screenshot')
    if args.validate_only:
        return
    dest = ROOT/'dist'; dest.mkdir(exist_ok=True)
    assert not list(dest.iterdir()), 'Use an empty dist folder'
    prefix = 'Rift-Ascent-'+version
    shutil.copyfile(pbp, dest/(prefix+'-EBOOT.PBP'))
    shutil.copyfile(screenshot, dest/(prefix+'-Combat.png'))
    with zipfile.ZipFile(dest/(prefix+'-PSP.zip'), 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
        archive.write(pbp, 'PSP/GAME/RIFTASCENT/EBOOT.PBP')
        archive.writestr('README.txt', 'Rift Ascent '+version+'\n\nExtract PSP to the Memory Stick root. Keep SAVE.DAT and PROFILE.DAT.\n\n'+notes.read_text())
    subprocess.run(['git', 'archive', '--format=zip', '--prefix=rift-ascent/',
                    '--output='+str(dest/(prefix+'-Source.zip')), 'HEAD'], cwd=ROOT, check=True)
    checksums = {p.name:sha(p) for p in sorted(dest.iterdir())}
    (dest/'SHA256SUMS.txt').write_text(''.join(digest+'  '+name+'\n' for name, digest in checksums.items()))
    for path in dest.glob('*.zip'):
        with zipfile.ZipFile(path) as archive:
            assert archive.testzip() is None
            if path.name.endswith('-PSP.zip'):
                assert hashlib.sha256(archive.read('PSP/GAME/RIFTASCENT/EBOOT.PBP')).hexdigest() == meta['eboot_sha256']
            else:
                assert archive.read('rift-ascent/'+meta['eboot']) == data
                assert archive.read('rift-ascent/release/current.json') == (ROOT/'release/current.json').read_bytes()
    print('PASS GitHub release assets packaged and checked:', tag)

if __name__ == '__main__':
    main()
