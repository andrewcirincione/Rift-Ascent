#!/usr/bin/env python3
"""Rift 1.1.0 soundtrack packer: normalize perceptual loudness before ADPCM.

Same single-EBOOT 10-track indexed format as 1.0.3 and same 11,025 Hz
mono IMA compression. Use FFmpeg loudnorm to -14 LUFS, -1dB true peak.
"""
from pathlib import Path
import argparse,array,importlib.util,subprocess,sys
folder=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location("soundtrack103",folder/"embed_soundtrack_103.py")
pack103=importlib.util.module_from_spec(spec)
spec.loader.exec_module(pack103)

def loud_wav(path):
    p=subprocess.run([
        "ffmpeg","-nostdin","-hide_banner","-loglevel","error",
        "-i",str(path),"-af","loudnorm=I=-14:TP=-1.0:LRA=9",
        "-ac","1","-ar","11025","-acodec","pcm_s16le","-f","s16le","-"
    ],capture_output=True,check=True)
    output=array.array("h");output.frombytes(p.stdout)
    if sys.byteorder!="little":output.byteswap()
    if not output:raise ValueError("Empty soundtrack after normalization: "+str(path))
    return output

def main():
    a=argparse.ArgumentParser()
    a.add_argument("--eboot",type=Path,required=True)
    a.add_argument("--music",type=Path,required=True)
    a.add_argument("--out",type=Path,required=True)
    args=a.parse_args()
    original=pack103.adpcm.decode_wav
    pack103.adpcm.decode_wav=loud_wav
    try:pack103.pack(args.eboot,args.music,args.out)
    finally:pack103.adpcm.decode_wav=original

if __name__=="__main__":main()
