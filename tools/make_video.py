#!/usr/bin/env python3
"""Compose real C screenshots and audio with the approved English narration.
Needs Pillow, cairosvg and ffmpeg. No invented controls or generated music.
Run the host capture tool first; input frames live outside the source tree.
"""
import json, math, subprocess, textwrap, re, os
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import cairosvg
R=Path(__file__).resolve().parents[1];M=R/'dist/media';W=Path(os.environ.get('BUNKER_VIDEO_CAPTURE',str(R/'firmware/build/video')))
chapters=json.loads((R/'docs/video/script.json').read_text())
SANS=R/'firmware/assets/fonts/InterTight[wght].ttf';MONO='/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf'
def font(n,mono=False,bold=False):
    f=ImageFont.truetype(str(MONO if mono else SANS),n)
    if not mono:f.set_variation_by_axes([700 if bold else 450])
    return f
bg='#080a12';ink='#e9edf4';muted='#9da8bd';purple='#b893ff';green='#96f5a0';cyan='#73d7e8';pink='#ff93c3'
cairosvg.svg2png(url=str(M/'panel.svg'),write_to=str(W/'panel.png'),output_width=1000,output_height=624)
panel=Image.open(W/'panel.png').convert('RGBA')
headlines=[['BUNKER','EVOLVED'],['Choose a world.','Make it yours.'],['A scale. A spark.','A little GROW.'],['Shape every','layer.'],['Let the phrase','find its way.'],['From original','to evolved.'],['Keep a way','back.'],['Make something','that grows.']]
notes=[['FREE GENERATIVE AMBIENT FIRMWARE','FOR THE M-VAVE FM-1'],['Stop → choose WORLD → OCT+','ALGORITHM selects a layer.','HOME changes the musical view.'],['1  ROOT       2  SCALE','3  STYLE      4  ONE / ALL','OCT+ APPLY    OCT− UNDO'],['16 starting sounds per role.','EDIT / ENV / LFO / FX','SEQ for notes. SAVE for projects.'],['MODE → CHANCE → LOOPS','OCT+ makes one step.','OCT− locks your phrase.'],['01  Charge. Connect by USB.','02  Detect. Back up. Select.','03  Install. Keep power connected.'],['Application works → roll back','MIDI loader answers → retry','No MIDI reply → recovery guide'],['FREE TO PLAY. FREE TO EXPLORE.','A project by Kid Bunka.','Built on Felucca. Open source.']]
def label(draw,xy,txt,size=16,color=muted,mono=False,bold=False):draw.text(xy,txt,font=font(size,mono,bold),fill=color)
def frame(x,n):
    i=x['index'];t=n/10;dur=x['duration'];im=Image.new('RGB',(1280,720),bg);d=ImageDraw.Draw(im)
    for k in range(9):
        yy=105+k*57;d.line((56,yy,1224,yy),fill='#111521',width=1)
    d.line((56,70,1224,70),fill='#303348');label(d,(56,28),'BUNKER / EVOLVED',17,ink,True)
    label(d,(1008,30),f'{i+1:02d} / 08',14,purple,True);d.ellipse((1199,31,1207,39),fill=green)
    label(d,(56,112),['MEET THE INSTRUMENT','WORLD / FOUR LAYERS','GROW / SCALE VARIATIONS','SOUND / FULL EDITORS','EVOLVE / BOUNDED CHANGE','INSTALL / DESKTOP CHROME','RESCUE / KNOW THE ROUTE','A GIFT TO CURIOUS MUSICIANS'][i],14,green,True)
    if i in (0,7):
        label(d,(52,164),headlines[i][0],68,ink,False,True);label(d,(52,241),headlines[i][1],68,purple,False,True)
        for k,line in enumerate(notes[i]):label(d,(56,374+k*34),line,16,muted,k==0)
        p=panel.copy();scr=Image.open(W/x['name']/f'{n:04d}.ppm').convert('RGBA');p.alpha_composite(scr,(300,52));p.thumbnail((640,400));im.paste(p,(601,175),p)
        label(d,(665,591),'ACTUAL C DISPLAY / ILLUSTRATED PANEL',10,muted,True)
        if i==0:
            for k,(num,name,c) in enumerate([('4','LAYERS',green),('64','SOUNDS',cyan),('24','WORLDS',purple),('32','SCALES',pink)]):
                xx=56+k*138;label(d,(xx,524),num,40,c);label(d,(xx,578),name,11,muted,True)
    else:
        for k,line in enumerate(headlines[i]):label(d,(52,167+k*62),line,53,purple if k else ink,False,True)
        for k,line in enumerate(notes[i]):label(d,(56,355+k*40),line,19,ink,True)
        d.rounded_rectangle((793,116,1217,560),radius=14,fill='#171926',outline='#3e4156',width=2)
        scr=Image.open(W/x['name']/f'{n:04d}.ppm').resize((384,384),Image.Resampling.NEAREST);im.paste(scr,(813,136))
        label(d,(835,535),'ACTUAL FIRMWARE / SYNCHRONISED AUDIO',10,muted,True)
        if i==2:
            tags=['TUNE','SOFT','FLOW','NEW']
            for j,tag in enumerate(tags):
                xx=56+j*150;d.rectangle((xx,510,xx+130,547),outline='#394154');label(d,(xx+15,518),tag,17,green,True)
            label(d,(56,571),'Sounds and mix stay intact.',19,muted)
        elif i==3:label(d,(56,539),'13 engines · 8 shared voices · 4 projects',19,green)
        elif i==4:label(d,(56,539),'WALK · ANSWER · ORBIT · MIRROR\nBLOOM · BREATH · VOICE',17,green,True)
        elif i==5:
            label(d,(56,511),'No AMB-1 intermediate step required.',20,green)
            label(d,(56,550),'Stock V15 route: protocol supported.\nPhysical round-trip test still to document.',15,muted)
        elif i==6:
            label(d,(56,509),'Rollback does not restore overwritten music.',17,pink)
            label(d,(56,548),'UBOOT is a separate chip mode.\nChrome recovery is not guaranteed.',17,muted)
    d.line((56,636,1224,636),fill='#303348');progress=(x['start']+t)/sum(a['duration'] for a in chapters);d.line((56,636,56+1168*progress,636),fill=green,width=2)
    label(d,(56,663),'FREE / OPEN SOURCE / 1.0 RC2',12,muted,True)
    label(d,(798,663),'BUNKER-EVOLVED.SUGARYPIXIE10.CHATGPT.SITE',10,muted,True)
    # A gentle six-frame fade between chapters, applied to the composite.
    fade=min(1,t/.6,(dur-t)/.6)
    if fade<1:im=Image.blend(Image.new('RGB',im.size,bg),im,max(0,fade))
    return im

def run(cmd):subprocess.run(cmd,check=True)
for x in chapters:
    i=x['index'];p=W/x['name'];frames=round(x['duration']*10)
    print('Composing',x['name'],frames,flush=True)
    # Music is the sound associated with these screenshots. Bring it up in speech gaps.
    filt=f"[0:a]loudnorm=I=-23:TP=-3:LRA=7,volume='if(between(t,2,{2+x['voiceDuration']:.3f}),0.20,0.8)':eval=frame,afade=t=in:d=1,afade=t=out:st={x['duration']-1:.2f}:d=1[m];[1:a]loudnorm=I=-17:TP=-2:LRA=7,adelay=2000|2000[v];[m][v]amix=inputs=2:normalize=0,alimiter=limit=0.89:level=false,apad,atrim=duration={x['duration']}[a]"
    run(['ffmpeg','-v','error','-y','-i',str(p/'audio.wav'),'-i',str(M/f"voice-{x['name']}.mp3"),'-filter_complex',filt,'-map','[a]','-ar','48000',str(p/'mixed.wav')])
    cmd=['ffmpeg','-v','error','-y','-f','rawvideo','-pix_fmt','rgb24','-s','1280x720','-r','10','-i','pipe:0','-i',str(p/'mixed.wav'),'-c:v','libx264','-preset','fast','-crf','25','-pix_fmt','yuv420p','-r','30','-c:a','aac','-b:a','160k','-movflags','+faststart','-shortest',str(p/'chapter.mp4')]
    proc=subprocess.Popen(cmd,stdin=subprocess.PIPE)
    for n in range(frames):
        im=frame(x,n)
        if n==80:im.save(R/f'docs/video/frame-{i}.jpg',quality=90)
        if i==0 and n==80:im.save(M/'thumbnail.jpg',quality=95)
        proc.stdin.write(im.tobytes())
    proc.stdin.close()
    if proc.wait():raise RuntimeError('Video encoding failed')

(W/'chapters.txt').write_text(''.join(f"file '{W/x['name']/'chapter.mp4'}'\n" for x in chapters))
run(['ffmpeg','-v','error','-y','-f','concat','-safe','0','-i',str(W/'chapters.txt'),'-c','copy','-movflags','+faststart',str(M/'Bunker-Evolved-Introduction.mp4')])
def stamp(t,sep='.'):
    ms=round(t*1000);return f'{ms//3600000:02}:{ms//60000%60:02}:{ms//1000%60:02}{sep}{ms%1000:03}'
vtt=['WEBVTT\n'];srt=[];n=0
for x in chapters:
    # Sentence-weighted timings within each independently generated chapter.
    chunks=re.split(r'(?<=[.!?])\s+',x['script']);weights=[len(a.split()) for a in chunks];total=sum(weights);off=x['start']+2
    for chunk,weight in zip(chunks,weights):
        end=off+x['voiceDuration']*weight/total;n+=1;text='\n'.join(textwrap.wrap(chunk,64));vtt.append(f'{stamp(off)} --> {stamp(end)}\n{text}\n');srt.append(f'{n}\n{stamp(off,",")} --> {stamp(end,",")}\n{text}\n');off=end
(M/'Bunker-Evolved-English.vtt').write_text('\n'.join(vtt));(M/'Bunker-Evolved-English.srt').write_text('\n'.join(srt))
print('Film created:',(M/'Bunker-Evolved-Introduction.mp4').stat().st_size,'bytes',flush=True)
