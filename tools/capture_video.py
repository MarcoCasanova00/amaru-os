#!/usr/bin/env python3
"""Build the host recorder, then capture the English film's chapters."""
import json,os,subprocess
from pathlib import Path
R=Path(__file__).resolve().parents[1];F=R/'firmware'
W=Path(os.environ.get('BUNKER_VIDEO_CAPTURE',str(F/'build/video')))
(F/'build/host').mkdir(parents=True,exist_ok=True)
subprocess.run(['cc','-O1','-w','-ffp-contract=off','-Ibuild/gen','-Ifirmware/src','-o','build/host/bunker_video','tests/bunker_video.c','-lm'],cwd=F,check=True)
for x in json.loads((R/'docs/video/script.json').read_text()):
    out=W/x['name'];out.mkdir(parents=True,exist_ok=True)
    subprocess.run([str(F/'build/host/bunker_video'),str(x['index']),str(round(x['duration']*10)),str(out)],cwd=F,check=True)
