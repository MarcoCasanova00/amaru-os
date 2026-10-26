#!/usr/bin/env python3
"""Package the released, unchanged firmware with current publication records."""
from pathlib import Path
import hashlib, shutil, subprocess, sys, zipfile
R=Path(__file__).resolve().parents[1]
D=R/'dist'; M=D/'media'
subprocess.run([sys.executable,str(R/'tools/package_source.py')],check=True)
files={
 'bunker-evolved-rc2.fwsc':D/'install/bunker-evolved-rc2.fwsc',
 'Bunker-Evolved-Source.zip':D/'Bunker-Evolved-Source.zip',
 'Bunker-Evolved-Introduction.mp4':M/'Bunker-Evolved-Introduction.mp4',
 'Bunker-Evolved-English.srt':M/'Bunker-Evolved-English.srt',
 'Bunker-Evolved-English.vtt':M/'Bunker-Evolved-English.vtt',
 'thumbnail.jpg':M/'thumbnail.jpg',
 'Production-Notes.txt':M/'Production-Notes.txt',
 'YouTube-Description.txt':M/'YouTube-Description.txt',
 'YouTube-Title.txt':M/'YouTube-Title.txt',
 'Chapters.txt':M/'Chapters.txt',
 'Publication-Readme.txt':M/'Publication-Readme.txt',
 'GitHub-Release-Notes.md':R/'docs/release/GitHub-Release-Notes.md',
 'hardware-acceptance.md':R/'docs/release/hardware-acceptance.md',
 'test-status.txt':R/'docs/release/test-status.txt',
 'feature-provenance.md':R/'docs/release/feature-provenance.md',
 'editor-architecture.md':R/'docs/release/editor-architecture.md',
 'editor-test.log':R/'docs/release/editor-test.log',
 'editor-ui-test.log':R/'docs/release/editor-ui-test.log',
}
assert hashlib.sha256(files['bunker-evolved-rc2.fwsc'].read_bytes()).hexdigest()=='db0b50d0ae9c3bfccee76e3b93de09fffa55389169278a0c5899dad9c79e6bb8'
sums=''.join(hashlib.sha256(p.read_bytes()).hexdigest()+'  '+name+'\n' for name,p in files.items())
(M/'SHA256SUMS.txt').write_text(sums)
files['SHA256SUMS.txt']=M/'SHA256SUMS.txt'
with zipfile.ZipFile(D/'Bunker-Evolved-Publication-Kit.zip','w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
 for name,p in files.items():z.write(p,name)
if len(sys.argv)>1:
 dest=Path(sys.argv[1]);dest.mkdir(parents=True,exist_ok=True)
 for name,p in files.items():shutil.copy2(p,dest/name)
 shutil.copy2(D/'Bunker-Evolved-Publication-Kit.zip',dest/'Bunker-Evolved-Publication-Kit.zip')
print('Publication kit ready; firmware SHA-256 unchanged.')
