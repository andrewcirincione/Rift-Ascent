from pathlib import Path
q=Path('qa/v160');src=Path('qa/v151/qa_ppsspp.py').read_text();prefix=src[:src.index(' await shot(\'boot\')')];suffix=src[src.index('\ntry:\n'):]
prefix=prefix.replace('qa/v151','qa/v160').replace('qa151','qa160').replace('19451','19462').replace(':451',':462').replace('4851','4862')
suffix=suffix.replace('19451','19462')
body=''' await shot('boot')
 if control_mode=='resume':
  assert await val('resume_available')==1 and field(await state(),'screen')==0
  await button('cross');assert field(await state(),'screen')==2
  await shot('cold-resume');await fixture('status',(2,0,0));await shot('status');print('PASS cold boot Resume loads the saved combat',flush=True);return
 assert await val('resume_available')==0
 await button('up');await button('cross');assert field(await state(),'screen')==0
 await shot('resume-disabled');await button('down');await button('cross')
 await button('right');await button('right');await shot('character-selection');await button('cross')
 for _ in range(1500):
  if field(await state(),'screen')==18:break
  await asyncio.sleep(.1)
 assert field(await state(),'character')==2
 await shot('regent-blessing');await button('cross');await shot('new-run-map')
 for name in ['combat','no-stars','blade-preview','stars-15','stars-3','nightshade','warden','upgrade-cost','upgrade-mod','shop']:
  await fixture(name);await shot(name)
 await fixture('status',(2,0,0));await shot('status')
 await fixture('no-stars');before=await state();await button('cross');after=await state()
 assert field(after,'hand_count')==field(before,'hand_count') and field(after,'regent')==0 and field(after,'energy')==field(before,'energy')
 await fixture('combat');before=await state();await button('cross');after=await state()
 assert field(after,'regent')==field(before,'regent')-2 and field(after,'hand_count')==field(before,'hand_count')-1
 await button('select');await shot('pause');await button('cross');assert field(await state(),'screen')==0 and await val('resume_available')==1
 saved=(gamedir/'SAVE.DAT').read_bytes()[4:];await shot('saved-title');await button('cross');assert (await state())==saved
 await shot('resumed-combat')
 await fixture('choice');before=await state()
 for key in ['start','square','triangle','circle']:
  await button(key);after=await state()
  assert all(field(after,k)==field(before,k) for k in ['screen','hand_count','energy','deck_count','selection','target','potion_count','regent']) and await val('inspect_open')==0
 await shot('mandatory-choice');await button('select');await button('cross');assert field(await state(),'screen')==0
 saved=(gamedir/'SAVE.DAT').read_bytes()[4:];await button('cross');assert (await state())==saved
 await button('cross');assert field(await state(),'screen')==2
 await shot('choice-completed');await button('select');await button('cross');assert field(await state(),'screen')==0
 print('PASS final release EBOOT: all three characters; Stars rejection/payment; upgrade costs; pause icons; save/quit/title/resume; mandatory choice shortcuts and mid-choice save/resume',flush=True)
 print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)
'''
(q/'qa_ppsspp.py').write_text(prefix+body+suffix)
src=Path('qa/v151/qa_ppsspp_animation.py').read_text();prefix=src[:src.index(" for action in ['attack'")];suffix=src[src.index('\ntry:\n'):]
prefix=prefix.replace('qa/v151','qa/v160').replace('qa151','qa160').replace('19454','19464').replace(':454',':464').replace('4854','4864')
suffix=suffix.replace('19454','19464')
body=''' for action in ['attack-1','attack-2','block-1','block-2','power-1','power-2','finisher-2','death-2']:
  await fixture(action)
  async def capture():
   for i in range(32 if action.startswith('finisher') else 20):
    await shot('animation-'+action+'-'+str(i).zfill(2))
    observed=await val('animation_tick');print('TICK',action,i,observed,flush=True)
    for _ in range(600):
     if await val('animation_tick')>=observed+4:break
     await asyncio.sleep(.1)
  start=await val('animation_tick');await req('input.buttons.send',buttons={'cross':True})
  for _ in range(600):
   if await val('animation_tick')>=start+8:break
   await asyncio.sleep(.1)
  await req('input.buttons.send',buttons={'cross':False})
  await capture()
  raw=await state();assert struct.unpack_from('<i',raw,layout['hand_count'])[0]==0 or action in ['block-2','attack-2','finisher-2'],action
 print('PASS final release EBOOT: Nightshade/Regent actual-input attack, block, power, killing blow and death animation screenshots',flush=True)
 print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)
'''
(q/'qa_ppsspp_animation.py').write_text(prefix+body+suffix)
