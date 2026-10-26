// SPDX-License-Identifier: GPL-3.0-only
// Local sketch model. No hardware, audio, or autonomous EVOLVE simulation.
import {Device} from '../core/device.mjs';
import {P,copy,rest,motif} from '../core/music.mjs';
import {integer,encodeStep} from '../core/protocol.mjs';
export class OfflineDevice extends Device{
 constructor(data){super(null,data);this.info={version:'OFFLINE SKETCH',pcount:114,pe0:106,nengines:data.ENG.length};this.g=data.GP.map(d=>d.def);this.gdescs=copy(data.GP);for(let k=0;k<4;k++){this.tracks.push({engine:0,preset:0,p:data.TP.concat(data.ENG[0].edit).map(d=>d.def),steps:Array.from({length:64},rest)});this.descs.push(copy(data.TP.concat(data.ENG[0].edit)));}}
 async initialise(){const w=this.data.library.worlds[0];for(let k=0;k<4;k++){await this.sound(k,w[1][k]);const t=this.tracks[k];t.p[P.root]=w[3];t.p[P.scale]=w[4];t.p[P.length]=32;t.p[P.division]=1;t.steps=motif(t,k,this.data.library.motifs[w[2][k]][1],w[6][k],this.data.scales[w[4]].mask);}return this;}
 async stop(){} async readTrack(k){return this.tracks[k];} async readPhrase(k){return copy(this.tracks[k]);}
 async set(k,id,v){const d=this.descs[k][id];integer(v,d.min,d.max);this.tracks[k].p[id]=v;return v;}
 async global(id,v){const d=this.gdescs[id];integer(v,d.min,d.max);this.g[id]=v;}
 async preset(k,engine,preset){integer(engine,0,this.data.ENG.length-1);const en=this.data.ENG[engine];integer(preset,0,en.presets.length-1);const sound=en.presets[preset],t=this.tracks[k];t.engine=engine;t.preset=preset;this.descs[k]=copy(this.data.TP.concat(en.edit));for(let i=0;i<8;i++)t.p[106+i]=sound.e[i];sound.env.forEach((v,i)=>t.p[1+i]=v);t.p[P.voice]=sound.mono?1:0;this.undo=null;}
 async writePhrase(k,steps,params={}){if(steps.length!==64)throw Error('Expected 64 steps');steps.forEach(encodeStep);const before=copy(this.tracks[k]);for(const [id,v] of Object.entries(params))await this.set(k,+id,v);this.tracks[k].p[P.evolve]=params[P.evolve]??0;this.tracks[k].steps=copy(steps);this.undo={k,before,after:this.signature(this.tracks[k])};return this.tracks[k];}
}
