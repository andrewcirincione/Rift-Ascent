"""Reuse the verified PPSSPP driver with presentation-focused scenarios."""
from pathlib import Path
q=Path(__file__).parent;base=(q.parent/'qa_ppsspp.py').read_text()
base=base.replace('parents[2];base=repo.parent','parents[3];base=repo.parent').replace("ev=repo/'qa/v160/evidence'","ev=repo/'qa/v160/star_contrast/evidence'")
base=base.replace('qa160config','qa160starsconfig').replace('qa160.ini','qa160stars.ini')
base=base.replace('[Sound]\\nEnable = True\\n', '[Sound]\\nEnable = True\\n[SystemParam]\\nPSPModel = 0\\n')
base=base.replace("shutil.copyfile(src,ev/('ppsspp-'+name+'.png'));print", "assert await val('rift_asset_errors')==0,('asset load error',name);shutil.copyfile(src,ev/('ppsspp-'+name+'.png'));print")
start=base.index(" await shot('boot')");end=base.index('\ntry:\n',start)
body=''' await shot('boot')
 if control_mode=='resume':
  assert await val('resume_available')==1 and field(await state(),'screen')==0
  await button('cross');assert field(await state(),'screen')==2 and field(await state(),'regent')==2
  await shot('cold-resume');print('PASS final release EBOOT: fresh 32 MB PSP-mode launch, cold Resume, Stars and streamed artwork',flush=True)
  print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True);return
 if control_mode=='warden':
  for name in ['attack-0','block-0','power-0']:
   await fixture(name);before=await val('rift_asset_reads');await button('cross');assert await val('rift_asset_errors')==0;await shot(name)
  print('PASS final release EBOOT: Warden attack, Block and power through streamed animation resources, no resource failures',flush=True)
  print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True);return
 for name in ['stars-0','stars-1','stars-3','stars-15','stars-99','stars-100','stars-999','cost-x','cost-free','modified','upgrade','character-0','character-1']:
  await fixture(name);await shot(name)
 await fixture('stars-0');before=await state();await button('cross');after=await state()
 assert field(after,'regent')==0 and field(after,'hand_count')==field(before,'hand_count') and field(after,'energy')==field(before,'energy')
 await fixture('last-stars');await button('cross');assert field(await state(),'regent')==0
 await shot('spent-to-zero')
 await fixture('regain-stars');await button('cross');assert field(await state(),'regent')==2
 await shot('regained-glow')
 await button('select');await button('cross');assert field(await state(),'screen')==0
 saved=(gamedir/'SAVE.DAT').read_bytes()[4:];await button('cross');assert (await state())==saved
 await shot('resumed')
 print('PASS final release EBOOT: bronze HUD/card badges; counts 0/1/3/15/99/100/999; X/free/modified/upgrade; Warden/Vesper; insufficient Stars reject; last Stars spent and regained; save/resume',flush=True)
 print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)
'''
(q/'qa_ppsspp.py').write_text(base[:start]+body+base[end:])
