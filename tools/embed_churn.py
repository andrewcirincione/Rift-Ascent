#!/usr/bin/env python3
"""Embed a strongly compressed CHURN.wav at the end of a PSP EBOOT.PBP.

Output has one file only: EBOOT.PBP. PSP loads the standard executable section
and the game streams the trailer using sceIoLseek32 and sceIoRead.

Encoding: continuous IMA ADPCM, mono, 11,025 Hz, 4 bits/sample.
Original 44.1k stereo floating WAV -> approximately 1/64 source size.
Footer: b'RIFTIMA1' + <uint32 payload_bytes, uint32 samples, uint32 crc32>.
"""
import argparse
import array
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
import zlib

MAGIC = b"RIFTIMA1"
STEPS = (
 7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,
 50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,
 230,253,279,307,337,371,408,449,494,544,598,658,724,796,
 876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,
 2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,
 8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,
 22385,24623,27086,29794,32767
)
INDEX = (-1,-1,-1,-1,2,4,6,8,-1,-1,-1,-1,2,4,6,8)

def clamp(x, lo, hi):
    return min(hi, max(lo, x))

def decode_nibble(nib, predictor, index):
    step = STEPS[index]
    delta = step >> 3
    if nib & 1: delta += step >> 2
    if nib & 2: delta += step >> 1
    if nib & 4: delta += step
    predictor = clamp(predictor - delta if nib & 8 else predictor + delta, -32768, 32767)
    index = clamp(index + INDEX[nib], 0, 88)
    return predictor, index

def encode(samples):
    predictor = index = 0
    out = bytearray()
    prev = 0
    for sample in samples:
        diff = int(sample) - predictor
        sign = 8 if diff < 0 else 0
        d = abs(diff)
        step = STEPS[index]
        bits = 0
        if d >= step:
            bits |= 4
            d -= step
        if d >= step >> 1:
            bits |= 2
            d -= step >> 1
        if d >= step >> 2:
            bits |= 1
        nibble = sign | bits
        predictor, index = decode_nibble(nibble, predictor, index)
        if prev & 0x100:
            out.append((prev & 15) | (nibble << 4))
            prev = 0
        else:
            prev = 0x100 | nibble
    if prev & 0x100:
        out.append(prev & 15)
    return bytes(out)

def decode(payload, count):
    predictor = index = 0
    out = []
    for b in payload:
        for nibble in (b & 15, b >> 4):
            if len(out) == count:
                return out
            predictor, index = decode_nibble(nibble, predictor, index)
            out.append(predictor)
    return out

def decode_wav(path):
    process = subprocess.run(
      ["ffmpeg","-nostdin","-hide_banner","-loglevel","error","-i",str(path),
       "-ar","11025","-ac","1","-acodec","pcm_s16le","-f","s16le","-"],
      check=True, capture_output=True)
    vals = array.array("h")
    vals.frombytes(process.stdout)
    if sys.byteorder != "little": vals.byteswap()
    return vals

def inspect(data):
    if len(data) < 40 or data[:4] != b"\x00PBP":
        raise ValueError("Not a PSP EBOOT.PBP")
    offsets = struct.unpack_from("<8I",data,8)
    if tuple(sorted(offsets)) != offsets:
        raise ValueError("PBP offsets not monotonic")
    return offsets

def footer_of(data):
    if data[-20:-12] != MAGIC:
        raise ValueError("Embedded CHURN footer not found")
    nbytes, count, crc = struct.unpack_from("<III",data,len(data)-12)
    if nbytes != (count+1)//2 or nbytes > len(data)-60 or count == 0:
        raise ValueError("Invalid embedded sound length")
    encoded = data[-20-nbytes:-20]
    if zlib.crc32(encoded) != crc:
        raise ValueError("Embedded CHURN checksum failure")
    return nbytes,count,crc

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--wave",type=Path,help="Original CHURN.wav")
    p.add_argument("--eboot",type=Path,help="Compiled, *unpatched* EBOOT.PBP")
    p.add_argument("--out",type=Path,help="Output EBOOT.PBP")
    p.add_argument("--check",type=Path,help="Verify previously packaged EBOOT")
    p.add_argument("--self-test",action="store_true")
    args=p.parse_args()
    if args.self_test:
        samples = [int(26000*((i%57)/56*2-1)) for i in range(9831)]
        data=encode(samples)
        decoded=decode(data,len(samples))
        assert len(data)==(len(samples)+1)//2
        assert len(decoded)==len(samples)
        assert max(abs(x) for x in decoded)>20000
        print("PASS: IMA nibble encoding/decoding, odd sample count")
        return
    if args.check:
        data=args.check.read_bytes()
        inspect(data)
        n,c,crc=footer_of(data)
        print("PASS: embedded CHURN",n,"bytes",c,"samples",
              round(c/11025,2),"seconds; EBOOT SHA256",hashlib.sha256(data).hexdigest())
        return
    if not (args.wave and args.eboot and args.out):
        p.error("--wave, --eboot and --out required together")
    original=args.eboot.read_bytes()
    inspect(original)
    if original[-20:-12]==MAGIC:
        raise ValueError("Input already contains CHURN; start with unpatched EBOOT")
    samples=decode_wav(args.wave)
    compressed=encode(samples)
    footer=MAGIC+struct.pack("<III",len(compressed),len(samples),zlib.crc32(compressed))
    args.out.parent.mkdir(parents=True,exist_ok=True)
    args.out.write_bytes(original+compressed+footer)
    final=args.out.read_bytes()
    assert inspect(final)==inspect(original)
    assert footer_of(final)[0]==len(compressed)
    assert len(final)==len(original)+len(compressed)+len(footer)
    print("Packaged:",args.out,"bytes:",len(final),
          "CHURN:",len(compressed),"bytes",len(samples)/11025,"sec",
          "SHA256:",hashlib.sha256(final).hexdigest())

if __name__=="__main__":
    main()
