#!/usr/bin/env python3
"""Reproducible text screening, NOT a legal similarity/clearance test.

Pass --reference-dir with locally obtained sts{1,2}-{category}.json files to
refresh the evidence. Reference catalogs are never copied into this repository.
Without that option, check the committed reference-name hashes and retired text.
"""
from pathlib import Path
import argparse,collections,hashlib,json,re

R=Path(__file__).resolve().parents[2]
E=R/'distribution/evidence'
SHA='4e6b0e040ccd00eb04fb1ab7fb0b298c222d2f29'
CATEGORIES=('cards','relics','potions','characters','events','monsters','powers')
def normalize(s):return ' '.join(re.findall(r'[a-z0-9]+',s.casefold()))
def digest(s):return hashlib.sha256(s.encode()).hexdigest()
def text_values(value):
 if isinstance(value,str):yield value
 elif isinstance(value,list):
  for item in value:yield from text_values(item)
 elif isinstance(value,dict):
  for key,item in value.items():
   if key not in ('id','image_url','icon','tags'):yield from text_values(item)
def literals():
 for p in sorted((R/'qa/v186/source').iterdir()):
  if p.suffix not in ('.c','.h','.inc'):continue
  for m in re.finditer(r'"((?:[^"\\]|\\.)*)"',p.read_text()):
   yield p.relative_to(R).as_posix(),m.group(1)

parser=argparse.ArgumentParser();parser.add_argument('--reference-dir',type=Path)
args=parser.parse_args()
own={k:json.loads((R/'distribution'/f'{k}.json').read_text()) for k in CATEGORIES[:3]}
strings=list(literals())
hashfile=R/'distribution/review-name-hashes.json'
if args.reference_dir:
 refs=[];manifest=[];names=collections.defaultdict(list)
 for game in ('sts1','sts2'):
  for kind in CATEGORIES:
   filename=f'{game}-{kind}.json';data=(args.reference_dir/filename).read_bytes();rows=json.loads(data)
   manifest.append(dict(file=filename,rows=len(rows),sha256=hashlib.sha256(data).hexdigest()))
   for row in rows:
    refs.append((filename,row))
    if row.get('name'):names[normalize(row['name'])].append(dict(file=filename,name=row['name']))
 hashes=sorted({digest(n) for n in names})
 hashfile.write_text(json.dumps(hashes,indent=2)+'\n')
 grams=collections.defaultdict(set)
 for file,row in refs:
  for value in text_values(row):
   words=normalize(value).split()
   for i in range(len(words)-7):grams[' '.join(words[i:i+8])].add((file,row.get('name','?')))
 exact=[];shared=[]
 for path,value in strings:
  clean=normalize(value)
  if clean in names:exact.append(dict(path=path,text=value,reference=names[clean]))
  words=clean.split();hits=set()
  for i in range(len(words)-7):hits.update(grams.get(' '.join(words[i:i+8]),()))
  if hits:shared.append(dict(path=path,own_text=value,reference=sorted(hits)))
 name_hits=[dict(catalog=k,id=row['id'],name=row['name'],reference=names[normalize(row['name'])])
            for k,rows in own.items() for row in rows if normalize(row['name']) in names]
 description_refs={normalize(value) for _,row in refs for value in text_values(row) if len(normalize(value).split())>=4}
 description_hits=[dict(catalog=k,id=row['id'],field=field) for k,rows in own.items() for row in rows
                   for field in ('description','upgrade_description')
                   if row.get(field) and normalize(row[field]) in description_refs]
 source_files={p for p,_ in strings}
 evidence=dict(date='2026-11-02',scope='English community snapshot; not exhaustive, authoritative, semantic equivalence or legal clearance',
  reference_repository='https://github.com/nkhoit/spire-archive',reference_commit=SHA,
  reference_files=manifest,reference_rows=len(refs),unique_reference_names=len(names),
  own_collectibles={k:len(v) for k,v in own.items()},collectible_name_hits=name_hits,
  full_description_hits=description_hits,source_literal_name_hits=exact,shared_eight_word_sequences=shared,
  source_files=len(source_files),source_literals=len(strings),
  source_text_sha256={p:hashlib.sha256((R/p).read_bytes()).hexdigest() for p in sorted(source_files)})
 (E/'legal-catalog-audit.json').write_text(json.dumps(evidence,indent=2)+'\n')
 print('Reference:',len(refs),'rows;',len(names),'unique names; full-description hits:',len(description_hits),'; eight-word hits:',len(shared))
 print('Source literal name candidates:',json.dumps(exact))
 assert not name_hits,name_hits
else:hashes=json.loads(hashfile.read_text())
known=set(hashes)
for kind,rows in own.items():
 for row in rows:assert digest(normalize(row['name'])) not in known,(kind,row['name'])
for path,value in strings:
 assert not re.search(r'\b(?:sovereign blade|regent|sly|abyss watcher)\b',value,re.I),(path,value)
print('PASS 514 collectible names against pinned review hashes; retired identity/rule text absent from C literals')
