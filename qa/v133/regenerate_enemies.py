#!/usr/bin/env python3
"""Thirty-six PSP-friendly RGB565 sprites from nine original enemy illustrations.
27 new enemies use individually color-graded, mirrored/augmented derivative artwork.
Opaque magenta is a color key; sprite alpha is flattened at compile time.
"""
from pathlib import Path
from PIL import Image,ImageDraw,ImageEnhance,ImageFilter
import numpy as np
import colorsys
import sys
root=Path(sys.argv[1]).resolve()
(root/'screenshots').mkdir(parents=True,exist_ok=True)
art=root/'assets/runtime'; W,H=104,110
source=[Image.open(art/f'enemy_{i:02d}.png').convert('RGBA') for i in range(9)]
# Deliberately varied silhouettes across floors. Never overwrite the original nine.
parent=[0,1,2,3,4,5,6,7,8,0,1,2,3,5,1,0,4,3,2,5,1,6,6,7,7,2,1,2,5,4,8,3,4,6,7,0]
palette=[(248,94,52),(131,204,251),(187,87,226),(225,184,75),(156,215,103),(241,106,120),
 (89,217,224),(228,78,98),(255,154,56),(87,202,221),(123,186,255),(202,137,245),
 (236,112,78),(237,133,52),(211,217,236),(255,93,33),(78,230,213),(176,114,245),
 (130,211,93),(255,101,72),(143,178,241),(214,182,252),(82,186,239),(131,183,255),
 (244,66,97),(144,106,220),(92,234,240),(190,117,255),(255,126,116),(218,215,172),
 (122,149,251),(224,135,255),(142,223,233),(255,109,78),(147,192,255),(244,68,139)]
imgs=[]
for idx in range(36):
 im=source[idx] if idx<9 else source[parent[idx]].copy()
 if idx>=9:
  if idx%3==0: im=im.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
  rgb=np.asarray(im.convert('RGB')).astype(np.float32)
  alpha=np.asarray(im.getchannel('A')).copy()
  # Original brightness retained in the armor, while magic/cape highlights take a
  # different enemy-specific hue. This is an inexpensive offline color grade.
  accent=np.asarray(palette[idx],dtype=np.float32)
  luminance=(rgb*.2126).sum(axis=2)/255.0
  sat=np.max(rgb,axis=2)-np.min(rgb,axis=2)
  mask=(sat>14).astype(np.float32)[:,:,None]
  accent_rgb=(accent[None,None,:]*np.clip(.28+luminance[:,:,None]*.78,0,1))
  mixed=rgb*(1-mask*.55)+accent_rgb*(mask*.55)
  if idx%4==0: mixed=mixed*.84
  if idx%4==1: mixed=mixed*1.08
  rgba=np.dstack([np.uint8(np.clip(mixed,0,255)),alpha])
  im=Image.fromarray(rgba,'RGBA')
 im.save(art/f'enemy_{idx:02d}.png',optimize=True)
 imgs.append(im)
body=['#include "enemy_art.h"']
head=['#ifndef RIFT_ENEMY_ART_H','#define RIFT_ENEMY_ART_H','#include <stdint.h>','#define RIFT_ENEMY_W 104','#define RIFT_ENEMY_H 110','#define RIFT_ENEMY_COUNT 36','extern const uint16_t *const rift_enemy_sprites[RIFT_ENEMY_COUNT];']
for idx,im in enumerate(imgs):
 rgba=np.array(im,dtype=np.uint16)
 rgb=rgba[:,:,:3]
 pix=((rgb[:,:,0]>>3)<<11)|((rgb[:,:,1]>>2)<<5)|(rgb[:,:,2]>>3)
 pix=np.where(rgba[:,:,3]>110,pix,0xf81f).astype(np.uint16)
 pix[(pix==0xf81f)&(rgba[:,:,3]>110)]=0xf01e
 flat=pix.ravel()
 body.append(f'const uint16_t rift_enemy_{idx}[RIFT_ENEMY_W*RIFT_ENEMY_H] = {{')
 for j in range(0,len(flat),32): body.append(','.join(f'0x{int(v):04x}' for v in flat[j:j+32])+',')
 body.append('};')
 head.append(f'extern const uint16_t rift_enemy_{idx}[RIFT_ENEMY_W*RIFT_ENEMY_H];')
body.append('const uint16_t *const rift_enemy_sprites[RIFT_ENEMY_COUNT] = {'+', '.join(f'rift_enemy_{i}' for i in range(36))+'};')
head.append('#endif')
(root/'src/enemy_art.c').write_text('\n'.join(body)+'\n')
(root/'src/enemy_art.h').write_text('\n'.join(head)+'\n')
sheet=Image.new('RGB',(6*128,6*144),(16,22,35));draw=ImageDraw.Draw(sheet)
for i,im in enumerate(imgs):
 x,y=(i%6)*128,(i//6)*144
 # dark drop shadow behind alpha image
 sheet.paste(im,(x+12,y+8),im)
 draw.text((x+9,y+120),f'{i:02d}',fill=(235,210,177))
sheet.save(root/'screenshots/enemy_roster_36.png',optimize=True)
print('Created 36 RGB565 enemy images: ',36*104*110*2,'bytes raw')
