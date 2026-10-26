#!/usr/bin/env python3
import subprocess,json,concurrent.futures
from pathlib import Path
root=Path(__file__).resolve().parents[1]
(root.parent/'dist/audio').mkdir(parents=True,exist_ok=True)
def run(args):
 r=subprocess.run(['build/host/bunker_runtime']+args,cwd=root,capture_output=True,text=True,timeout=90)
 if r.returncode:raise RuntimeError(str(args)+r.stdout+r.stderr)
 return json.loads(r.stdout.strip().splitlines()[-1])
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
 sounds=list(pool.map(run,[['sound',str(k),str(p)] for k in range(4) for p in range(16)]))
 assert len({x['hash'] for x in sounds})==64
 (root/'build/bunker-sounds.json').write_text(json.dumps(sounds,indent=2))
 assert min(x['peak'] for x in sounds)>50, [(i//16,i%16,x) for i,x in enumerate(sounds) if x['peak']<=50]
 worlds=list(pool.map(run,[['world',str(w),'16','../dist/audio/world-%02d.wav'%w] for w in range(24)]))
 assert len({x['hash'] for x in worlds})==24
 assert all(x['maxVoices']<=8 and x['peak']<32768 for x in worlds)
report=dict(sounds=sounds,worlds=worlds)
(root/'build/bunker-audio-bank.json').write_text(json.dumps(report,indent=2))
print('64/64 sound fingerprints unique and audible; 24/24 worlds unique, bounded; maximum voices',max(x['maxVoices'] for x in worlds));print('World peak range',min(x['peak'] for x in worlds),max(x['peak'] for x in worlds))
