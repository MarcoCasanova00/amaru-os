// The generated install.html, run end to end in a DOM against a simulated FM-1.
//
// This is the regression test for the 0.4 web installer bug: the page used to
// *push* 512-byte SysEx blobs, while the FM-1 *pulls* the package with 0x30
// read requests. Here the simulated device asks for the image, every returned
// block is compared byte for byte, and the identity it reports at the end has
// to match the package.
//
//   node tests/installer.page.test.mjs     (from the repository root)
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
import { webcrypto } from 'node:crypto';

const html = fs.readFileSync('install.html', 'utf8');
const script = html.match(/<script type="module">\n([\s\S]*)<\/script>/)[1];
// --- a minimal DOM: the page only needs element ids, text, value and click ---
const els = new Map();
const el = (id) => {
  if (!els.has(id)) els.set(id, { id, textContent: '', value: '', className: '', title: '', disabled: false,
    scrollTop: 0, scrollHeight: 0, files: [], _h: {},
    addEventListener(t, fn) { this._h[t] = fn; },
    click() { if (this._h.click) return this._h.click({ target: this }); },
    classList: { _s: new Set(), add(c) { this._s.add(c); }, remove(c) { this._s.delete(c); }, toggle(c, on) { on === undefined ? (this._s.has(c) ? this._s.delete(c) : this._s.add(c)) : (on ? this._s.add(c) : this._s.delete(c)); }, contains(c) { return this._s.has(c); } } });
  return els.get(id);
};
const document = { getElementById: el };
const window = { addEventListener() {} };

// ---- the published package, as the page would fetch it ----
const pkg = new Uint8Array(fs.readFileSync('firmware/build/felucca.fwsc'));
const sha256hex = async (b) => Buffer.from(await webcrypto.subtle.digest('SHA-256', b)).toString('hex');
const fact = (m) => `data:text/javascript;base64,${Buffer.from(m).toString('base64')}`;

// ---- shared protocol helpers (same modules the page inlines) ----
const lib = ['firmware/web/fm1pkg.js', 'firmware/web/fm1ota.js'].map((f) =>
  fs.readFileSync(f, 'utf8').replace(/^export\s+/gm, '').replace(/^import .*?;\n/gm, '')).join('\n') +
  '\nexport { pack7, unpack7, logicalImage, parseRequest };\n';
const P = await import(fact(lib));
const image = P.logicalImage(pkg);

// ---- a simulated FM-1: identity, then "device asks, host answers" ----
class Device {
  constructor() {
    this.identity = 'FM-1_015';
    this.phase = 'idle';
    this.queue = [];
    this.want = null;
    this.reads = 0;
    this.bad = 0;
  }
  frame(body) {
    body = body.slice();
    body.push(~body.slice(6).reduce((a, x) => a + x, 0) & 255);
    return [0xF0, ...P.pack7(body), 0xF7];
  }
  reply(text) {                                   // cmd 0x11
    const b = [0, 0x59, 0x11, 0, 0, 0, ...Array.from({ length: 27 }, (_, i) => (i < text.length ? text.charCodeAt(i) : 0))];
    b.push(~b.slice(6).reduce((a, x) => a + x, 0) & 255);
    this.toHost([0xF0, ...P.pack7(b), 0xF7]);
  }
  request(addr, len) {
    const finish = addr === 0xE0000000 || addr === 0xF0000000;   // "success" handshake, no image data
    this.want = { addr, len, data: finish ? null : Array.from(image.subarray(addr, addr + len)) };
    this.toHost(this.frame([0, 0x59, 0x30, (len + 8) & 255, 0, 0, 0,
      addr & 255, (addr >>> 8) & 255, (addr >>> 16) & 255, (addr >>> 24) & 255, len & 255, (len >> 8) & 255, 0]));
  }
  next() {
    if (!this.queue.length) {
      this.want = null;
      if (this.phase === 'step1') { this.phase = 'wait-for-loader'; this.identity = 'ota-FM-1_1'; }
      if (this.phase === 'step2') { this.phase = 'done'; this.identity = 'FM-1_7100002'; }
      return;
    }
    const [addr, len] = this.queue.shift();
    this.request(addr, len);
  }
  start(phase, blocks) {
    this.phase = phase;
    this.queue = blocks;
    setTimeout(() => this.next(), 0);
  }
  handle(bytes) {
    const s = Array.from(bytes);
    if (s[0] !== 0xF0 || s[s.length - 1] !== 0xF7) return;
    const u = P.unpack7(s.slice(1, -1));
    if (u.length === 7 && u[1] === 0x59 && u[2] === 0x11) return this.reply(this.identity);
    if (s.length === 6 && s[1] === 0x22 && s[2] === 0x24 && s[3] === 0x35) {
      if (this.phase === 'idle') return this.start('step1', [...Array.from({ length: 74 }, (_, i) => [i * 512, 512]), [0xE0000000, 8]]);
      if (this.phase === 'wait-for-loader') {
        const blocks = [];
        for (let off = 0; off < image.length; off += 512) blocks.push([off, Math.min(512, image.length - off)]);
        blocks.push([0xF0000000, 8]);
        return this.start('step2', blocks);
      }
      return;
    }
    if (u[2] !== 0x30 || !this.want) return;
    const addr = (u[7] | (u[8] << 8) | (u[9] << 16) | (u[10] << 24)) >>> 0;
    const len = u[11] | (u[12] << 8) | (u[13] << 16);
    if (addr !== this.want.addr || len !== this.want.len) { this.bad++; return; }
    if (this.want.data) {
      const data = u.slice(14, 14 + len);
      for (let i = 0; i < len; i++) if (data[i] !== this.want.data[i]) { this.bad++; break; }
    }
    this.reads++;
    if (addr === 0xE0000000 || addr === 0xF0000000) { this.phase = addr === 0xE0000000 ? 'wait-for-loader' : 'done'; this.identity = addr === 0xE0000000 ? 'ota-FM-1_1' : 'FM-1_7100002'; this.want = null; return; }
    setTimeout(() => this.next(), 0);
  }
  attach(port) { this.port = port; }
  toHost(data) { const h = this.port.onmidimessage; setTimeout(() => { if (h) h({ data }); }, 0); }
}

const dev = new Device();
const input = { state: 'connected', name: 'FM-1 MIDI 1', onmidimessage: null, async open() {} };
const output = { state: 'connected', name: 'FM-1 MIDI 1', async open() {}, send(bytes) { dev.handle(bytes); return true; } };
dev.attach(input);
const access = { inputs: new Map([[0, input]]), outputs: new Map([[0, output]]) };

// ---- run the page ----
const timers = [];
const sandbox = {
  document, window, console,
  crypto: webcrypto,
  TextEncoder,
  navigator: { requestMIDIAccess: async () => access },
  fetch: async (url) => ({ ok: url === 'firmware/build/felucca.fwsc', status: 200,
                           arrayBuffer: async () => pkg.buffer.slice(pkg.byteOffset, pkg.byteOffset + pkg.byteLength) }),
  confirm: () => true,
  alert: () => {},
  setTimeout: (fn, ms, ...a) => { const t = setTimeout(fn, Math.min(ms || 0, 5), ...a); timers.push(t); return t; },
  clearTimeout,
  URL, Blob, Promise, Date, Math, JSON, Error, String, Number, Array, Object, Uint8Array, DataView, Map, Set,
};
sandbox.globalThis = sandbox;
vm.createContext(sandbox);
await vm.runInContext(`(async () => {\n${script}\n})()`, sandbox, { filename: 'install.html' });
await new Promise((r) => setTimeout(r, 30));

const $ = (id) => document.getElementById(id);
assert.match($("verdict").textContent, /Verified/, "package verified from the published URL");
assert.equal($("pkg-ident").textContent, 'FM-1_7100002');
assert.equal($("pkg-size").textContent, '610,090 B');
assert.equal($("go").disabled, false, "Install enabled only after the pinned hash matched");
console.log('ok  page loads the published package and unlocks Install');

$("go").click();
const end = Date.now() + 20000;
let sawWrite = false, warnArmed = false;
while (Date.now() < end && !/Done\./.test($("status").textContent)) {
  if (/Writing — DO NOT UNPLUG/.test($("status").textContent)) sawWrite = true;
  if ($("warn").classList.contains("live")) warnArmed = true;
  await new Promise((r) => setTimeout(r, 20));
}
assert.ok(sawWrite, 'the status line says DO NOT UNPLUG while writing');
assert.ok(warnArmed, 'the banner is armed (pulsing) while writing');
assert.equal($("warn").classList.contains("live"), false, 'banner disarms when finished');
assert.match($("status").textContent, /You can unplug now/);
assert.match($("status").textContent, /Done\. The FM-1 reports FM-1_7100002/, "install finished");
assert.equal($("bar").value, 100);
assert.equal(dev.bad, 0, "every block the device asked for arrived byte-correct");
assert.equal(dev.reads, 74 + Math.ceil(image.length / 512) + 2, "74 verify reads + 1192 write reads + 2 success ACKs");
assert.equal(dev.identity, 'FM-1_7100002');
assert.equal($("go").disabled, false, "page unlocked again");
console.log(`ok  full install through the page: ${dev.reads} reads, 0 corrupted, device now ${dev.identity}`);
console.log('PASS: install.html end-to-end in a DOM with a simulated FM-1 (not hardware).');
