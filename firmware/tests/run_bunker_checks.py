#!/usr/bin/env python3
"""Focused release checks. Original upstream suite retained separately.
All native UI, audio, storage and protocol paths compile against the fork.
No audio golden file or timing budget is rewritten by this runner.
"""
from pathlib import Path
import subprocess,concurrent.futures,json,os
root=Path(__file__).resolve().parents[1];os.chdir(root)
out=Path('build/bunker-checks');out.mkdir(parents=True,exist_ok=True)
names=['storage_test','upreset_test','input_test','midi_uart_test','scale_test','project_test','motion_test','detail_test','ui155_test','v156_test','fun10_test','midi_control_test','audio_test','persistence_test','backup_test','editor_test','robust_test','fm6bank_test','ui_test']
def run(name):
 log=out/(name+'.log')
 with log.open('w') as f:
  c=subprocess.run(['cc','-O1','-w','-Ibuild/gen','-Ifirmware/src','-o',str(out/name),'tests/'+name+'.c','-lm'],stdout=f,stderr=f)
  if c.returncode:return dict(test=name,compile=c.returncode,exit=None)
  c=subprocess.run([str(out/name)],stdout=f,stderr=f,timeout=600)
 return dict(test=name,compile=0,exit=c.returncode)
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:
 results=[]
 for r in pool.map(run,names):results.append(r);print(r,flush=True)
(out/'results.json').write_text(json.dumps(results,indent=2))
raise SystemExit(any(r['compile'] or r['exit'] for r in results))
