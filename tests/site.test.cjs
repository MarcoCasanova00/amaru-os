const fs = require('node:fs'), path = require('node:path'), vm = require('node:vm');
const assert = require('node:assert/strict');
const {parseHTML} = require('linkedom');
(async () => {
  const root = path.resolve('dist');
  for (const name of ['index.html', 'comparison.html', 'editor-architecture.html', 'editor/index.html', 'manual.html', 'research.html', 'rescue.html', 'release.html', 'install/index.html', 'simulator/index.html']) {
    const {document} = parseHTML(fs.readFileSync(path.join(root,name),'utf8'));
    for (const el of document.querySelectorAll('[href],[src]')) {
      const ref = el.getAttribute('href') || el.getAttribute('src');
      if (!ref || /^(https?:|data:|#)/.test(ref)) continue;
      const p = path.resolve(path.dirname(path.join(root,name)),ref.split('#')[0]);
      assert(p.startsWith(root + '/') || p === root, `${name}: outside distribution: ${ref}`);
      assert(fs.existsSync(p), `${name}: missing ${ref}`);
    }
  }
  const {window,document} = parseHTML(fs.readFileSync('dist/index.html','utf8'));
  const bank = JSON.parse(fs.readFileSync('dist/library.json','utf8'));
  let audio;
  class FakeAudio {
    constructor() { audio=this; this.events={}; this.pauses=0; this.plays=0; }
    addEventListener(k,f) {this.events[k]=f;}
    pause() {this.pauses++;}
    async play() {this.plays++;}
  }
  const c = vm.createContext({document,window,Audio:FakeAudio,console,fetch:async()=>({ok:true,json:async()=>bank})});
  await vm.runInContext('(async()=>{'+fs.readFileSync('dist/app.js','utf8')+'})()',c);
  assert.equal(document.querySelectorAll('.world').length,24);
  for (const b of document.querySelectorAll('[data-role]')) {
    b.onclick(); assert.equal(document.querySelectorAll('.sound').length,16);
    const role=Number(b.dataset.role);
    for (let i=0;i<16;i++) assert(document.querySelectorAll('.sound')[i].textContent.includes(bank.banks[role][i][0]));
  }
  const play = [...document.querySelectorAll('.world button')];
  await play[0].onclick(); assert.equal(audio.src,'audio/world-00.mp3'); assert.equal(audio.plays,1);
  await play[23].onclick(); assert.equal(audio.src,'audio/world-23.mp3'); assert.equal(audio.plays,2);
  assert.equal(document.querySelectorAll('.playing').length,1);
  await play[23].onclick(); assert.equal(document.querySelectorAll('.playing').length,0);
  audio.events.error(); assert(document.getElementById('audioStatus').textContent.includes('could not be loaded'));
  for (let i=0;i<24;i++) assert(fs.statSync(`dist/audio/world-${String(i).padStart(2,'0')}.mp3`).size>10000);
  console.log('PASS site DOM/data: local links resolve; 24 worlds, all 64 named sounds, exclusive preview play/pause, audio error handling, 24 audio files. No browser visual QA.');
})().catch(e=>{console.error(e);process.exit(1)});
