from pathlib import Path
import hashlib,struct,json,sys,importlib.util,zipfile,shutil
root=Path(__file__).resolve().parents[1];repo=root.parents[1]
sha=lambda b:hashlib.sha256(b).hexdigest()
if sys.argv[1]=='package':
 old=Path(sys.argv[2]).read_bytes();assert sha(old)=='4e8c80ff4a519e6dfda7a9ad6a125b654dd4dcbb216aefa48842abd587498560'
 build=(root/'source/EBOOT.PBP').read_bytes();a=struct.unpack_from('<8I',old,8);b=struct.unpack_from('<8I',build,8)
 parts=[build[b[0]:b[1]]]+[old[a[i]:a[i+1]] for i in range(1,6)]+[build[b[6]:b[7]],old[a[7]:]]
 offsets=[];cursor=40
 for part in parts:offsets.append(cursor);cursor+=len(part)
 path=root/'release/EBOOT.PBP';path.write_bytes(old[:8]+struct.pack('<8I',*offsets)+b''.join(parts))
 spec=importlib.util.spec_from_file_location('metadata',repo/'tools/fix_psp_elf_metadata.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);m.fix(path)
 final=path.read_bytes();assert final[offsets[7]:]==old[a[7]:] and final[offsets[5]:offsets[6]]==old[a[5]:a[6]]
 audit=json.loads((root/'release/PACKAGE_AUDIT.json').read_text());audit.update(sha256=sha(final),eboot_bytes=len(final),release_tag='v1.8-pre.2',startup_fix='Kernel launch argument selects the executable and saves; failed package loading displays diagnostics.',media_tail_preserved=True,previous_sha256=sha(old),hardware_tested=False)
 (root/'release/PACKAGE_AUDIT.json').write_text(json.dumps(audit,indent=2));(root/'release/EBOOT.PBP.sha256').write_text(sha(final)+'  EBOOT.PBP\n');print(json.dumps(audit,indent=2))
else:
 out=root/'bootfix/dist';out.mkdir(exist_ok=True)
 final=(root/'release/EBOOT.PBP').read_bytes();passed=json.loads((root/'evidence/bootfix/BOOT_SMOKE_PASSED.json').read_text());assert passed['sha256']==sha(final)
 notes='''# Rift Ascent 1.8 Pre-release — startup correction (pre.2)

This revision fixes a reproduced boot failure in pre.1: art, music and saves
were opened from a hardcoded RIFTASCENT_A18 directory, and a failed art open
returned silently to the PSP home screen before displaying a frame.

The executable now captures the PSP launch path, opens its own embedded art
and soundtrack, and keeps A18 profile/run files beside the actual executable.
Spaces and renamed game folders are supported. ef0 paths are supported by the
path parser; a physical PSP Go test is not claimed. A missing/damaged package
or unwritable profile produces an on-screen diagnostic instead of a silent
exit. RIFT_BOOT.LOG includes the selected path and file-operation status.

All game artwork, all 60 custom card paintings, soundtrack, XMB audio and
progression mechanics are preserved. No prior release assets/tags are replaced.

## Install
Replace only EBOOT.PBP in your existing 1.8 game folder. Do not delete A18_*
profile or run files. The installation ZIP uses PSP/GAME/RIFTASCENT_A18, but
that exact folder name is no longer required. Keep it separate from stable
versions. Do not move saves unless moving the whole 1.8 installation.

## Verification and limitations
Fresh PSPDEV compile, native progression tests, sanitized launch-path tests,
and exact-EBOOT PPSSPP boot tests. The old binary fails in a renamed directory;
the corrected binary reaches title/save-slot screens with zero asset errors
in both normal and renamed directories, and loads all five music streams.
A deliberately damaged disposable package displays the diagnostic screen.
These are emulator checks, not a physical-PSP confirmation. The user's newly
mentioned boot log was not attached/available during this correction; this
fix addresses a reproduced startup defect, not a proven diagnosis of every
possible black-screen failure.
'''
 (out/'RELEASE_NOTES.md').write_text(notes);(root/'bootfix/RELEASE_NOTES.md').write_text(notes)
 for name in ['EBOOT.PBP','EBOOT.PBP.sha256','PACKAGE_AUDIT.json']:shutil.copyfile(root/'release'/name,out/name)
 with zipfile.ZipFile(out/'Rift-Ascent-1.8-Pre-release-2-PSP.zip','w',zipfile.ZIP_DEFLATED) as z:
  z.writestr('PSP/GAME/RIFTASCENT_A18/EBOOT.PBP',final);z.writestr('README.txt',notes)
 with zipfile.ZipFile(out/'Rift-Ascent-1.8-Pre-release-2-Source.zip','w',zipfile.ZIP_DEFLATED) as z:
  for folder in ['source','bootfix','tests']:
   for p in sorted((root/folder).rglob('*')):
    if p.is_file() and 'dist' not in p.parts and p.suffix not in ['.o','.elf','.PBP'] and p.name not in ['PARAM.SFO','fixtures','symbols.txt','game.map']:
     z.write(p,str(p.relative_to(root)))
 with zipfile.ZipFile(out/'Rift-Ascent-1.8-Pre-release-2-Validation.zip','w',zipfile.ZIP_DEFLATED) as z:
  for p in sorted((root/'evidence/bootfix').rglob('*')):
   if p.is_file():z.write(p,p.name)
 manifest={p.name:dict(sha256=sha(p.read_bytes()),bytes=p.stat().st_size) for p in sorted(out.iterdir()) if p.is_file() and p.name not in ['RELEASE_MANIFEST.json','SHA256SUMS.txt']}
 (out/'RELEASE_MANIFEST.json').write_text(json.dumps(manifest,indent=2));(out/'SHA256SUMS.txt').write_text(''.join(v['sha256']+'  '+k+'\n' for k,v in manifest.items()));print('PASS final release files',sha(final))
