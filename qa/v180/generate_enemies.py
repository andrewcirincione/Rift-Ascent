"""Generate distinct enemy paintings for the 1.8 roster audit.
Uses the project's pinned diffusion pipeline. No procedural substitute art.
"""
import sys,json,hashlib
from pathlib import Path
import torch
from diffusers import AutoPipelineForText2Image
out=Path('enemy-art');out.mkdir(exist_ok=True)
torch.set_num_threads(2);torch.set_num_interop_threads(1)
subjects={
0:('Ash Crawler','a low crouching charcoal-black six-legged insectoid beast, thick cracked chitin, tiny glowing ember eyes, a small ember furnace within its chest, ash clinging to its carapace, side profile with its head at the LEFT'),
1:('Bone Sentinel','an ancient skeletal cathedral guard in tarnished iron plate armor and a frayed burgundy tabard, tall battered heater shield in front of its body, short spear pointing toward the LEFT, armored skull looking LEFT, worn realistic bone and steel'),
11:('Glimmer Moth','a gigantic dark nocturnal moth, unmistakable broad ragged moth wings folded upward, six thin insect legs, pale blue luminous wing markings and dim eyes, side profile, insect head on the LEFT, antennae extending LEFT'),
18:('Mire Oracle','a hunched hooded swamp oracle with weathered green robes, long mossy wooden staff, small hanging green lantern, concealed face, knotted roots around its boots, facing LEFT, grounded walking pose'),
21:('Ashborne Monk','a gaunt ash-covered martial monk in scorched ochre cloth robes, wrapped hands and bare stone-gray feet, a necklace of black prayer beads, guarded fighting stance with lead fist and gaze toward the LEFT'),
31:('Entropy Knight','a heavily armored obsidian knight with a narrow violet glow in the visor, cracked dark plate armor and a torn indigo cape, an enormous serrated greatsword pointing diagonally LEFT, full-body three-quarter stance facing LEFT')}
pipe=AutoPipelineForText2Image.from_pretrained('stabilityai/sd-turbo',torch_dtype=torch.float32,variant='fp16',use_safetensors=True).to('cpu');pipe.set_progress_bar_config(disable=True)
records=[]
for i in map(int,sys.argv[1].split(',')):
 name,subject=subjects[i];seed=180260+i*113
 prompt='Detailed painterly dark gothic fantasy creature illustration. ONE '+subject+'. Complete body centered with generous empty margins, entire feet and head visible. Isolated on uniform pitch-black background. Single game enemy, richly textured realistic materials, restrained dramatic rim light, gritty muted palette, clear readable silhouette. No interface, no frame, no text, no symbols, no scenery.'
 print('GENERATE',i,name,flush=True)
 with torch.inference_mode():res=pipe(prompt=prompt,width=512,height=512,num_inference_steps=4,guidance_scale=0.0,generator=torch.Generator('cpu').manual_seed(seed))
 if getattr(res,'nsfw_content_detected',None) and any(res.nsfw_content_detected):raise RuntimeError('Safety check flagged image '+str(i))
 p=out/f'enemy_{i:02d}.png';res.images[0].convert('RGB').save(p)
 records.append(dict(id=i,name=name,prompt=prompt,seed=seed,model='stabilityai/sd-turbo',sha256=hashlib.sha256(p.read_bytes()).hexdigest()))
 (out/'manifest.json').write_text(json.dumps(records,indent=2))
 print('DONE',i,flush=True)
