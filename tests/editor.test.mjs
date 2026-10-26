// SPDX-License-Identifier: GPL-3.0-only
// End-to-end JS editor requests -> actual C firmware SysEx handler -> JS readback.
import assert from 'node:assert/strict';import {spawn} from 'node:child_process';import readline from 'node:readline';import fs from 'node:fs';
import {Session,CMD,frame,unframe,v14,Reader,encodeStep,decodeStep,trackDump} from '../dist/editor/core/protocol.mjs';
import {Device} from '../dist/editor/core/device.mjs';import {OfflineDevice} from '../dist/editor/adapters/offline.mjs';
import {P,copy,rest,motif,grow,phraseFile,readPhrase} from '../dist/editor/core/music.mjs';
const data=JSON.parse(fs.readFileSync(new URL('../dist/editor/data.json',import.meta.url)));
let checks=0;const eq=(a,b)=>{assert.deepEqual(a,b);checks++;};
function bridge(){const child=spawn('./firmware/build/host/editor_bridge',[],{stdio:['pipe','pipe','inherit']});const pending=[];readline.createInterface({input:child.stdout}).on('line',l=>{const v=JSON.parse(l);if(pending.length)pending.shift()(v);else t.onData?.(v);});const t={send(d){child.stdin.write(Array.from(d).join(' ')+'\n');},close(){child.stdin.end();},manual(text){return new Promise(r=>{pending.push(r);child.stdin.write(text+'\n');});},onData:null,onClose:null};child.on('exit',code=>{assert.equal(code,0);});return t;}
const transport=bridge(),session=new Session(transport,{gap:0,timeout:4000}),d=new Device(session,data);
try{
 await d.initialise();eq(d.info.pcount,114);eq(d.descs[0][P.scale].names.length,32);eq(d.info.pe0,106);
 for(let k=0;k<4;k++){await d.set(k,P.level,70+k);eq((await d.readPhrase(k)).p[P.level],70+k);}
 console.log('PASS: identity, four-track read/write, 114 parameters and 32 scales');
 // All 64 browser recipe applications must equal the hardware's own bunker_sound().
 for(let k=0;k<4;k++)for(let i=0;i<16;i++){await d.sound(k,i);const browser=trackDump(await d.rq(CMD.TRACK_DUMP,[k]),114);await transport.manual(`!sound ${k} ${i}`);const native=trackDump(await d.rq(CMD.TRACK_DUMP,[k]),114);eq(browser,native);}
 console.log('PASS: all 64 sound recipes equal native bunker_sound(), all parameters');
 // All 24 motifs across four roles match native notes, rests, chance and velocities.
 for(let k=0;k<4;k++){await d.set(k,P.length,32);await d.set(k,P.root,7);await d.set(k,P.scale,17);for(let i=0;i<24;i++){const t=await d.readPhrase(k),density=(i%16)+1;const js=motif(t,k,data.library.motifs[i][1],density,data.scales[17].mask);await transport.manual(`!motif ${k} ${i} ${density}`);eq((await d.readPhrase(k)).steps.map(encodeStep),js.map(encodeStep));}}
 console.log('PASS: 24 motifs × four roles equal native bunker_pattern()');
 const before=await d.readPhrase(1);const steps=grow(before,1,{root:3,scale:30,style:2,seed:321},data.scales);await d.writePhrase(1,steps,{[P.root]:3,[P.scale]:30,[P.quant]:3,[P.evolve]:0});eq((await d.readPhrase(1)).steps.map(encodeStep),steps.map(encodeStep));await d.undoPhrase();eq(d.signature(await d.readPhrase(1)),d.signature(before));
 await d.writePhrase(1,steps);await d.set(1,P.root,2);await assert.rejects(()=>d.undoPhrase(),/changed/);checks++;
 console.log('PASS: real phrase writes, readback, undo and stale-undo refusal');
 const exotic=rest();Object.assign(exotic,{n:4,notes:[24,37,89,108],time:0,flags:3,vel:103,hit:255,acc:165,chance:100,ratchet:4,nudge:-8,condition:11});const arr=Array.from({length:64},()=>copy(exotic));await d.writePhrase(2,arr);eq((await d.readPhrase(2)).steps.map(encodeStep),arr.map(encodeStep));
 await assert.rejects(()=>d.set(1,P.level,999));assert.throws(()=>readPhrase({...phraseFile(before),steps:[]}));assert.throws(()=>frame(3,[128]));eq(unframe([240,125,70,76,3,255,247]),null);checks+=3;
 for(const v of [-8192,-1,0,8191])eq(new Reader(v14(v)).v(),v);
 for(let root=0;root<12;root++)for(let scale=0;scale<32;scale++)for(let style=0;style<4;style++){const out=grow(before,1,{root,scale,style,seed:511},data.scales);for(const s of out)for(let j=0;j<s.n;j++){assert.ok(s.notes[j]>=24&&s.notes[j]<=108);assert.ok(data.scales[scale].mask&(1<<((s.notes[j]-root+120)%12)));checks+=2;}}
 eq(readPhrase(phraseFile(before)),phraseFile(before));const offline=await new OfflineDevice(data).initialise();await offline.sound(2,0);eq(offline.tracks[2].p[P.voice],1);
 console.log('PASS: all 1,536 key/scale/style combinations stay in scale and range; import validation; full step preservation');
}finally{session.close();}
// Fault injection: a timeout kills queued writes, no automatic retries.
let sent=0;const drop={send(){sent++;},close(){}};const failing=new Session(drop,{timeout:15,gap:0});const results=await Promise.allSettled([failing.request(25),failing.request(3,[0,0,...v14(12)])]);eq(results.map(x=>x.status),['rejected','rejected']);eq(sent,1);eq(failing.closed,true);
// Ignore unrelated frames and mismatched track/parameter responses.
const fake={send(bytes){const f=unframe(bytes);queueMicrotask(()=>{fake.onData(frame(f.cmd,[2,0,...v14(88)]));fake.onData(frame(f.cmd,[1,0,...v14(70)]));});},close(){}};const s=new Session(fake,{gap:0});eq(await s.request(31,[1,0,...v14(70)]),[1,0,...v14(70)]);s.close();
console.log(`PASS: ${checks} assertions. Actual C handler used; zero explicit flash erases/writes. No physical USB or browser-render verification.`);
