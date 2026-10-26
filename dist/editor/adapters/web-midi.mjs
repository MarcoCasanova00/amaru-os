// SPDX-License-Identifier: GPL-3.0-only
// Replace this adapter with CoreMIDI / a native bridge in a desktop or mobile shell.
export async function ports(){if(!globalThis.isSecureContext||!navigator.requestMIDIAccess)throw Error('Use desktop Google Chrome on this HTTPS site for USB editing. Offline editing remains available.');const access=await navigator.requestMIDIAccess({sysex:true});return {access,inputs:[...access.inputs.values()].filter(p=>p.state==='connected'),outputs:[...access.outputs.values()].filter(p=>p.state==='connected')};}
export async function openMidi(access,inputId,outputId){
 if(!navigator.locks)throw Error('Use current desktop Chrome (Web Locks required).');
 let release,resolveLock,rejectLock;const acquired=new Promise((r,j)=>{resolveLock=r;rejectLock=j;});
 const held=navigator.locks.request('bunker-evolved-fm1-update',{ifAvailable:true},async lock=>{if(!lock){rejectLock(Error('Another editor or installer from this site is connected. Disconnect it first.'));return;}await new Promise(r=>{release=r;resolveLock();});});held.catch(rejectLock);await acquired;
 const input=access.inputs.get(inputId),output=access.outputs.get(outputId);
 let closed=false;
 const transport={onData:null,onClose:null,send(data){if(closed||input.state!=='connected'||output.state!=='connected')throw Error('Selected FM-1 disconnected');output.send(data);},close(){if(closed)return;closed=true;input?.removeEventListener('midimessage',receive);access.removeEventListener('statechange',changed);input?.close().catch(()=>{});output?.close().catch(()=>{});release();}};
 const receive=e=>transport.onData?.(e.data),changed=()=>{if(input.state!=='connected'||output.state!=='connected'){transport.onClose?.();transport.close();}};
 try{if(!input||!output||input.state!=='connected'||output.state!=='connected')throw Error('Select a connected MIDI input and output');await input.open();await output.open();input.addEventListener('midimessage',receive);access.addEventListener('statechange',changed);return transport;}catch(e){transport.close();throw e;}
}
