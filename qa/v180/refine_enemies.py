import sys,json,hashlib
from pathlib import Path
import torch
from diffusers import AutoPipelineForText2Image
torch.set_num_threads(2);torch.set_num_interop_threads(1)
out=Path('enemy-refined');out.mkdir(exist_ok=True)
prompts={0:'3D game asset, a single crouching six-legged giant charcoal beetle with ember eyes and cracked lava carapace, side view facing left, entire creature visible, pure black background, realistic Dark Souls creature render, studio lighting, centered full body with empty margins',1:'3D game character asset, full body of one skeleton soldier in rusty medieval armor, burgundy ragged tabard, large iron shield and spear, facing left, feet visible, pure black background, realistic Dark Souls character render, centered with empty margins',11:'3D game creature asset, a single complete gigantic midnight blue moth with folded ragged wings, six insect legs and glowing blue antennae, side view facing left, entire wings visible, pure black background, photorealistic fantasy render, centered with empty margins',18:'3D game character asset, full body of one sinister hooded swamp sorcerer in worn moss green robes, wooden staff and green lantern, looking left, boots visible, isolated on pure black background, realistic Dark Souls character render, centered with empty margins',21:'3D game character asset, full body of one gaunt martial monk in scorched ochre cloth and hand wrappings, ash gray skin, unarmed fighting stance facing left, entire figure and bare feet visible, isolated on pure black background, realistic Dark Souls character render, centered with empty margins',31:'3D game character asset, full body of one black obsidian armored medieval knight with violet eyes and torn indigo cape, jagged greatsword, looking left, boots visible, pure black background, realistic Dark Souls character render, centered with empty margins'}
pipe=AutoPipelineForText2Image.from_pretrained('stabilityai/sd-turbo',torch_dtype=torch.float32,variant='fp16',use_safetensors=True).to('cpu');pipe.set_progress_bar_config(disable=True)
records=[]
for i in map(int,sys.argv[1].split(',')):
 seed=730184+i*71;print('RENDER',i,flush=True)
 with torch.inference_mode():r=pipe(prompt=prompts[i],height=512,width=512,num_inference_steps=2,guidance_scale=0.0,generator=torch.Generator('cpu').manual_seed(seed))
 if getattr(r,'nsfw_content_detected',None) and any(r.nsfw_content_detected):raise RuntimeError('Flagged output')
 p=out/f'enemy_{i:02}.png';r.images[0].save(p);records.append(dict(id=i,prompt=prompts[i],seed=seed,model='stabilityai/sd-turbo',steps=2,sha256=hashlib.sha256(p.read_bytes()).hexdigest()));(out/'manifest.json').write_text(json.dumps(records,indent=2));print('DONE',i,flush=True)
