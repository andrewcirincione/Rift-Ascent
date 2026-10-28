"""Generate actual diffusion-model paintings, never geometric placeholders.
Each output has its own recorded prompt/seed. Run one character or world group.
Model: stabilityai/sd-turbo, official Diffusers pipeline. Safety defaults retained.
"""
import os,json,hashlib,sys,time
from pathlib import Path
import torch
from diffusers import AutoPipelineForText2Image
from PIL import Image
GROUP=sys.argv[1];out=Path('art-output')/GROUP;out.mkdir(parents=True,exist_ok=True)
torch.set_num_threads(2);torch.set_num_interop_threads(1)
style='Detailed hand-painted dark gothic fantasy book illustration, cinematic lighting, strong recognizable silhouette, textured steel and cloth, rich atmosphere, full bleed unlettered painting. '
subjects={
'warden':[
('Iron Benediction','a black armored knight raising a silver shield into warm light in a ruined chapel'),
('Grave Cleaver','a massive worn cleaver sword leaning across an ancient stone grave, ash on steel'),
('Ember Tithe','an iron gauntlet dropping three glowing embers into a ritual stone bowl'),
('Rivet Strike','a riveted armored fist striking a cracked iron gate, bright metallic sparks'),
('Keep Standing','a battered black knight bracing on a greatsword amid cold rain, resolute posture'),
('Ashhook','an ornate hooked sword pulling through swirling gray ash, one clear weapon'),
('Furnace Oath','a black armored knight swearing before a vast orange furnace, dramatic iron architecture'),
('Grief Engine','a colossal ancient machine of iron gears beneath a ruined cathedral, funeral candles'),
('Bastion Breaker','a black knight driving a heavy shield against a crumbling stone wall'),
('Red Ledger','a weathered crimson book open on a stone altar, ember runes without legible writing'),
('Cinder March','a lone black knight walking through a field of extinguished bonfires, orange embers'),
('Iron Reprisal','two crossed heavy iron swords over a battered heater shield, clear symmetric composition'),
('Grave Sovereign','a crowned armored warrior on a throne of old shields in a candlelit crypt'),
('Coffin Storm','three flying iron blades sweeping through dust in an empty gothic mausoleum'),
('Last Bastion','a single steadfast knight shielding a narrow ruined bridge from a golden storm'),
('Debt of Ash','iron scales weighing glowing cinders against an ancient black coin'),
('Oath of Ruin','a black greatsword splitting an empty stone altar, bright sparks and floating rubble'),
('Final Verdict','an enormous executioner sword planted before an empty judge throne, dramatic side light'),
('Crown of Cinders','a royal black iron crown glowing with orange coals on a dark velvet cloth'),
('The Last Wall','a black armored guardian holding a towering shield before a purple cosmic storm')],
'vesper':[
('Venom Stitch','a white hooded assassin holding a thin dagger threaded with green poison, close-up'),
('Silk Guard','a white hooded rogue wrapping a silver dagger in flowing protective silk'),
("Raven's Draw",'a white gloved hand releasing a black raven over moonlit playing parchments'),
('Glass Needle','one translucent crystal dagger with green reflections against a dark stone wall'),
('Night Supplies','two polished throwing daggers and a rolled white cloak laid on a wooden crate'),
('Fade Step','a white cloaked assassin stepping silently between moonlit gothic pillars'),
('Laced Ambush','a hidden silver dagger above a green glass poison flask in tall night grass'),
('Moonlit Exchange','two white gloved hands exchanging a sealed scroll and a dagger beneath moonlight'),
('Hushblade','a white hooded rogue holding a matte black knife upright in falling snow'),
('Caustic Veil','a white cloak billowing through translucent green mist, silver blade in foreground'),
('Dusk Pact','two ornate silver daggers crossed over a dusk-colored wax seal'),
('Ghost Thread','a spectral white hooded figure trailing silver silk through a blue ruined corridor'),
('Venom Cathedral','a vast ruined cathedral illuminated by luminous green vines and poison mist'),
('Knife Requiem','three silver throwing knives hovering above candles in a dark chapel'),
('Empty Scabbard','an elegant empty black dagger sheath lying across a white cloak, moonlit close-up'),
('Dark Harvest','a white hooded rogue collecting glowing green night flowers in an old graveyard'),
('Midnight Refuge','a white cloaked traveler sheltered under a silver translucent dome in midnight rain'),
('Bell of Silence','an ancient silver bell wrapped in white cloth, hanging in a ruined dark tower'),
('The Last Whisper','a white hooded assassin with one finger at a closed mask, green dagger glint'),
('Queen of No One','a white hooded rogue beneath a floating broken silver crown, purple twilight')],
'proxy':[
('Comet Edict','a blue robed crowned wizard directing a bright comet across a purple night sky'),
('Vault Ward','a luminous blue magic shield guarding a sealed ornate celestial vault'),
('Forge Spark','a hammer striking a star-shaped spark on a magical anvil in a blue-lit forge'),
('Astral Census','a blue gauntlet arranging glowing miniature stars above an unlettered parchment'),
('Crown Shard','a broken fragment of a blue crystal crown floating in a starry void'),
('Orbit Guard','a blue cloaked wizard protected by three orbiting tiny stars'),
('Sovereign Toll','a golden coin bearing an abstract crown beside a glowing blue greatsword'),
('Meteor Temper','a celestial sword forged within a falling silver meteor, bright blue sparks'),
('Star Tax','a royal blue gauntlet plucking one radiant star from a cosmic tapestry'),
('Throne Echo','two mirrored spectral thrones in a sapphire cathedral, luminous ripples'),
('Gravity Mantle','a crowned blue wizard wearing a cloak that bends floating stone fragments around it'),
('Empty Scepter','a hollow ceremonial scepter with a small blue star suspended inside its crown'),
('Collapsing Orbit','three bright meteors orbiting a collapsing violet star over empty ruins'),
('Royal Arsenal','a floating royal array of glowing blue swords and spears in a dark hall'),
('Heirless Decree','an empty blue throne beneath a broken gold crown, a sealed unlettered scroll'),
('Solar Judgment','a crowned blue wizard raising a brilliant golden sun above a ruined cathedral'),
('Unmade Kingdom','a ruined floating kingdom disassembling into blue stars, sweeping cosmic vista'),
('Crown Beyond','a blue crystal crown floating above the edge of a purple galaxy'),
('Last Constellation','a majestic constellation shaped like a sword in a deep violet star field'),
('Throne of Nothing','a lone empty obsidian throne floating in purple space, faint blue crown aura')]
}
items=[]
if GROUP in subjects:
 offset={'warden':334,'vesper':354,'proxy':374}[GROUP]
 for i,(name,subject) in enumerate(subjects[GROUP]):items.append((f'card_{offset+i:03d}',style+subject,512,512))
 hero={'warden':'a black armored knight with a worn greatsword and crimson cloak','vesper':'a slender white hooded assassin with silver daggers','proxy':'a regal blue robed masked wizard with a star crown'}[GROUP]
 scenes=[('ending','standing alone before a fallen gothic throne at dawn, exhausted triumph'),('beyond','discovering an immense violet doorway behind the shattered throne, stars beyond it'),('before','facing a huge distant cosmic gatekeeper across a purple obsidian arena'),('true','walking away as an ancient cosmic chain breaks into stars, quiet freedom, purple sunrise')]
 for scene,subject in scenes:items.append((f'{GROUP}_{scene}',style+hero+' '+subject+'. Wide cinematic environment, entire body visible, no interface.',768,432))
else:
 items.append(('void_arena',style+'an EMPTY broad flat obsidian fighting platform floating in deep purple outer space, broken gothic arches at the sides, indigo and violet nebulae, distant stars, luminous galaxy, level unobstructed arena floor, no people, no creatures, no writing, no interface',768,432))
 bosses=[('boss_warden','a colossal armored cosmic jailer with a broken iron halo and a huge key-shaped sword'),('boss_vesper','a slender faceless cosmic witness draped in violet ribbons and silver masks'),('boss_proxy','an ancient floating obsidian king with a shattered crown and a blue star in its chest')]
 for name,subject in bosses:items.append((name,style+subject+', full body centered, isolated against a plain black background, facing left, readable silhouette',512,512))
 potions=[('Frostfire Flask','a blue and orange frosted round glass potion'),('Nightsap','a dark green flask shaped like a leaf'),('Sunder Glass','a jagged amber potion flask'),('Ironwake','a steel-bound cobalt potion bottle'),('Rift Draught','a violet swirling potion in a tall crystal flask'),('Starlight Elixir','a sapphire vial filled with tiny stars'),('Glassskin','a clear pearlescent potion in a diamond flask'),('Cleansing Rain','a pale turquoise raindrop potion bottle'),('Red Communion','a dark ruby ceremonial potion chalice'),('Quietus','a narrow black potion vial with a silver stopper'),('Ashwater','a smoky gray flask glowing softly orange'),('Thornwine','a green potion bottle wrapped in silver thorn vines'),('Time Salt','a golden hourglass flask filled with glowing sand'),('Echo Tonic','a twin-necked indigo potion bottle'),('Fortress Dew','a shield-shaped jade potion flask'),('Void Ink','a black ink bottle with purple luminous liquid'),('Blade Vial','a long glass flask with silver dagger-like stoppers'),('Sovereign Oil','a royal sapphire bottle with a gold crown stopper'),('Bloodmercy','a warm crimson healing potion in a round brass flask'),('Abyssal Tincture','a deep violet shell-shaped potion bottle')]
 for i,(name,subject) in enumerate(potions):items.append((f'potion_{i+6:02d}',style+subject+', one object centered, isolated on a plain black background, ornate alchemy illustration',512,512))
 relics=[('Ashen Lantern','an ornate iron lantern filled with glowing cinders'),('Vowstone','a small ancient oath stone wrapped in silver chain'),('Rift Compass','a silver compass pointing toward a tiny violet rift'),('Nightglass Spur','a black glass riding spur tipped with green light'),('Starmetal Seal','a royal wax seal cast from glowing blue star metal'),('Mourning Crown','a small tarnished silver crown hung with black pearls'),('Void Hourglass','an obsidian hourglass containing a purple galaxy'),('Cinder Reliquary','a gold reliquary holding a single burning coal'),('Pilgrim Quill','a white feather quill in an ornate dark inkwell'),('Dawnless Sigil','a purple crystal sigil mounted in gothic silver')]
 for i,(name,subject) in enumerate(relics):items.append((f'relic_{i+64:02d}',style+subject+', one small object centered, isolated on plain black, fantasy inventory illustration',512,512))
 items.extend([('badge_flame',style+'a small stylized orange flame inside a dark metal round medal, centered, isolated black',512,512),('badge_rift',style+'a small violet spiral portal inside a dark metal round medal, centered, isolated black',512,512)])
print('LOADING MODEL',GROUP,len(items),flush=True)
pipe=AutoPipelineForText2Image.from_pretrained('stabilityai/sd-turbo',torch_dtype=torch.float32,variant='fp16',use_safetensors=True)
pipe.to('cpu');pipe.set_progress_bar_config(disable=True)
manifest=[]
for i,(name,prompt,w,h) in enumerate(items):
 seed=180000+sum(map(ord,GROUP))*100+i
 print('GENERATE',i,name,flush=True);start=time.time()
 with torch.inference_mode():result=pipe(prompt=prompt,height=h,width=w,num_inference_steps=4,guidance_scale=0.0,generator=torch.Generator('cpu').manual_seed(seed))
 if getattr(result,'nsfw_content_detected',None) and any(result.nsfw_content_detected):raise RuntimeError('Safety checker flagged '+name)
 image=result.images[0].convert('RGB');path=out/(name+'.png');image.save(path)
 manifest.append(dict(name=name,prompt=prompt,seed=seed,model='stabilityai/sd-turbo',steps=4,size=[w,h],sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
 (out/'manifest.json').write_text(json.dumps(manifest,indent=2));print('DONE',name,round(time.time()-start,1),flush=True)
print('ALL IMAGES GENERATED',GROUP,flush=True)
