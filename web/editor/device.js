/* AMARU Song Editor — device librarian (WebMIDI).
 *
 * Turns the editor into a librarian for the FM-1: read the device's songs/projects
 * and write them back, over the Felucca editor protocol.
 *
 * SAFETY: this module ONLY uses the firmware's validated commands — the backup
 * objects (BACKUP_LIST/GET/PUT). These run through the firmware's own A/B sector
 * pairs, CRC and commit records; nothing is written until the firmware validates
 * the content and commits. It never addresses flash directly and never touches
 * the bootloader header. See docs/SAFETY.md and web/EDITOR_PROTOCOL.md.
 *
 * NOTE: the protocol encoding here is written to the documented wire format. It
 * has NOT been verified against physical hardware in this environment — treat the
 * first hardware test as the verification step.
 */

"use strict";
const D = (function(){

  /* ----------------------------- SysEx framing -------------------------- */
  const MANUF = 0x7D, FL_A = 0x46, FL_B = 0x4C;
  const CMD = { INFO:1, BACKUP_LIST:65, BACKUP_GET:66, BACKUP_PUT:67 };

  /* pack7: groups of up to 7 bytes, each preceded by a byte holding their top bits */
  function pack7(bytes){
    const out=[];
    for(let i=0;i<bytes.length;i+=7){
      const g = bytes.slice(i,i+7);
      let hdr=0;
      for(let j=0;j<g.length;j++) if(g[j]&0x80) hdr |= 1<<j;
      out.push(hdr);
      for(let j=0;j<g.length;j++) out.push(g[j]&0x7f);
    }
    return out;
  }
  function unpack7(wire, expectedLen){
    const out=[]; let i=0;
    while(i<wire.length && out.length<expectedLen){
      const hdr=wire[i++];
      for(let j=0;j<7 && i<wire.length && out.length<expectedLen;j++){
        out.push(((hdr>>j)&1 ? 0x80 : 0) | (wire[i++]&0x7f));
      }
    }
    return out;
  }
  /* u32 over 5 x 7-bit bytes, LSB first; last byte is 0..15 */
  function u32to7(v){ v=v>>>0; const o=[]; for(let i=0;i<5;i++) o.push((v>>>(7*i))&0x7f); return o; }
  function u32from7(b){ let v=0; for(let i=0;i<5;i++) v |= (b[i]&0x7f) << (7*i); return v>>>0; }
  /* CRC-32 (zlib) */
  function crc32(buf){
    let crc=0xFFFFFFFF;
    for(let i=0;i<buf.length;i++){
      let c=(crc ^ buf[i]) & 0xFF;
      for(let k=0;k<8;k++) c = (c&1) ? (0xEDB88320 ^ (c>>>1)) : (c>>>1);
      crc = (crc>>>8) ^ c;
    }
    return (crc ^ 0xFFFFFFFF) >>> 0;
  }

  /* ----------------------------- MIDI session -------------------------- */
  function Session(){
    this.output = null; this.input = null;
    this.pending = null;   // {resolve, reject, cmd, buf[]}
    this.ready = false;
    this.info = null;
  }
  Session.prototype.onMessage = function(ev){
    const data = ev.data;
    if(!data || data[0] !== 0xF0) return;
    // strip F0 ... F7; find "FL" prefix
    const body = Array.from(data);
    if(body.length < 4 || body[1] !== MANUF || body[2] !== FL_A || body[3] !== FL_B) return;
    if(this.pending){
      // payload = all bytes after the FL prefix, before F7
      const payload = body.slice(4, body[body.length-1]===0xF7 ? body.length-1 : body.length);
      const p = this.pending; this.pending = null;
      p.resolve(payload);
    }
  };
  Session.prototype.send = function(payload){
    const frame = [0xF0, MANUF, FL_A, FL_B].concat(payload, [0xF7]);
    this.output.send(Array.from(frame));
    return new Promise((resolve,reject)=>{
      this.pending = {resolve, reject};
      setTimeout(()=>{ if(this.pending){ const p=this.pending; this.pending=null; p.reject(new Error("no reply (timeout)")); } }, 2000);
    });
  };
  Session.prototype.cmd = async function(cmd, args){
    const payload = [cmd].concat(args || []);
    const reply = await this.send(payload);
    // reply echoes cmd first (or starts with cmd byte)
    return reply;
  };
  Session.prototype.info = async function(){
    const r = await this.cmd(CMD.INFO);
    // INFO: version string (0-terminated?), NENGINES, P_COUNT, G_COUNT, NSTEP, P_E0, engine names...
    let i = 0; const s = [];
    while(i < r.length && r[i] !== 0) s.push(String.fromCharCode(r[i++]));
    i++; // skip NUL
    const NENGINES = r[i++];
    const P_COUNT = r[i++], G_COUNT = r[i++], NSTEP = r[i++], P_E0 = r[i++];
    const engines = [];
    for(let e=0;e<NENGINES;e++){
      let nm=""; while(i<r.length && r[i]!==0) nm += String.fromCharCode(r[i++]); i++;
      engines.push(nm);
    }
    this.info = { version:s.join(""), NENGINES, P_COUNT, G_COUNT, NSTEP, P_E0, engines };
    return this.info;
  };

  /* ----------------------------- backup (librarian) --------------------- */
  /* return [{id,size,crc}] */
  Session.prototype.backupList = async function(){
    const r = await this.cmd(CMD.BACKUP_LIST);
    // r = 1, rc, count, then per object id, size u32, crc u32
    let i = 0;
    if(r[i++] !== 1) throw new Error("BACKUP_LIST: bad reply");
    const rc = r[i++]; if(rc !== 0) throw new Error("BACKUP_LIST rc="+rc);
    const count = r[i++];
    const objects = [];
    for(let k=0;k<count;k++){
      const id = r[i++];
      const size = u32from7(r.slice(i,i+5)); i += 5;
      const crc = u32from7(r.slice(i,i+5)); i += 5;
      objects.push({id, size, crc});
    }
    return objects;
  };
  /* read one object fully; verifies CRC */
  Session.prototype.backupGet = async function(id, size, expectCrc){
    const chunks = [];
    let offset = 0;
    while(offset < size){
      const count = Math.min(256, size - offset);
      const args = [id].concat(u32to7(offset), [count & 0x7f, (count>>7) & 0x7f]);
      const r = await this.cmd(CMD.BACKUP_GET, args);
      let i=0;
      if(r[i++] !== id) throw new Error("BACKUP_GET: id mismatch");
      const rc = r[i++]; if(rc !== 0) throw new Error("BACKUP_GET rc="+rc);
      const gotOff = u32from7(r.slice(i,i+5)); i += 5;
      const gotCount = r[i++] | (r[i++] << 7);
      const wire = r.slice(i);
      const data = unpack7(wire, gotCount);
      if(data.length !== gotCount) throw new Error("BACKUP_GET: short data");
      chunks.push(Uint8Array.from(data));
      offset += gotCount;
      if(gotOff !== offset - gotCount) throw new Error("BACKUP_GET: offset mismatch");
    }
    let total = 0; chunks.forEach(c=> total += c.length);
    const buf = new Uint8Array(total);
    let o = 0; chunks.forEach(c=>{ buf.set(c, o); o += c.length; });
    if(expectCrc !== undefined && crc32(buf) !== expectCrc) throw new Error("BACKUP_GET: CRC mismatch");
    return buf;
  };
  /* write one object; safe: device validates + commits atomically */
  Session.prototype.backupPut = async function(id, buf, crcVal){
    const crc = crcVal !== undefined ? crcVal : crc32(buf);
    // begin
    let r = await this.cmd(CMD.BACKUP_PUT, [0, id].concat(u32to7(buf.length), u32to7(crc)));
    if(r[1] !== 0) throw new Error("BACKUP_PUT begin rc="+r[1]);
    // data pieces: at most 256 decoded bytes each, pack7
    for(let off=0; off<buf.length; off+=256){
      const slice = buf.slice(off, off+256);
      const chunkWire = pack7(slice);
      r = await this.cmd(CMD.BACKUP_PUT, [1, id].concat(u32to7(off), chunkWire));
      if(r[1] !== 0) throw new Error("BACKUP_PUT data rc="+r[1]);
    }
    // commit
    r = await this.cmd(CMD.BACKUP_PUT, [2, id]);
    if(r[1] !== 0) throw new Error("BACKUP_PUT commit rc="+r[1]+" (device rejected — nothing written)");
    return true;
  };

  /* connect: request MIDI access with sysex */
  async function connect(){
    if(!navigator.requestMIDIAccess) throw new Error("WebMIDI not supported in this browser");
    const access = await navigator.requestMIDIAccess({ sysex:true });
    let output=null, input=null;
    access.outputs.forEach(o => { if(!output) output=o; });
    access.inputs.forEach(i => { if(!input) input=i; });
    if(!output || !input) throw new Error("no FM-1 MIDI port found");
    const s = new Session();
    s.output = output; s.input = input;
    input.onmidimessage = (ev)=> s.onMessage(ev);
    s.ready = true;
    return s;
  }

  return { Session, connect, pack7, unpack7, u32to7, u32from7, crc32, CMD };
})();
