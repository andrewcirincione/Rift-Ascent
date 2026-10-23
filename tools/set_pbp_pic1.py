#!/usr/bin/env python3
"""Replace XMB/PBP PIC1.PNG safely without editing ELF or PSP startup.

PBP offsets: SFO, ICON0, ICON1, PIC0, PIC1, SND0, DATA.PSP, DATA.PSAR.
Call before the 1.0.3 soundtrack packer so offsets continue to remain valid.
"""
import argparse,struct
from pathlib import Path
def main():
 p=argparse.ArgumentParser()
 p.add_argument("--eboot",type=Path,required=True)
 p.add_argument("--pic1",type=Path,required=True)
 p.add_argument("--out",type=Path,required=True)
 a=p.parse_args()
 original=a.eboot.read_bytes()
 art=a.pic1.read_bytes()
 if original[:4]!=b"\x00PBP":raise ValueError("Not EBOOT.PBP")
 if art[:8]!=b"\x89PNG\r\n\x1a\n":raise ValueError("Not PNG image")
 old=list(struct.unpack_from("<8I",original,8))
 if sorted(old)!=old or not(40<=old[4]<old[5]<=len(original)):
     raise ValueError("Corrupted PBP PIC1 offsets")
 replacement=bytearray(original[:old[4]]+art+original[old[5]:])
 delta=len(art)-(old[5]-old[4])
 for i in range(5,8):struct.pack_into("<I",replacement,8+4*i,old[i]+delta)
 new=list(struct.unpack_from("<8I",replacement,8))
 if not (new[4]==old[4] and new[5]==new[4]+len(art)
         and new[6]-new[5]==old[6]-old[5]):
     raise ValueError("PBP header invalid after PNG replacement")
 a.out.parent.mkdir(parents=True,exist_ok=True)
 a.out.write_bytes(replacement)
 print("PASS: XMB background replaced",len(art),"bytes; PBP",len(replacement),"bytes")
if __name__=="__main__":main()
