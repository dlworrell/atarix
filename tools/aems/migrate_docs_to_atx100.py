#!/usr/bin/env python3
from pathlib import Path
import argparse, re, shutil, sys

ROOT=Path(__file__).resolve().parents[2]
DOCS=ROOT/'docs'
CHDIR=DOCS/'architecture'/'ATX-100'/'chapters'
ARCH=DOCS/'archive'/'atx100-migrated'
BOOK=DOCS/'architecture'/'ATX-100'/'ATX-100-COMPILED.md'
PLAN=ROOT/'build'/'ATX-100-MIGRATION-PLAN.md'

CHAPTERS=[
(1,'Purpose and Scope',['purpose','scope','design-001','design-002']),
(2,'Historical Context',['historical','history','context']),
(3,'Documentation Taxonomy',['taxonomy','style-guide','documentation']),
(4,'Architecture Before Implementation',['architecture-before','architecture review','architecture-review']),
(5,'Security Before Convenience',['security','authority','policy']),
(6,'Human Readability',['readability','human']),
(7,'Fabric-Centric Computing',['fabric-centric','fabric']),
(8,'Engineering as Architecture',['engineering-as','engineering']),
(9,'System Layers',['system-layers','layers','spec-001','spec-002','spec-003','spec-005']),
(10,'Object Model',['object model','object-model','object']),
(11,'Capability Model',['capability','capabilities']),
(12,'Mailbox Transport',['mailbox','transport']),
(13,'Directory and Discovery Services',['directory','discovery']),
(14,'Memory and Data Movement',['memory','data-movement','spec-021']),
(15,'Lookup Acceleration',['lookup','acceleration','hash','index','spec-020']),
(16,'Processor Modules',['processor','module']),
(17,'Fabric Northbridge',['northbridge']),
(18,'Supervisor Controller',['supervisor','controller']),
(19,'Storage and Networking',['storage','network']),
(20,'AEMS and Engineering Gates',['aems','gate','gates','spec-090','spec-091']),
(21,'Requirements and Traceability',['requirements','traceability','trace']),
(22,'Roadmap and Evolution',['roadmap','evolution'])]

def strip_meta(s):
    if s.startswith('---\n'):
        i=s.find('\n---',4)
        if i!=-1: return s[i+4:].strip()
    return s.strip()

def title(n,t): return f'Chapter {n}: {t}'
def chfile(n,t): return CHDIR/(f'{n:02d}-'+t.lower().replace(' and ','-and-').replace(' ','-')+'.md')
def source_files():
    skip={'archive','ATX-100'}
    out=[]
    for p in sorted(DOCS.rglob('*.md')):
        r=p.relative_to(DOCS).parts
        if not r or r[0] in skip: continue
        out.append(p)
    return out

def score(p,body,keys):
    h=(p.as_posix()+'\n'+body[:6000]).lower()
    return sum(1 for k in keys if k.lower() in h)

def bucket(p,body):
    best=(0,None)
    for n,t,keys in CHAPTERS:
        v=score(p,body,keys)
        if v>best[0]: best=(v,(n,t))
    return best[1] or (22,'Roadmap and Evolution')

def block(src,body):
    rel=src.relative_to(ROOT)
    return '\n'.join(['','<!-- AEMS-MIGRATED-SOURCE: '+str(rel)+' -->','### Integrated source: `'+str(rel)+'`','',body,''])

def build(apply=False):
    files=source_files(); groups={}; plan=['# ATX-100 Migration Plan','']
    for p in files:
        body=strip_meta(p.read_text(encoding='utf-8'))
        n,t=bucket(p,body); groups.setdefault((n,t),[]).append((p,body))
    for n,t,_ in CHAPTERS:
        key=(n,t); plan.append(f'## {title(n,t)}')
        for p,_ in groups.get(key,[]): plan.append(f'- {p.relative_to(ROOT)}')
        plan.append('')
    PLAN.parent.mkdir(parents=True,exist_ok=True); PLAN.write_text('\n'.join(plan),encoding='utf-8')
    if not apply:
        print(f'PASS wrote migration plan: {PLAN.relative_to(ROOT)}')
        print(f'AEMS MIGRATION RESULT: DRY-RUN files={len(files)}')
        return 0
    CHDIR.mkdir(parents=True,exist_ok=True); ARCH.mkdir(parents=True,exist_ok=True)
    book=['# ATX-100: Compiled Atarix Architecture Book','']
    moved=0
    for n,t,_ in CHAPTERS:
        path=chfile(n,t)
        existing=path.read_text(encoding='utf-8') if path.exists() else f'# {title(n,t)}\n'
        additions=''.join(block(p,b) for p,b in groups.get((n,t),[]))
        path.write_text(existing.rstrip()+'\n\n## AEMS Integrated Material\n'+additions,encoding='utf-8')
        book.append(path.read_text(encoding='utf-8'))
        book.append('\n---\n')
        for p,_ in groups.get((n,t),[]):
            dest=ARCH/p.relative_to(DOCS); dest.parent.mkdir(parents=True,exist_ok=True); shutil.move(str(p),str(dest)); moved+=1
    BOOK.parent.mkdir(parents=True,exist_ok=True); BOOK.write_text('\n'.join(book),encoding='utf-8')
    print(f'PASS wrote compiled book: {BOOK.relative_to(ROOT)}')
    print(f'AEMS MIGRATION RESULT: APPLIED moved={moved}')
    return 0

def main(argv):
    ap=argparse.ArgumentParser(); ap.add_argument('--apply',action='store_true')
    return build(ap.parse_args(argv).apply)
if __name__=='__main__': raise SystemExit(main(sys.argv[1:]))
