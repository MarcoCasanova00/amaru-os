#!/usr/bin/env python3
"""Generate editor metadata from this release's actual C source; never flash or rebuild firmware."""
import json,re,subprocess
from pathlib import Path
R=Path(__file__).resolve().parents[1]; F=R/'firmware'
subprocess.run(['cc','-O1','-w','-Ibuild/gen','-Ifirmware/src','-o','build/host/descdump','tests/descdump.c','-lm'],cwd=F,check=True)
x=json.loads(subprocess.check_output(['build/host/descdump'],cwd=F))
s=(F/'firmware/src/seq.c').read_text(); body=s.split('SCALE_MASK[] = {')[1].split('};')[0];body=re.sub(r'/\*.*?\*/','',body,flags=re.S)
masks=[eval(a.strip(),{'__builtins__':{}},{}) for a in body.split(',') if a.strip()]
names=re.findall(r'"([^"]+)"',(F/'firmware/src/bunker_grow.c').read_text().split('B_SCALE_LONG[] = {')[1].split('};')[0])
assert len(masks)==len(names)==32 and x['P_COUNT']==114
x['scales']=[{'name':a,'mask':b} for a,b in zip(names,masks)]
x['library']=json.loads((R/'dist/library.json').read_text())
(R/'dist/editor/data.json').write_text(json.dumps(x,separators=(',',':')))
print('Editor metadata: 114 parameters, 32 scales, real C engine tables')
