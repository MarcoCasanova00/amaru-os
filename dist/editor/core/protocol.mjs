// SPDX-License-Identifier: GPL-3.0-only
// Bunker editor codec, derived from Felucca's EDITOR_PROTOCOL.md / editor.c.
// Protocol design: Leo Kuroshita (@kurogedelic), Hügelton Instruments.
export const CMD={INFO:1,GET:2,SET:3,DUMP:4,DESC:5,PRESET:8,NAMES:10,TRACK:27,TRACK_DUMP:29,TRACK_STEP:30,TRACK_PARAM:31,SONG:33};
export function integer(x,lo,hi){if(!Number.isInteger(x)||x<lo||x>hi)throw Error(`Expected integer ${lo}…${hi}`);return x;}
export function v14(v){const u=integer(v,-8192,8191)+8192;return [u&127,u>>7];}
export function frame(cmd,args=[]){return Uint8Array.from([240,125,70,76,integer(cmd,0,127),...args.map(x=>integer(x,0,127)),247]);}
export function unframe(data){const a=Array.from(data||[]);if(a.length<6||a[0]!==240||a[1]!==125||a[2]!==70||a[3]!==76||a.at(-1)!==247)return null;if(a.slice(4,-1).some(x=>!Number.isInteger(x)||x<0||x>127))return null;return {cmd:a[4],args:a.slice(5,-1)};}
export class Reader{constructor(a){this.a=a;this.i=0;} b(){if(this.i>=this.a.length)throw Error('Incomplete device reply');return integer(this.a[this.i++],0,127);} v(){return (this.b()|(this.b()<<7))-8192;} s(){let s='';for(let n=0;n<128;n++){const x=this.b();if(!x)return s;s+=String.fromCharCode(x);}throw Error('Invalid device string');}}
export function info(a){const r=new Reader(a);const o={version:r.s(),nengines:r.b(),pcount:r.b(),gcount:r.b(),nstep:r.b(),pe0:r.b(),engines:[]};for(let i=0;i<o.nengines;i++)o.engines.push(r.s());o.ntrk=r.b();return o;}
export function descriptor(a){const r=new Reader(a),d={scope:r.b(),id:r.b(),fmt:r.b(),min:r.v(),max:r.v(),def:r.v(),label:r.s(),unit:r.s(),names:[]};if(d.fmt===8)while(r.i<a.length)d.names.push(r.s());return d;}
export function trackDump(a,count){const r=new Reader(a),d={track:r.b(),engine:r.b(),preset:r.b(),p:[]};for(let i=0;i<count;i++)d.p.push(r.v());return d;}
export function validateStep(s){integer(s.n,0,4);if(!Array.isArray(s.notes)||s.notes.length!==4)throw Error('Four note slots required');s.notes.forEach(n=>integer(n,0,127));integer(s.time,0,2);integer(s.flags,0,3);integer(s.vel,0,127);integer(s.hit,0,255);integer(s.acc,0,255);if(s.acc&~s.hit)throw Error('Accent without hit');integer(s.chance,0,100);integer(s.ratchet,1,4);integer(s.nudge,-8,7);integer(s.condition,0,11);return s;}
export function encodeStep(s){validateStep(s);return [s.n,...s.notes,s.time,s.flags,s.vel,s.hit&127,s.acc&127,(s.hit>>7)|((s.acc>>7)<<1),s.chance,s.ratchet,s.nudge+8,s.condition];}
export function decodeStep(a){const r=new Reader(a),s={n:r.b(),notes:[r.b(),r.b(),r.b(),r.b()],time:r.b(),flags:r.b(),vel:r.b()};const h=r.b(),c=r.b(),x=r.b();s.hit=h|((x&1)<<7);s.acc=c|((x&2)<<6);s.chance=r.b();s.ratchet=r.b();s.nudge=r.b()-8;s.condition=r.b();return validateStep(s);}
export function matches(cmd,req,reply){if([2,3,5,30,31].includes(cmd))return reply[0]===req[0]&&reply[1]===req[1];if([8,10,29].includes(cmd))return reply[0]===req[0];if(cmd===33)return reply[0]===req[0];if(cmd===27&&req.length)return reply[0]===req[0];return true;}
// One in-flight request; no automatic retries. A timeout invalidates the session,
// preventing a delayed reply from being mistaken for a subsequent write.
export class Session{
 constructor(transport,{timeout=1800,gap=8,onFault=()=>{}}={}){this.transport=transport;this.timeout=timeout;this.gap=gap;this.onFault=onFault;this.closed=false;this.tail=Promise.resolve();this.pending=null;this.count=0;transport.onData=d=>this.receive(d);transport.onClose=()=>this.close(Error('FM-1 disconnected. Reconnect and read again.'));}
 request(cmd,args=[]){if(this.closed)return Promise.reject(Error('Connection closed'));if(this.count>=16)return Promise.reject(Error('Command queue full'));frame(cmd,args);this.count++;const next=this.tail.then(()=>this.perform(cmd,args));this.tail=next.catch(()=>{});return next.finally(()=>this.count--);}
 async perform(cmd,args){if(this.gap)await new Promise(r=>setTimeout(r,this.gap));if(this.closed)throw Error('Connection closed');return new Promise((resolve,reject)=>{const timer=setTimeout(()=>this.close(Error(`No reply to command ${cmd}. Reconnect and read again; the last operation may be partial.`)),this.timeout);this.pending={cmd,args,resolve,reject,timer};try{this.transport.send(frame(cmd,args));}catch(e){this.close(e);}});}
 receive(data){const f=unframe(data),p=this.pending;if(!f||!p||f.cmd!==p.cmd||!matches(p.cmd,p.args,f.args))return;clearTimeout(p.timer);this.pending=null;p.resolve(f.args);}
 close(error=Error('Disconnected')){if(this.closed)return;this.closed=true;if(this.pending){clearTimeout(this.pending.timer);this.pending.reject(error);this.pending=null;}this.transport.close?.();this.onFault(error);}
}
