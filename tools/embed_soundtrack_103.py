#!/usr/bin/env python3
"""Single-EBOOT Rift Ascent 1.0.3 soundtrack packer.

Order: Checkers (title), RiftMenu (navigation/shop), 8 battle tracks.
Format: 11025 Hz mono signed 4-bit IMA ADPCM; 96-byte indexed footer.
This intentionally favors file size and PSP RAM/CPU over fidelity.
"""
from pathlib import Path
import argparse, hashlib, struct, sys, zlib, importlib.util

MAGIC=b"RIFTM103"
NAMES=["Checkers (1).wav","RiftMenu.wav",
       "BARBARICE (1).wav","BREAKER (1).wav","Cof (1).wav",
       "Devour (1).wav","DIRE (1).wav","Drudge (1).wav",
       "OVERT (1).wav","STAMMER (1).wav"]
spec=importlib.util.spec_from_file_location("adpcm",Path(__file__).with_name("embed_churn.py"))
adpcm=importlib.util.module_from_spec(spec)
spec.loader.exec_module(adpcm)

def pack(base,music,output):
    pbp=base.read_bytes()
    adpcm.inspect(pbp)
    if pbp[-20:-12]==b"RIFTIMA1" or pbp[-96:-88]==MAGIC:
        raise ValueError("Input EBOOT already includes embedded music")
    track_bytes=[]
    entries=[]
    length=0
    for name in NAMES:
        samples=adpcm.decode_wav(music/name)
        compressed=adpcm.encode(samples)
        track_bytes.append(compressed)
        entries.append((length,len(samples)))
        length+=len(compressed)
        print(name,"samples",len(samples),"bytes",len(compressed))
    payload=b"".join(track_bytes)
    footer=MAGIC+b"".join(struct.pack("<II",*e) for e in entries)+struct.pack(
        "<II",len(payload),zlib.crc32(payload))
    assert len(footer)==96
    output.parent.mkdir(parents=True,exist_ok=True)
    output.write_bytes(pbp+payload+footer)
    verify(output)
    return output

def verify(path):
    data=path.read_bytes()
    adpcm.inspect(data)
    if data[-96:-88]!=MAGIC:raise ValueError("Music index footer missing")
    starts=[struct.unpack_from("<II",data,len(data)-88+i*8) for i in range(10)]
    length,crc=struct.unpack_from("<II",data,len(data)-8)
    if len(data)<96+length or not length:raise ValueError("Bad music length")
    payload=data[-96-length:-96]
    if zlib.crc32(payload)!=crc:raise ValueError("Music CRC mismatch")
    for i,(offset,count) in enumerate(starts):
        if not count or offset+(count+1)//2>length:raise ValueError("Bad track "+str(i))
    print("PASS: ten indexed tracks, CRC32, standalone PSP PBP")
    print("Music bytes:",length,"/ EBOOT bytes:",len(data))
    print("SHA256:",hashlib.sha256(data).hexdigest())
    return length

def main():
    cli=argparse.ArgumentParser()
    cli.add_argument("--eboot",type=Path)
    cli.add_argument("--music",type=Path)
    cli.add_argument("--out",type=Path)
    cli.add_argument("--check",type=Path)
    cli.add_argument("--self-test",action="store_true")
    args=cli.parse_args()
    if args.self_test:
        from math import sin,pi
        for n in (13,1000,11025):
            samples=[int(28000*sin(2*pi*i/97)) for i in range(n)]
            enc=adpcm.encode(samples)
            assert len(enc)==(n+1)//2
            assert len(adpcm.decode(enc,n))==n
        print("PASS: multi-track ADPCM codec self-tests")
    elif args.check:verify(args.check)
    else:
        if not(args.eboot and args.music and args.out):cli.error("Provide --eboot --music --out")
        pack(args.eboot,args.music,args.out)

if __name__=="__main__":main()
