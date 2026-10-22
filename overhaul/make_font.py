#!/usr/bin/env python3
"""Create compact 7x11 antialiased printable ASCII glyphs for PSP RGB8888 renderer.
Build-time dependency only: Pillow, DejaVu Sans Condensed font.
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
out=Path(sys.argv[1])
candidates=[
 "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf",
 "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
]
path=next((p for p in candidates if Path(p).exists()),None)
if not path: raise RuntimeError("No DejaVu font; CI must install fonts-dejavu-core")
S=4
font=ImageFont.truetype(path,39)
rows=[]
for code in range(32,127):
    glyph=chr(code)
    board=Image.new("L",(7*S,11*S),0)
    draw=ImageDraw.Draw(board)
    bounds=draw.textbbox((0,0),glyph,font=font)
    ink_w=bounds[2]-bounds[0]
    ink_h=bounds[3]-bounds[1]
    x=(7*S-ink_w)//2-bounds[0]
    y=(11*S-ink_h)//2-bounds[1]
    # Move descenders a pixel lower and baseline characters a little lower.
    if glyph in "gjpqy":y+=2
    draw.text((x,y),glyph,fill=255,font=font,stroke_width=0)
    reduced=board.resize((7,11),Image.Resampling.LANCZOS)
    # Preserve high-contrast stems on the tiny physical LCD while retaining
    # 2-3 opacity levels along the glyph contours.
    levels=[]
    for value in reduced.getdata():
        if value<38:levels.append(0)
        elif value>170:levels.append(255)
        else:levels.append(min(255,50+(value-38)*205//132))
    rows.append(levels)
# All intensity values are composited, not turned back into hard on/off bits.
lines=[
 "#ifndef RIFT_SMOOTH_FONT_H",
 "#define RIFT_SMOOTH_FONT_H",
 "#include <stdint.h>",
 "static const uint8_t rift_smooth_font[95][77]={"
]
for code,row in enumerate(rows):
    chunks=[",".join(str(v) for v in row[i:i+11]) for i in range(0,77,11)]
    lines.append(" /* %d */ {%s},"%(code+32,",".join(chunks)))
lines+=["};","#endif"]
out.write_text("\n".join(lines)+"\n")
print("Wrote",out,out.stat().st_size,"bytes")
