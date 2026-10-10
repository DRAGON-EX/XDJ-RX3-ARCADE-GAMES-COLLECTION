/* Desktop inspection of the exact EUP/SUB bytes used by the RX3 show. */
export function cafeLedPreview(api,host){
 const style=document.createElement('style');style.textContent=`
 .rx3-led-preview{max-width:1000px;margin:22px auto 32px;padding:20px;border:1px solid #403142;border-radius:15px;background:#101016;color:#978b9c;font:10px monospace}
 .rx3-led-preview header{display:flex;justify-content:space-between;gap:12px;margin-bottom:20px;font-size:12px;letter-spacing:2px}.rx3-led-body{display:grid;grid-template-columns:1fr .7fr 1fr;gap:22px}.rx3-led-deck,.rx3-led-mixer{min-width:0}.rx3-led-bank{display:grid;grid-template-columns:repeat(4,1fr);gap:7px;margin:10px 0}.rx3-led-bank.buttons{grid-template-columns:repeat(3,1fr)}
 .rx3-led{--led:#24202c;display:flex;align-items:center;justify-content:center;min-height:27px;border:2px solid var(--led);border-radius:5px;color:#afa5b8;box-shadow:inset 0 0 13px color-mix(in srgb,var(--led) 30%,transparent);transition:color .05s;text-align:center;font-size:9px}
 .rx3-led.pad{height:38px;background:color-mix(in srgb,var(--led) 42%,#101016);box-shadow:0 0 10px color-mix(in srgb,var(--led) 35%,transparent)}.rx3-led-ring{height:72px;width:72px;border:7px solid #292530;border-radius:50%;margin:14px auto;background:radial-gradient(circle,#29222e 0 25%,#15121b 27% 100%)}
 .rx3-led-mixer .rx3-led-bank{grid-template-columns:repeat(2,1fr)}.rx3-led-preview footer{margin-top:14px;line-height:1.6;color:#827789}@media(max-width:600px){.rx3-led-body{gap:7px}.rx3-led-preview{padding:10px}.rx3-led{font-size:7px}.rx3-led-bank{gap:3px}}
 `;document.head.append(style);host.className='rx3-led-preview';
 const header=document.createElement('header'),name=document.createElement('span'),status=document.createElement('span');name.textContent='RX3 LIGHTSHOW';status.textContent='CLICK THE RECORD';header.append(name,status);host.append(header);
 const body=document.createElement('div');body.className='rx3-led-body';host.append(body);const lights=[];
 function group(parent,names,kind,decode){const bank=document.createElement('div');bank.className='rx3-led-bank '+kind;parent.append(bank);names.forEach((label,i)=>{const el=document.createElement('span');el.className='rx3-led '+(kind==='pads'?'pad':'');el.textContent=label;bank.append(el);lights.push({el,decode:p=>decode(p,i)});});}
 const rgb=(p,at)=>`rgb(${Math.min(255,p[at+3]*255/26)},${Math.min(255,p[at+2]*255/28)},${Math.min(255,p[at+1]*255/60)})`;
 const mono=(p,at,bit,color)=>p[at]&bit?color:'#24202c';
 function deck(d){const el=document.createElement('div');el.className='rx3-led-deck';body.append(el);const title=document.createElement('div');title.textContent=d?'DECK 2':'DECK 1';el.append(title);
  const offsets=[[1,128],[1,64],[1,32],[1,4],[2,8],[3,16],[3,8],[3,1]];
  group(el,['SLIP','SYNC','MASTER','QUANTIZE','VINYL','RELOOP','M.TEMPO','REVERSE'],'buttons',(p,i)=>mono(p,offsets[i][0]+d*4,offsets[i][1],i<4?'#f9b446':'#65bffa'));
  const ring=document.createElement('div');ring.className='rx3-led-ring';el.append(ring);
  group(el,['IN','OUT','CUE','PLAY'],'transport',(p,i)=>mono(p,100+(d?98:99),128>>i,i===3?'#77e294':'#ffb044'));
  group(el,['HOT CUE','BEAT LOOP','SLIP LOOP','BEAT JUMP'],'modes',(p,i)=>rgb(p,100+d*48+i*4));
  group(el,['A','B','C','D','E','F','G','H'],'pads',(p,i)=>rgb(p,100+d*48+16+i*4));
 }
 deck(0);const mixer=document.createElement('div');mixer.className='rx3-led-mixer';body.append(mixer);const title=document.createElement('div');title.textContent='MIXER / FX';mixer.append(title);
 group(mixer,['FILTER','SWEEP','DUB ECHO','NOISE','CRUSH','SPACE','BEAT FX','TAP'],'fx',(p,i)=>mono(p,19,1<<i,i<6?'#52c9ff':'#75ecdc'));
 group(mixer,['CH1 CUE','CH2 CUE','MASTER CUE'],'cue',(p,i)=>mono(p,10,32>>i,'#f7a349'));deck(1);
 const footer=document.createElement('footer');footer.textContent='Preview of the programmed RX3 button lights. Actual panel colours and brightness may differ.';host.append(footer);
 let last=-100;
 return (state,now)=>{if(now-last<50)return;last=now;const p=new Uint8Array(api.memory.buffer,api.cats_leds(state.ms,state.active?1:0),204);lights.forEach(({el,decode})=>el.style.setProperty('--led',decode(p)));status.textContent=state.active?`GOOD · ${Math.floor(state.ms/60000)}:${String(Math.floor(state.ms/1000)%60).padStart(2,'0')}`:'CLICK THE RECORD';};
}
