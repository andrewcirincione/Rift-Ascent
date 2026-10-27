from pathlib import Path
q=Path(__file__).parent
s=Path('qa/v162/qa_ppsspp.py').read_text().replace('v162','v170').replace('qa162','qa170').replace('19464','19470').replace(':464',':470').replace(':4864',':4870').replace('port=4864','port=4870')
# Fixture setup may install any valid persisted screen; all subsequent actions
# use real emulated buttons. Keep the existing debugger/screenshot plumbing.
s=s.replace("if control_mode!='resume':(gamedir/'SAVE.DAT').unlink(missing_ok=True)","if control_mode!='resume':(gamedir/'SAVE.DAT').unlink(missing_ok=True)")
s=s.replace("'r161_transition_active']", "'r161_transition_active','r170_story_queue']")
s=s.replace("await val('animation_tick')-await val('r161_transition_tick')>=48", "await val('animation_tick')-await val('r161_transition_tick')>=await val('r161_transition_length')")
a=s.index(' def field(raw,key,index=0):');b=s.index('\ntry:\n time.sleep(1)',a)
flow=r''' def field(raw,key,index=0):return struct.unpack_from('<i',raw,layout[key]+4*index)[0]
 async def ticks(n):
  start=await val('animation_tick')
  for _ in range(900):
   if await val('animation_tick')>=start+n:return
   await asyncio.sleep(.15)
  raise AssertionError('frames stopped')
 async def pcm():return base64.b64decode((await req('memory.read',address=symbols['audio_buffer'],size=2048))['base64'])
 async def settled():
  for _ in range(900):
   if not await val('r161_transition_active') and await val('action_cooldown')==0:return
   await asyncio.sleep(.15)
  raise AssertionError('transition stuck')
 async def fade_action(key,label):
  # Stop before the next frame renders, preserving each actual PSP framebuffer.
  # Wall-clock polling can miss an eight-frame fade on a fast emulator.
  await settled();await command('cpu.stepping')
  previous=await val('r161_transition_tick')
  await req('input.buttons.send',buttons={key:True})
  for _ in range(6):
   await next_frame()
   if await val('r161_transition_tick')!=previous:break
  start=await val('r161_transition_tick');length=await val('r161_transition_length')
  assert start!=previous and await val('r161_transition_active'),(label,field(await state(),'screen'),start,previous)
  await req('input.buttons.send',buttons={key:False})
  for j,age in enumerate([8,length//2,length-8,length+4]):
   await next_frame(start+age+4)
   await shot(label+'-'+str(j))
  await req('cpu.breakpoint.remove',address=symbols['game_render_frame']);await command('cpu.resume');await settled()
 frame_target=0
 async def next_frame(target=None):
  nonlocal frame_target
  if target is None:target=max(await val('animation_tick'),frame_target)+4
  frame_target=target
  await req('cpu.breakpoint.add',address=symbols['game_render_frame'],condition=f'a3 >= 0x{target:x}',enabled=True)
  await command('cpu.resume')
  for _ in range(900):
   if (await req('cpu.status')).get('stepping'):return
   await asyncio.sleep(.15)
  raise AssertionError('frame breakpoint did not stop')
 await shot('boot')
 if control_mode=='resume':
  assert await val('resume_available')==1
  await button('cross');assert field(await state(),'screen')==27 and field(await state(),'selection')==1
  assert await val('music_route')==0;await shot('cold-resume-boss');print('PASS cold Resume into saved boss dialogue, RiftBoss selected',flush=True);return
 # Every character and act illustration, including endless, on native PSP output.
 for c in ([] if control_mode=='controls' else range(3)):
  for a in range(1,5):
   await fixture(f'entry-{c}-{a}-0');await ticks(12);assert await val('music_route')&256
   assert not any(await pcm()),('entry audio not silent',c,a)
   before=await state()
   for key in (['triangle','square','start','circle'] if a==1 else []):
    await button(key);assert await state()==before and not await val('inspect_open')
   await shot(f'entry-{c}-{a}')
  for a in range(1,4):
   await fixture(f'boss-{c}-{a}-0-0');await shot(f'boss-{c}-{a}-0');assert await val('music_route')==0 and await val('soundtrack_current')==0
   await button('cross');assert field(await state(),'selection')==1;await shot(f'boss-{c}-{a}-1')
   await button('cross');assert field(await state(),'selection')==2;await shot(f'boss-{c}-{a}-2')
   left=await val('left_samples');await button('cross');assert field(await state(),'screen')==2 and await val('music_route')==0 and await val('soundtrack_current')==0
   assert await val('left_samples')<left,'RiftBoss restarted at combat'
   # Show the character and matching boss unobscured by an open card.
   for _ in range(field(await state(),'hand_count')):await button('right')
   await shot(f'fight-{c}-{a}')
 if control_mode=='scenes':
  print('PASS exact 1.7 EBOOT: 12 entry illustrations, all 9 three-beat boss scenes and matching fights, entry silence, uninterrupted RiftBoss',flush=True)
  print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True);return
 # Previous event -> scene -> following event fades and track handoffs.
 await fixture('blessing-0-1');await fade_action('cross','entry-in-fade');assert field(await state(),'screen')==26 and await val('music_route')&256
 await fade_action('cross','entry-map-fade');assert field(await state(),'screen')==1 and field(await state(),'floor')==0 and await val('music_route')==1
 await fixture('map-boss-2-2');await fade_action('cross','boss-in-fade');assert field(await state(),'screen')==27 and await val('music_route')==0
 await button('cross');await button('cross');left=await val('left_samples');await fade_action('cross','boss-fight-fade');assert field(await state(),'screen')==2 and await val('left_samples')<left and await val('music_route')==0
 # Hold X through the opening fade: one edge must not advance the whole exchange.
 await fixture('map-boss-0-1');await req('input.buttons.send',buttons={'cross':True});await ticks(120);await req('input.buttons.send',buttons={'cross':False});await ticks(8)
 assert field(await state(),'screen')==27 and field(await state(),'selection')==0
 # Queue three actual edges during an entry fade; no queued click chooses a map node.
 await fixture('blessing-1-1');await command('cpu.stepping')
 for edge in range(8):
  await req('input.buttons.send',buttons={'cross':edge%2==0});await next_frame()
 await req('cpu.breakpoint.remove',address=symbols['game_render_frame']);await command('cpu.resume')
 await ticks(64);await settled();assert field(await state(),'screen')==1 and field(await state(),'floor')==0
 await shot('rapid-x-map-safe')
 # Shortcuts still cannot open inventory in dialogue; save/resume preserves exact beat.
 await fixture('boss-2-3-0-1');before=await state();await button('select');assert field(await state(),'screen')==13
 await button('circle');assert await state()==before and await val('music_route')==0
 await button('select');await button('cross');assert field(await state(),'screen')==0
 saved=(gamedir/'SAVE.DAT').read_bytes()[4:];assert field(saved,'screen')==27 and field(saved,'selection')==1
 await shot('saved-title-three-options');await button('down');await button('cross');assert field(await state(),'screen')==25
 await button('cross');assert (gamedir/'SAVE.DAT').read_bytes()[4:]==saved
 # Restored title position is Begin Ascent; return to Resume.
 await button('up');await button('cross');assert await state()==saved;await shot('resumed-boss')
 print('PASS exact 1.7 EBOOT: fades, held/rapid X, shortcuts and save/resume'+('' if control_mode=='controls' else '; 12 entry illustrations, all 9 three-beat boss scenes and fights, silence, uninterrupted RiftBoss'),flush=True)
 print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)
'''
s=s[:a]+flow+s[b:]
# Ordinary button checks should also wait for save/resume fades before taking
# their final-state screenshots. Frame-sampled fades above remain independent.
s=s.replace("if await val('animation_tick')>=end and await val('action_cooldown')==0", "if await val('animation_tick')>=end and not await val('r161_transition_active') and await val('action_cooldown')==0")
(q/'qa_ppsspp.py').write_text(s)
