"""Package the already validated 1.3.10 private release and its evidence."""
from pathlib import Path
import hashlib
import json
import shutil
import zipfile

root = Path(__file__).resolve().parents[2]
workspace = root.parent
evidence = workspace / 'evidence1310'
output = workspace / 'deliverables1310'
output.mkdir(exist_ok=True)
saved_evidence = root / 'qa/v1310/evidence'
saved_evidence.mkdir(exist_ok=True)

for log, required in [
    ('ppsspp-results.txt', 'PASS'),
    ('ppsspp-final.txt', 'PASS final release screenshots'),
    ('qa-results.txt', 'PASS all six attack stance anchors'),
    ('relic-tests.txt', 'PASS all 28 relic effects'),
]:
    assert required in (evidence / log).read_text(), log

for source in (root / 'qa/v1310/source').iterdir():
    if source.is_file():
        for target in [root / 'GAME/src' / source.name, root / 'portable_build' / source.name]:
            if source.name == 'psp_start.c' and target.parent == root / 'GAME/src':
                continue
            assert source.read_bytes() == target.read_bytes(), (source, target)

for pattern in ['ppsspp-*.png', 'state-*.dat', 'impact-*.s16le']:
    for source in evidence.glob(pattern):
        shutil.copy2(source, saved_evidence / source.name)
for name in [
    'qa-results.txt', 'relic-tests.txt', 'ppsspp-results.txt',
    'ppsspp-final.txt',
    'balance-independent.txt',
    'psp-build.log', 'asset-audit.txt', 'import-audit.txt', 'structure-audit.txt',
]:
    shutil.copy2(evidence / name, saved_evidence / name)

pbp = evidence / 'EBOOT.PBP'
pbp_hash = hashlib.sha256(pbp.read_bytes()).hexdigest()
for config in ['qa1310releaseconfig', 'qa1310finalconfig']:
    tested = workspace / 'runtime' / config / 'ppsspp/PSP/GAME/RIFTASCENT/EBOOT.PBP'
    assert hashlib.sha256(tested.read_bytes()).hexdigest() == pbp_hash
manifest = {
    'version': '1.3.10',
    'eboot_bytes': pbp.stat().st_size,
    'eboot_sha256': pbp_hash,
    'save_version': 18,
    'save_game_bytes': 3144,
    'emulator': 'PPSSPP 1.20.4',
    'physical_psp_tested': False,
    'music': 'All three original tracks preserved byte for byte',
    'soundtrack_sha256': '77c0d56c3d352eaf79e3d902d7c911dc254efe79dc8c282f870549b92640401a',
    'final_visual_check': 'ppsspp-results.txt and ppsspp-final.txt: art, facing, size, arrows, END, event reveal, intents, potions, Whirlwind, Block and animation',
    'additional_attack_cases': 81312,
    'independent_simulation': {'seeds': 1000, 'wins': 377, 'stalls': 0},
}
(saved_evidence / 'release.json').write_text(json.dumps(manifest, indent=2) + '\n')

prefix = 'Rift-Ascent-1.3.10'
shutil.copy2(pbp, output / (prefix + '-EBOOT.PBP'))
for source, suffix in [
    ('ppsspp-final-release-combat.png', 'Combat'),
]:
    shutil.copy2(evidence / source, output / (prefix + '-' + suffix + '.png'))

install = '''Rift Ascent 1.3.10

Extract the PSP folder to your Memory Stick root, or replace
PSP/GAME/RIFTASCENT/EBOOT.PBP with the included EBOOT.PBP.
Keep your existing SAVE.DAT and PROFILE.DAT files. Save format remains version 18.

PPSSPP: open PSP/GAME/RIFTASCENT/EBOOT.PBP.
This release was tested in PPSSPP 1.20.4; it has not been tested on a physical PSP.
The original three music tracks are included unchanged.

Changes and reproducible validation are in qa/v1310/README.md in the source ZIP.
'''
with zipfile.ZipFile(output / (prefix + '-PSP.zip'), 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    archive.write(pbp, 'PSP/GAME/RIFTASCENT/EBOOT.PBP')
    archive.writestr('README.txt', install)

excluded_suffixes = {'.o', '.elf', '.pbp', '.ppm', '.pyc', '.wav', '.mp3', '.zip'}
with zipfile.ZipFile(output / (prefix + '-Source.zip'), 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as archive:
    for source in sorted(root.rglob('*')):
        if not source.is_file():
            continue
        relative = source.relative_to(root)
        if any(part in {'.git', '__pycache__'} for part in relative.parts):
            continue
        if source.suffix.lower() in excluded_suffixes or source.name in {'game.map', 'PARAM.SFO'} or source.name.startswith('core.'):
            continue
        archive.write(source, 'rift-ascent/' + str(relative))

for archive_path in output.glob('*.zip'):
    with zipfile.ZipFile(archive_path) as archive:
        assert archive.testzip() is None, archive_path
        if archive_path.name.endswith('-PSP.zip'):
            assert hashlib.sha256(archive.read('PSP/GAME/RIFTASCENT/EBOOT.PBP')).hexdigest() == pbp_hash
        else:
            assert archive.read('rift-ascent/qa/v1310/source/combat_fx.inc') == (root / 'qa/v1310/source/combat_fx.inc').read_bytes()
            assert 'rift-ascent/GAME/src/rift1310_art.h' in archive.namelist()
print(json.dumps({'release': manifest, 'files': [{'name': f.name, 'bytes': f.stat().st_size, 'sha256': hashlib.sha256(f.read_bytes()).hexdigest()} for f in sorted(output.iterdir()) if f.is_file()]}, indent=2))
