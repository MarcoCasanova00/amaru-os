const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict');
const {webcrypto}=require('node:crypto'),{parseHTML}=require('linkedom');
(async()=>{
 const mod=await import('data:text/javascript;base64,'+Buffer.from(fs.readFileSync('dist/install/fm1backup.js')).toString('base64'));
 const {window}=parseHTML(fs.readFileSync('dist/install/index.html','utf8')),document=window.document;
 let id='FM-1_7100002',reads=0,puts=0,downloads=0,closed=0,busyLock=false,staged;
 const objects=new Map(mod.BACKUP_IDS.map((id,i)=>[id,new Uint8Array(i<2?64:0).fill(i+1)]));
 async function request([cmd,a]){
   if(cmd===65){reads++;const r=[1,0,mod.BACKUP_IDS.length];for(const id of mod.BACKUP_IDS){const b=objects.get(id);r.push(id,...mod.bkU32(b.length),...mod.bkU32(b.length?mod.bkCrc(b):0));}return r;}
   if(cmd===66){reads++;const off=mod.bkR32(a,1),n=a[6]|a[7]<<7;return [a[0],0,...mod.bkU32(off),n&127,n>>7,...mod.bkPack(objects.get(a[0]).slice(off,off+n))];}
   if(cmd===67){puts++;const [op,obj]=a;if(op===0)staged={id:obj,bytes:new Uint8Array(mod.bkR32(a,2))};if(op===1){const off=mod.bkR32(a,2);const packed=a.slice(7);const size=packed.length-Math.ceil(packed.length/8);staged.bytes.set(mod.bkUnpack(packed,size),off);}if(op===2)objects.set(obj,staged.bytes);return [op,obj,0];}
   if(cmd===14){puts++;return [a[0],0];}
   throw Error('unexpected request '+cmd);
 }
 class BackupConnection{constructor(){assert(busyLock);}request(...args){return request(...args);}close(){closed++;}}
 class Updater{async find(filter){const d={model:'FM-1',text:id};return filter(d)?{id:d,link:{input:{},output:{},close(){}}}:null;}}
 let downloaded;
 const c={...mod,BackupConnection,Updater,window,document,console,crypto:webcrypto,Uint8Array,Blob,Date,JSON,setTimeout,
   URL:{createObjectURL(blob){downloaded=blob;downloads++;return 'blob:test';},revokeObjectURL(){}},
   localStorage:{getItem(){return null},setItem(){},removeItem(){}},
   navigator:{requestMIDIAccess:async()=>({inputs:new Map([['1',{state:'connected',name:'FM-1'}]])}),locks:{request:async(n,o,fn)=>{if(busyLock)return fn(null);busyLock=true;try{return await fn({});}finally{busyLock=false;}}}},
   fetch:async p=>{const b=fs.readFileSync('dist/install/'+p);return {ok:true,json:async()=>JSON.parse(b),arrayBuffer:async()=>b.buffer.slice(b.byteOffset,b.byteOffset+b.byteLength)};}
 };
 vm.createContext(c);vm.runInContext(fs.readFileSync('dist/install/fm1pkg.js','utf8').replaceAll('export ',''),c);
 vm.runInContext(fs.readFileSync('dist/install/installer.js','utf8').replace(/^import .*;$/gm,'').replace('export async','async'),c);
 await document.getElementById('connect').onclick();assert.equal(puts,0);
 await document.getElementById('backupButton').onclick();assert.equal(downloads,1);assert.equal(puts,0);assert(reads>=3);assert.equal(closed,1);
 const file=await downloaded.text();assert.equal(mod.readBackup(file).firmware,'FM-1_7100002');
 const pick=async text=>document.getElementById('backupFile').onchange({target:{files:[{size:text.length,text:async()=>text}]}});
 await pick(file);assert(document.getElementById('restoreButton').disabled);
 await document.getElementById('restoreButton').onclick();assert.equal(puts,0);
 document.getElementById('restoreReady').checked=true;document.getElementById('restoreReady').onchange();assert.equal(document.getElementById('restoreButton').disabled,false);
 await document.getElementById('restoreButton').onclick();assert(puts>0);assert.equal(closed,2);assert(!document.getElementById('restoreReady').checked);assert(document.getElementById('backupStatus').textContent.includes('restored'));
 const before=puts;const corrupt=JSON.parse(file);corrupt.objects[0].crc^=1;await pick(JSON.stringify(corrupt));assert(document.getElementById('restoreButton').disabled);assert.equal(puts,before);
 const wrong=JSON.parse(file);wrong.firmware='FM-1_015';await pick(JSON.stringify(wrong));assert(document.getElementById('restoreButton').disabled);
 id='FM-1_015';await document.getElementById('connect').onclick();assert(document.getElementById('backupButton').disabled);await document.getElementById('backupButton').onclick();assert.equal(downloads,1);
 console.log('PASS backup UI + real archive codec: read-only backup, CRC-checked file, explicit restore consent, full object transfer, corrupt and foreign archives refused, stock disabled, connection cleanup. Simulated MIDI objects, not hardware.');
})().catch(e=>{console.error(e);process.exit(1)});
