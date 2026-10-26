// SPDX-License-Identifier: GPL-3.0-only
// Portable composition model: no DOM, MIDI, filesystem or audio dependencies.
// Motifs and GROW adapted from Bunker's bunker_ui.c / bunker_grow.c.
import {integer,validateStep} from './protocol.mjs';
export const ROLE=['AIR','MELODY','ROOT','SPARK'];
export const NOTES=['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];
export const P={level:0,attack:1,release:4,lrate:9,lwave:10,lfilter:14,root:25,scale:26,quant:27,length:29,division:30,gate:32,chorus:34,delay:35,reverb:36,voice:37,pan:39,mute:40,ltrigger:92,spread:95,evolve:103,amount:104,every:105};
export const TONE=[4,2,1,2,3,1,4,1,7,2,2,2,2,7];
export const copy=x=>JSON.parse(JSON.stringify(x));
export const rest=()=>({n:0,notes:[0,0,0,0],time:2,flags:0,vel:76,hit:0,acc:0,chance:100,ratchet:1,nudge:0,condition:0});
export function noteName(n){return NOTES[n%12]+(Math.floor(n/12)-1);}
export function parseNotes(text){const tokens=text.trim().split(/[ ,]+/).filter(Boolean);if(tokens.length>4)throw Error('Use up to four notes');return tokens.map(t=>{if(/^\d+$/.test(t))return integer(Number(t),0,127);const m=/^([a-g])([#b]?)(-?\d)$/i.exec(t);if(!m)throw Error('Use notes such as C4 E4 G4 or MIDI numbers');return integer(({C:0,D:2,E:4,F:5,G:7,A:9,B:11})[m[1].toUpperCase()]+(m[2]==='#'?1:m[2]==='b'?-1:0)+(Number(m[3])+1)*12,0,127);});}
export function snap(note,root,mask){const n=Math.max(24,Math.min(108,note));for(let j=0;j<12;j++)if(n-j>=24&&(mask&(1<<((n-j-root+120)%12))))return n-j;for(let j=1;j<12;j++)if(mask&(1<<((n+j-root+120)%12)))return n+j;throw Error('Empty scale');}
export function walk(note,steps,root,mask){let n=note,dir=steps<0?-1:1;for(let i=0;i<Math.abs(steps);i++){let next=n;for(let j=0;j<12;j++){next+=dir;if(next<24||next>108)return n;if(mask&(1<<((next-root+120)%12)))break;}n=next;}return n;}
export function random(seed){let x=(seed>>>0)||1;return()=>{x^=x<<13;x^=x>>>17;x^=x<<5;return x>>>0;};}
export function motif(track,k,degrees,density,mask){integer(density,1,16);const ds=Array.from({length:12},(_,i)=>i).filter(i=>mask&(1<<i));if(!ds.length)throw Error('Empty scale');let hit=0;const base=[48,60,36,72][k];const degree=d=>Math.max(24,Math.min(108,base+track.p[P.root]+12*Math.floor(d/ds.length)+ds[((d%ds.length)+ds.length)%ds.length]));return Array.from({length:64},(_,i)=>{const s=rest();s.vel=0;if(i>=track.p[P.length]||(i*density%16)>=density)return s;const d=degrees[hit++&7];s.n=k===0?2:1;s.notes[0]=degree(d);if(s.n===2)s.notes[1]=degree(d+4);s.time=0;s.vel=k===0?65:k===2?76:68+(i*7%24);s.chance=k===3?80:100;return s;});}
export function grow(track,k,{root,scale,style,seed},scales){integer(root,0,11);integer(scale,0,31);integer(style,0,3);if([10,13].includes(track.engine))throw Error('Choose a pitched engine: drum and slice notes are addresses');const p=copy(track.steps),len=track.p[P.length],mask=scales[scale].mask,rng=random(seed),delta=root-track.p[P.root];if(!p.slice(0,len).some(s=>s.n&&s.time===0)&&style===3)for(let i=0;i<len;i+=(k===1||k===3?4:8)){p[i]=rest();p[i].n=k===0?2:1;p[i].time=0;p[i].notes[0]=[48,60,36,72][k];if(k===0)p[i].notes[1]=55;}
 const phrase=Array.from({length:8},()=>rng()%9-4);let hit=0,changed=0;
 for(let i=0;i<64;i++){const s=p[i];for(let j=0;j<s.n;j++)s.notes[j]=snap(s.notes[j]+delta,root,mask);if(i>=len||!s.n||s.time!==0||!style)continue;let shift=0;if(style===3)shift=phrase[hit++&7];else if(rng()%100<(style===1?30:65)){shift=1+rng()%(style===1?1:3);if(rng()&1)shift=-shift;}for(let j=0;j<s.n;j++){const old=s.notes[j];s.notes[j]=walk(old,shift,root,mask);changed+=old!==s.notes[j];}}
 if(style&&!changed){const s=p.slice(0,len).find(s=>s.n&&s.time===0);if(s){const n=s.notes[0],next=walk(n,1,root,mask);s.notes[0]=next===n?walk(n,-1,root,mask):next;}}
 p.forEach(validateStep);return p;
}
export function phraseFile(track){return {format:'bunker-phrase',version:1,root:track.p[P.root],scale:track.p[P.scale],length:track.p[P.length],steps:copy(track.steps)};}
export function readPhrase(x){if(x?.format!=='bunker-phrase'||x.version!==1)throw Error('Not a Bunker phrase v1 file');integer(x.root,0,11);integer(x.scale,0,31);integer(x.length,1,64);if(!Array.isArray(x.steps)||x.steps.length!==64)throw Error('Exactly 64 steps required');x.steps.forEach(validateStep);return copy(x);}
