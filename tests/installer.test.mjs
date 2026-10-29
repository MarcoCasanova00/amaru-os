// The published AMARU installer/verifier pages, tested against the real package.
//
// The firmware installs by *pulling*: the device sends 0x30 read requests and the
// host answers with the package image. This test decodes the frames the generated
// page would send and compares them with the released .fwsc, so a page that
// pushes raw data (the 0.4 web installer bug) cannot pass again.
//
//   node tests/installer.test.mjs        (from the repository root)
import assert from 'node:assert/strict';
import fs from 'node:fs';

const read = (f) => fs.readFileSync(f, 'utf8');
const strip = (f) => read(f).replace(/^export\s+/gm, '').replace(/^import .*?;\n/gm, '');

const lib = strip('firmware/web/fm1pkg.js') + '\n' + strip('firmware/web/fm1ota.js') +
  '\nexport { Updater, pack7, unpack7, parseIdentity, productOf, logicalImage, sha256hex, IS_OTA };\n';
const M = await import('data:text/javascript;base64,' + Buffer.from(lib).toString('base64'));

const html = read('install.html');
const meta = JSON.parse(html.match(/const meta = (\{.*?\});/s)[1]);
const pkg = new Uint8Array(fs.readFileSync('firmware/build/felucca.fwsc'));
const image = M.logicalImage(pkg);

// --- the page ships the tested build, and only that build ---
const sha = await M.sha256hex(pkg);
assert.equal(meta.product, M.productOf(pkg));
assert.equal(meta.product, 'FM-1_7100002');
assert.equal(meta.size, 610090);
assert.equal(meta.sha256, sha);
assert.equal(meta.pkg, 'firmware/build/felucca.fwsc');
assert.ok(html.includes('firmware/web') === false, 'the page is self-contained');
assert.ok(html.includes('new Updater('), 'the page uses the shared updater');
assert.ok(!/0xF0,\s*0x30,\s*\.\.\.slice/.test(html), 'the page must not push data frames');

const vhtml = read('verify.html');
const vmeta = JSON.parse(vhtml.match(/const meta = (\{.*?\});/s)[1]);
assert.deepEqual(vmeta, meta);
console.log('ok  install.html + verify.html pin', meta.product, meta.size, sha.slice(0, 12) + '…');

// --- the image the device addresses ---
const manual = new Uint8Array(pkg.length - 20);
for (let i = 0; i < 20; i++) manual.set(pkg.subarray(i * 0x30, i * 0x30 + 0x2F), i * 0x2F);
manual.set(pkg.subarray(20 * 0x30), 20 * 0x2F);
assert.deepEqual(image, manual, 'image = package without the 20 identity marker bytes');

// --- serve(): answer 0x30 read requests, decode what went back to the device ---
const request = (addr, len, fl = 0) => {
  const b = [0, 0x59, 0x30, (len + 8) & 255, 0, 0, fl,
    addr & 255, (addr >>> 8) & 255, (addr >>> 16) & 255, (addr >>> 24) & 255,
    len & 255, (len >> 8) & 255, 0];
  b.push(~b.slice(6).reduce((a, x) => a + x, 0) & 255);
  return [0xF0, ...M.pack7(b), 0xF7];
};
const decode = (f) => {
  const body = M.unpack7(Array.from(f).slice(1, -1));
  const len = body[11] | (body[12] << 8) | (body[13] << 16);
  return { addr: (body[7] | (body[8] << 8) | (body[9] << 16) | (body[10] << 24)) >>> 0,
           data: body.slice(14, 14 + len) };
};

const packets = [];
for (let off = 0; off < image.length; off += 512) packets.push(request(off, Math.min(512, image.length - off)));
packets.push(request(0xE0000000, 6));
const link = { lost: false, closed: false, out: [], async read() { return packets.shift() || null; },
               send(f) { this.out.push(f); return true; }, close() { this.closed = true; } };
let last = 0;
const r = await new M.Updater({}, () => {}).serve(link, image, 0xE0000000, 50, (n) => { last = n; });
assert.equal(r.finished, true);
assert.equal(r.served, Math.ceil(image.length / 512));
assert.equal(last, r.served);
for (let i = 0; i < r.served; i++) {
  const d = decode(link.out[i]); const off = i * 512, n = Math.min(512, image.length - off);
  assert.equal(d.addr, off);
  assert.equal(d.data.length, n);
  assert.deepEqual(Uint8Array.from(d.data), image.subarray(off, off + n));
}
console.log(`ok  ${r.served} blocks served byte-for-byte from the 610,070 B image (0xE0000000 finish ack)`);

// --- install(): orchestration, identity gate, cleanup ---
const quiet = { lost: false, async read() { return null; }, send() { return true; }, close() {} };
const fake = (from, back) => {
  const u = new M.Updater({});
  u.find = async () => ({ id: { model: 'FM-1', text: from }, link: quiet });
  u.serve = async () => ({ served: 74, finished: true, lost: false });
  const replies = [{ model: 'ota-FM-1', text: 'ota-FM-1_1' }, { model: 'FM-1', text: back }];
  u.waitFor = async (f) => { for (const id of replies) if (f(id)) return { id, link: quiet }; return null; };
  return u;
};
const stages = [];
assert.equal(await (await fake('FM-1_015', 'FM-1_7100002')).install(image, meta.product, (k) => stages.push(k)),
             'FM-1_7100002', 'V15 -> AMARU 0.4 is the supported route');
assert.equal(stages[0], 'start');
assert.ok(['loader', 'done'].every((k) => stages.includes(k)));
await assert.rejects((await fake('FM-1_7100002', 'FM-1_7010200')).install(image, meta.product),
                     (e) => e.code === 'mismatch');
console.log('ok  install(): V15 and AMARU accepted; a device reporting another build is refused');

// --- find(): named ports, and the loose fallback for odd driver names ---
const idFrame = (text) => {
  const b = [0, 0x59, 0x11, 0, 0, 0, ...Array.from({ length: 27 }, (_, i) => (i < text.length ? text.charCodeAt(i) : 0))];
  b.push(~b.slice(6).reduce((a, x) => a + x, 0) & 255);
  return [0xF0, ...M.pack7(b), 0xF7];
};
const port = (name, reply) => ({ name, reply, state: 'connected', opened: 0, async open() { this.opened++; }, onmidimessage: null });
const access = (ports) => ({
  inputs: new Map(ports.map((p, i) => [i, Object.assign(port(p.in, p.reply), {})])),
  outputs: new Map(ports.map((p, i) => [i, Object.assign(port(p.out, null), { sent: [], send(b) { this.sent.push(b); } })])),
});
const wire = (a) => {
  const ins = [...a.inputs.values()], outs = [...a.outputs.values()];
  ins.forEach((i, k) => {
    const out = outs[k]; if (!out) return;
    out.send = ((orig) => (bytes) => {
      orig.call(out, bytes);
      if (i.reply) setTimeout(() => i.onmidimessage({ data: i.reply }), 0);
      return true;
    })(out.send);
  });
};
const fm = idFrame('FM-1_7100002');
const named = access([{ in: 'FM-1 MIDI 1', out: 'FM-1 MIDI 1', reply: fm }]); wire(named);
assert.ok(await new M.Updater(named).find((id) => !M.IS_OTA(id)));
const mixed = access([{ in: 'Launchpad', out: 'Launchpad', reply: null },
                      { in: 'M-VAVE Composite', out: 'M-VAVE Composite', reply: fm }]); wire(mixed);
assert.ok(await new M.Updater(mixed).find((id) => !M.IS_OTA(id)));
const split = access([{ in: 'Device Port A', out: 'Device Port B', reply: fm }]); wire(split);
assert.equal(await new M.Updater(split).find((id) => !M.IS_OTA(id)), null, 'no probing unless asked');
assert.ok(await new M.Updater(split).find((id) => !M.IS_OTA(id), true), 'probeAll pairs split in/out names');
const silent = access([{ in: 'Random Synth', out: 'Random Synth', reply: null }]); wire(silent);
assert.equal(await new M.Updater(silent).find((id) => !M.IS_OTA(id), true), null, 'silent gear is never selected');
console.log('ok  find(): named port, named among others, loose pairing, silent gear rejected');

console.log('PASS: installer/verifier pages + M-UPGRADE protocol (simulated device, not hardware).');
