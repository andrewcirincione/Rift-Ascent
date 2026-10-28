from pathlib import Path
import torch,json,hashlib
from diffusers import AutoPipelineForText2Image
torch.set_num_threads(2);torch.set_num_interop_threads(1)
out=Path('refined-art');out.mkdir(exist_ok=True)
pipe=AutoPipelineForText2Image.from_pretrained('stabilityai/sd-turbo',torch_dtype=torch.float32,variant='fp16',use_safetensors=True).to('cpu');pipe.set_progress_bar_config(disable=True)
prompts=[
 'Cinematic three dimensional gothic videogame environment matte painting. Vacant broad flat dark obsidian arena floor occupying lower half, suspended in deep purple outer space. Violet nebula and tiny distant stars above. Broken cathedral arches framing far left and right. Wide horizontal low camera. Unoccupied architecture. Clear level foreground. Realistic stone, beautiful violet rim lighting, dark gritty atmosphere.',
 'Wide cinematic environment painting of an empty black stone platform floating through a purple galaxy. Level stone floor across entire lower half. Purple nebula, brilliant distant stars, fractured gothic pillars at the extreme edges. Unoccupied alien cathedral courtyard, architectural landscape asset. Dark realistic fantasy environment, restrained lighting, high detail.',
 'A deserted cosmic gothic arena, flat polished purple-black paving stones in foreground, distant shattered cathedral suspended in an indigo nebula, deep violet sky full of stars, symmetrical broken arches at sides. Wide cinematic matte painting, entirely vacant landscape, low viewpoint, beautiful realistic game environment.',
 'Empty exterior space cathedral courtyard, wide level dark stone fighting platform floating above purple clouds and a violet galaxy. Ruined gothic arches, distant stars, midnight atmosphere, physically detailed rock and stone. Cinematic full bleed environment illustration, unoccupied landscape, flat open foreground.',
 'A single bright orange flame, fire-shaped ruby glyph, isolated against pure black background. Simple recognizable upward tongues of orange and gold fire. Closeup centered fantasy game icon, clear silhouette, luminous flame.'
]
records=[]
for i,prompt in enumerate(prompts):
 name=f'arena_{i}' if i<4 else 'fire_badge';seed=88180+i
 print('GENERATE',name,flush=True)
 with torch.inference_mode():res=pipe(prompt=prompt,width=768 if i<4 else 512,height=432 if i<4 else 512,num_inference_steps=4,guidance_scale=0.0,generator=torch.Generator('cpu').manual_seed(seed))
 if getattr(res,'nsfw_content_detected',None) and any(res.nsfw_content_detected):raise RuntimeError('Safety check flagged output')
 path=out/(name+'.png');res.images[0].save(path);records.append(dict(name=name,prompt=prompt,seed=seed,model='stabilityai/sd-turbo',sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
(out/'manifest.json').write_text(json.dumps(records,indent=2))
