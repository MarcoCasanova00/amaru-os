#!/usr/bin/env python3
"""Package complete Bunker source, assets, build recipes, notices and test reports.
Device/wasm binaries and generated listening previews are separate downloads.
"""
from pathlib import Path
import zipfile

root = Path(__file__).resolve().parents[1]
target = root / 'dist/Bunker-Evolved-Source.zip'
def include(p):
    q = p.relative_to(root)
    if any(part in {'.git', '__pycache__', 'node_modules', '.openai'} for part in q.parts): return False
    s = q.as_posix()
    if s.startswith(('firmware/build/', 'dist/audio/')): return False
    if s == 'dist/media/SHA256SUMS.txt': return False  # generated after this archive; no self-checksum
    if s.startswith('firmware/') and p.suffix == '.log': return False
    if s.startswith('dist/') and p.suffix in {'.zip', '.gz', '.fwsc', '.wasm', '.mp4'}: return False
    return True

with zipfile.ZipFile(target, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as z:
    for p in sorted(root.rglob('*')):
        if p.is_file() and include(p):
            z.write(p, 'Bunker-Evolved/' + p.relative_to(root).as_posix())
print(f'{target.name}: {target.stat().st_size} bytes')
