#!/usr/bin/env python3
"""Code-native panel illustration, derived from the existing emulator layout."""
from pathlib import Path
R=Path(__file__).resolve().parents[1]; D=R/'dist'; (D/'media').mkdir(exist_ok=True)
svg=['<svg xmlns="http://www.w3.org/2000/svg" width="1000" height="624" viewBox="0 0 1000 624"><title>FM-1 panel illustration</title>']
def rect(x,y,w,h,r,fill,stroke='none'):
    svg.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{fill}" stroke="{stroke}" stroke-width="2"/>')
def txt(x,y,s,size=14,ink='#b6b7c7'):
    svg.append(f'<text x="{x}" y="{y}" text-anchor="middle" font-family="monospace" font-size="{size}" fill="{ink}">{s}</text>')
rect(4,4,992,616,40,'#1b1c26','#353544')
for x,y,w,h in [(50,250,220,50),(568,160,400,140),(36,332,928,256)]:rect(x,y,w,h,14,'#101118')
rect(292,44,256,256,18,'#05060a')
for name,x,y,r in [('MASTER',105,100,24),('SELECT',215,100,24),('PRESETS',105,205,24),('ALGORITHM',215,205,24)]+[(f'KNOB {i+1}',610+i*104,100,22) for i in range(4)]:
    svg.append(f'<circle cx="{x}" cy="{y}" r="{r+5}" fill="#101118"/><circle cx="{x}" cy="{y}" r="{r}" fill="#2b2d3a" stroke="#47495a" stroke-width="2"/><path d="M{x} {y-r+5}v9" stroke="#9ca0b7" stroke-width="3"/>')
    txt(x,y-r-13,name,13)
for name,x in [('OCT−',60),('OCT+',170)]:rect(x,258,90,34,8,'#292c38','#454757');txt(x+45,280,name,13)
for row,names in enumerate([['FX','SCL','ENV','LFO','EDIT','GLO'],['HOME','SAVE','ARP','SEQ','PLAY','REC']]):
    for i,name in enumerate(names):
        x,y=582+i*63,174+row*62;rect(x,y,54,48,8,'#96f5a0' if name=='PLAY' else '#292c38','#454757');txt(x+27,y+28,name,12,'#0c1813' if name=='PLAY' else '#ff8cc3' if name=='REC' else '#d5d6e2')
pitch=904/16; kw=pitch*.78;kh=256*.425
for row,locs in [(0,[a+1 for a in [0,1,2,4,5,7,8,9,11,12,14]]),(1,[i+.5 for i in range(16)])]:
    for j,v in enumerate(locs):
        x=48+v*pitch;y=332+256*(.25 if row==0 else .75)-kh/2
        rect(x-kw/2,y,kw,kh,kw/2,'#080a10','#343644');rect(x-3,y+kw*.4,6,kh-kw*.8,3,'#5c5f75')
txt(838,320,'FM-1 / BUNKER EVOLVED',11,'#71768a');svg.append('</svg>')
(D/'media/panel.svg').write_text(''.join(svg))

p=D/'app.js';s=p.read_text();mapping={'Vorschau konnte nicht geladen werden. Bitte erneut versuchen.':'Preview could not be loaded. Please try again.',' anhören':' — play preview',' · Firmware-Audio · 16 Sekunden':' · Firmware audio · 16 seconds','Wiedergabe blockiert. Bitte noch einmal auf ▶ drücken.':'Playback was blocked. Press play again.','Bibliothek konnte nicht geladen werden. Seite neu laden.':'The sound library could not be loaded. Reload the page.'}
for a,b in mapping.items():s=s.replace(a,b)
p.write_text(s)
p=D/'simulator/index.html';s=p.read_text();mapping={'Bunker Evolved: dieselbe generative FM-1-Firmware im Browser, mit Sound und vollständigem Gerätepanel.':'The Bunker Evolved C firmware in your browser, with audio and the FM-1 control panel.','gleicher C-Code':'same C program','Einschalten':'Power on','HOME oder SELECT: World / Sound / Motif / Evolve / Mix.':'HOME or SELECT: World / Grow / Sound / Motif / Evolve / Mix.','ALGORITHM: Spur. Regler ziehen oder Mausrad. WORLD und MOTIF mit OCT+ übernehmen.':'ALGORITHM: track. Drag knobs or use the wheel. OCT+ applies WORLD, GROW and MOTIF.','basiert auf':'is based on','Installieren':'Install','Bedienung':'Guide','Quellcode':'Source'}
for a,b in mapping.items():s=s.replace(a,b)
p.write_text(s)
