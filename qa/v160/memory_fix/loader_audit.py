"""Audit PSP allocation bounds and the CFW static-ELF fallback regression.

References: uofw/uofw src/kd/loadcore/loadelf.c (sceKernelCheckExecFile),
src/kd/modulemgr/modulemgr.c (execSize fallback/maxAllocSize), and
PSP-Archive/ARK-4 core/systemctrl/src/loadercore.c (_ProbeExec2).
This models those checks; it is not a physical-console execution test.
"""
from pathlib import Path
import struct,json,sys
def inspect(path):
 data=Path(path).read_bytes();o=struct.unpack_from('<8I',data,8);e=data[o[6]:o[7]]
 ph,n=struct.unpack_from('<I',e,28)[0],struct.unpack_from('<H',e,44)[0]
 loads=[struct.unpack_from('<8I',e,ph+32*i) for i in range(n)];loads=[x for x in loads if x[0]==1]
 first=loads[0];low=min(x[2] for x in loads)&~255;high=max(x[2]+x[5] for x in loads)
 # Raw static ELFs with an absent module-info locator depend on CFW repair;
 # the firmware's unset-execSize fallback reads to EOF, including PSAR.
 fallback=first[3]==first[2]
 size=len(data)-o[6] if fallback else max(x[1]+x[4] for x in loads)
 alloc=(max(high-low,size)+255)&~255
 return dict(path=str(path),module_locator=first[3],cfw_eof_fallback=fallback,data_psp_bytes=len(e),resident_span=high-low,modeled_allocation=alloc,allocation_end=low+alloc,available_partition_end=0x0a000000,within_32mb_user_partition=low+alloc<=0x0a000000,conservative_peak=len(e)+high-low)
old,new=map(inspect,sys.argv[1:3]);assert old['cfw_eof_fallback'] and not old['within_32mb_user_partition']
assert not new['cfw_eof_fallback'] and new['within_32mb_user_partition'] and new['conservative_peak']<=20*1024*1024
print(json.dumps(dict(original=old,corrected=new),indent=2))
print('PASS original allocation regression reproduced by loader model; corrected metadata and memory budgets pass (physical PSP confirmation pending)')
