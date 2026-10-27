from pathlib import Path
base=Path('qa/v150b/qa_ppsspp_status.py').read_text().replace('qa/v150b/evidence','qa/v151/evidence').replace('qa150bstatus','qa151').replace('19427','19451').replace(':427',':451').replace('4827','4851')
a=base.index(' for tab,scroll,name in [(1,0,');b=base.index('\ntry:',a)
base=base[:a]+''' def field(raw,key,index=0):return struct.unpack_from('<i',raw,layout[key]+4*index)[0]
 await shot('boot')
 await button('cross');await button('right');await shot('character-selection');await button('cross')
 for _ in range(600):
  if field(await state(),'screen')==18:break
  await asyncio.sleep(.1)
 assert field(await state(),'character')==1
 await button('cross');await shot('new-run-map')
 for name in ['combat','upgrade-cost','upgrade-mod','mystery','reward-potion']:
  await fixture(name);await shot(name)
 # Exact input through full-belt handling, discard, taking potion first, card preview.
 await fixture('reward-potion');before=await state();n=field(before,'deck_count')
 await button('cross');assert field(await state(),'potion_count')==3;await shot('full-belt')
 await button('start');assert await val('inspect_open')==1 and await val('inspect_tab')==3
 await button('down');await shot('inventory-potions');await button('triangle')
 assert field(await state(),'potion_count')==2 and field(await state(),'selection')==4
 await button('start');assert await val('inspect_open')==0
 await button('cross');raw=await state();assert field(raw,'potion_count')==3 and field(raw,'v151')==-1 and field(raw,'deck_count')==n
 await shot('potion-taken-card-pending');await button('triangle');assert field(await state(),'screen')==19;await shot('reward-preview');await button('circle');await button('cross');assert field(await state(),'deck_count')==n+1 and field(await state(),'screen')==1
 # Skipping card still leaves the potion choice; leaving potion then returns to map.
 await fixture('reward-potion');await button('up');await button('cross');raw=await state();assert field(raw,'screen')==3 and field(raw,'v151',1)==1 and field(raw,'v151')==5
 await shot('card-skipped-potion-pending');await button('right');await button('cross');assert field(await state(),'screen')==1
 # Every menu preserves its engine state through inventory/deck/relic browsing.
 for name in ['shop','map','rest','mystery','mystery-card','relic-result','rewards','upgrade-cost']:
  await fixture(name);before=await state();await button('start');assert await val('inspect_open')==1
  await button('right');assert await val('inspect_tab')==4
  if name=='shop':await shot('shop-relics')
  await button('square');assert await val('inspect_tab')==5
  if name=='rest':await shot('campfire-deck')
  await button('start');after=await state();assert before==after,('overlay changed menu',name)
 # Upgrade payment and resulting deck agree with the visible preview.
 await fixture('upgrade-cost');await button('cross');raw=await state();assert field(raw,'deck')==(111|256) and field(raw,'screen')==1
 await fixture('inventory');await button('start');await button('right');assert await val('inspect_tab')==3
 await button('triangle');assert field(await state(),'potion_count')==2;await button('start');await shot('combat-after-discard')
 await fixture('new-relics',overlay=(4,12,0));await shot('new-relic-inventory')
 # Forced card decisions remain mandatory and cannot open overlays.
 await fixture('sly-choice');before=await state()
 for key in ['start','square','triangle','circle']:
  await button(key);assert (await state())==before and await val('inspect_open')==0
 await button('cross');assert field(await state(),'screen')==2
 print('PASS packaged EBOOT: input navigation, independent rewards, full-belt discard/take, all menu overlays, cost-changing upgrade, forced-choice safety',flush=True)
 print('SHA256',hashlib.sha256((gamedir/'EBOOT.PBP').read_bytes()).hexdigest(),flush=True)
'''+base[b:]
Path('qa/v151/qa_ppsspp.py').write_text(base)
s=Path('qa/v150b/qa_ppsspp_animation.py').read_text().replace('qa/v150b/evidence','qa/v151/evidence').replace('qa150banimation','qa151animation').replace('19428','19452').replace(':428',':452').replace('4828','4852')
s=s.replace("for action in ['attack','block','power']:","for action in ['attack','warden-attack','block','power','finisher','death']:")
s=s.replace('for i in range(32):','for i in range(24):')
s=s.replace("assert struct.unpack_from('<i',raw,layout['hand_count'])[0]==0,action","assert struct.unpack_from('<i',raw,layout['hand_count'])[0]==0,action")
Path('qa/v151/qa_ppsspp_animation.py').write_text(s)
