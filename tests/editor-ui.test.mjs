// SPDX-License-Identifier: GPL-3.0-only
// DOM interaction tests. Not a browser layout or physical USB test.
import {createRequire} from 'node:module';import {readFileSync} from 'node:fs';import assert from 'node:assert/strict';
const require=createRequire(import.meta.url),{parseHTML}=require('linkedom');
const {window,document}=parseHTML(readFileSync('dist/editor/index.html','utf8'));
// linkedom omits the browser's select.value setter.
Object.defineProperty(window.HTMLSelectElement.prototype,'value',{get(){return this.querySelector('option[selected]')?.getAttribute('value')??this.querySelector('option')?.getAttribute('value')??'';},set(v){for(const o of this.querySelectorAll('option'))o.toggleAttribute('selected',o.getAttribute('value')===String(v));},configurable:true});
globalThis.document=document;globalThis.window=window;globalThis.fetch=async()=>({ok:true,json:async()=>JSON.parse(readFileSync('dist/editor/data.json','utf8'))});
const errors=[];const savedError=console.error;console.error=e=>errors.push(e);
await import('../dist/editor/app.mjs');const $=id=>document.getElementById(id);const wait=async()=>{for(let i=0;i<1000;i++){if(!$('work').disabled)return;await new Promise(r=>setTimeout(r,2));}throw Error('UI did not complete');};
assert.equal($('scale').options.length,32);assert.equal($('tracks').children.length,4);assert.equal($('sound').options.length,16);assert.equal($('motif').options.length,24);assert.match($('status').textContent,/Offline/);assert.equal($('work').disabled,false);
$('root').value='3';$('scale').value='30';$('style').value='2';$('grow').click();await wait();assert.equal($('root').value,'3');assert.equal($('scale').value,'30');assert.equal($('undo').disabled,false);assert.match($('status').textContent,/New phrase/);
$('undo').click();await wait();assert.equal($('root').value,'2');assert.equal($('scale').value,'3');
$('tracks').children[2].click();assert.match($('trackNum').textContent,/ROOT/);$('sound').value='15';$('applySound').click();await wait();assert.match($('soundName').textContent,/Quiet Gravity/);
$('motif').value='13';$('density').value='9';$('applyMotif').click();await wait();assert.match($('status').textContent,/Motif/);
$('step-notes').value='C4 E4 G4';$('stepEditor').querySelector('button').click();await wait();assert.match($('grid').children[0].getAttribute('aria-label'),/C4 E4 G4/);
$('next').click();assert.match($('stepPage').textContent,/17–32/);$('prev').click();
const seedBefore=$('seed').value;$('seed').value='0';$('grow').click();await wait();assert.match($('status').textContent,/Seed must/);$('seed').value=seedBefore;
assert.equal(errors.length,1);console.error=savedError;console.log('PASS: editor DOM boots; tracks, 32 scales, GROW, undo, sound recipes, motifs, chord edits, pagination and validation. No browser layout verification.');
