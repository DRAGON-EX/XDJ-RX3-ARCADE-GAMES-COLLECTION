// RX3_AUDIO_HOOK: playlist stays in the parent audio host, never on disk.
let rx3Effects;
const rx3Bus=()=>rx3Effects||(rx3Effects=window.RX3Audio.effects(audio));
for(const event of ['pointerdown','keydown'])window.addEventListener(event,()=>window.RX3Audio?.start());
window.RX3Audio?.start();
const canvas=document.querySelector('#game'),ctx=canvas.getContext('2d'),status=document.querySelector('#status');
const theme=new URLSearchParams(location.search).get('theme')==='neon'?1:0;
let game,image,raf,last=0,accumulator=0,lastDraw=0,held=0,pending=0,paused=false,audio=null,audioEnabled=false,audioNext=0,volume=65,disposed=false;
const sources=new Set();
function clearAudio(){for(const source of sources)try{source.stop()}catch{}sources.clear();audioNext=0;}
function stop(){disposed=true;cancelAnimationFrame(raf);held=pending=0;clearAudio();audio?.close();game=image=null;}
function menu(){if(game)window.RX3Session.score('basslines',game.game_value(6));stop();window.RX3Session.navigate(new URL('../../../launcher-preview.html?v=session-1',location.href).href);}
function pause(){if(game?.game_value(8))return;if(!game)return;paused=!paused;held=pending=0;clearAudio();last=0;accumulator=0;}
function reset(){if(game?.game_value(8))return;if(!game)return;game.game_reset(2401);paused=false;held=pending=0;clearAudio();}
function sound(){const p=game.game_sound(audioEnabled?100:0);if(!audio||audio.state!=='running')return;if(audioNext<audio.currentTime)audioNext=audio.currentTime+.025;if(audioNext>audio.currentTime+.12)return;
 const samples=new Int16Array(game.memory.buffer,p,1600),buffer=audio.createBuffer(2,800,48000);
 for(let ch=0;ch<2;ch++){const data=buffer.getChannelData(ch);for(let i=0;i<800;i++)data[i]=samples[i*2+ch]/32768;}
 const src=audio.createBufferSource();src.buffer=buffer;src.connect(rx3Bus());src.onended=()=>sources.delete(src);sources.add(src);src.start(audioNext);audioNext+=800/48000;
}
function frame(now){if(disposed)return;const dt=last?Math.min(100,now-last):0;last=now;if(!paused&&!document.hidden){accumulator+=dt;while(accumulator>=1000/60){game.game_tick(held|pending);const rescued=window.RX3Session.reportProgress('basslines',game.game_value(7));if(rescued>=0){clearAudio();held=0;game.game_rescue_begin(rescued);}pending=0;sound();accumulator-=1000/60;}}
 if(now-lastDraw>=1000/30){window.RX3Session.score('basslines',game.game_value(6));game.game_frame(theme,paused?1:0);ctx.putImageData(image,0,0);lastDraw=now;document.querySelector('#hud').textContent='Round '+game.game_value(1)+', score '+game.game_value(0)+', boost '+Math.round(game.game_value(3)*100/240)+'%';}
 raf=requestAnimationFrame(frame);
}
const keys={ArrowUp:1,w:1,W:1,ArrowDown:2,s:2,S:2,ArrowLeft:4,a:4,A:4,ArrowRight:8,d:8,D:8,' ':16,Enter:16};
window.addEventListener('keydown',e=>{if(game?.game_value(8)){e.preventDefault();return;}if(e.target instanceof HTMLInputElement||e.target instanceof HTMLSelectElement)return;if(keys[e.key]){e.preventDefault();held|=keys[e.key];if(!e.repeat)pending|=keys[e.key];}else if(!e.repeat){if(e.key==='Escape')menu();else if(e.key.toLowerCase()==='p')pause();else if(e.key.toLowerCase()==='r')reset();}});
window.addEventListener('keyup',e=>{if(keys[e.key]){e.preventDefault();held&=~keys[e.key];}});
window.addEventListener('blur',()=>{held=pending=0;});
document.addEventListener('visibilitychange',()=>{last=0;accumulator=0;held=pending=0;if(document.hidden){clearAudio();audio?.suspend();}else if(audio&&audioEnabled)audio.resume().catch(()=>{});});
function coords(e){const r=canvas.getBoundingClientRect();return [(e.clientX-r.left)*1280/r.width,(e.clientY-r.top)*800/r.height];}
function touchKey(x,y){
 if(y>=560&&y<598&&x>=126&&x<210)return 1;
 if(y>=656&&y<694&&x>=126&&x<210)return 2;
 if(y>=608&&y<646){if(x>=26&&x<110)return 4;if(x>=226&&x<314)return 8;if(x>=126&&x<210)return 16;}
 return 0;
}
canvas.addEventListener('pointerdown',e=>{if(game?.game_value(8))return;if(!game)return;const [x,y]=coords(e);
 if(x>=26&&x<315&&y>=721&&y<768){menu();return;}
 if(x>=26&&x<315&&y>=480&&y<520){if(x<158)pause();else if(x>=177)reset();return;}
 const key=touchKey(x,y);if(key){held|=key;pending|=key;canvas.setPointerCapture(e.pointerId);}
});
canvas.addEventListener('pointerup',()=>held=0);canvas.addEventListener('pointercancel',()=>held=0);
for(const b of document.querySelectorAll('[data-key]')){const mask=Number(b.dataset.key);b.onpointerdown=e=>{if(game?.game_value(8))return;e.preventDefault();b.setPointerCapture(e.pointerId);held|=mask;pending|=mask;};b.onpointerup=b.onpointercancel=()=>held&=~mask;}
document.querySelector('#volume').oninput=e=>{volume=Number(e.target.value);window.RX3Audio.setEffectsLevel(volume/100);};
document.querySelector('#sound').onclick=async e=>{if(!audio)audio=new AudioContext();audioEnabled=!audioEnabled;if(audioEnabled){await audio.resume();e.target.textContent='SOUND: ON';}else{clearAudio();await audio.suspend();e.target.textContent='SOUND: OFF';}};
window.addEventListener('pagehide',stop,{once:true});
try{const response=await fetch('basslines.wasm?v=rescue-audio-1');if(!response.ok)throw Error('Game module not available');game=(await WebAssembly.instantiate(await response.arrayBuffer(),{})).instance.exports;game.game_reset(2401);game.game_set_best(window.RX3Session.best('basslines'));image=new ImageData(new Uint8ClampedArray(game.memory.buffer,game.game_buffer(),1280*800*4),1280,800);status.textContent='ARROWS / WASD / TOUCH: STEER · HOLD SPACE: BOOST · SPACE / ENTER: NEXT ROUND · P: PAUSE · R: RESTART · ESC: MENU';raf=requestAnimationFrame(frame);}catch(e){status.textContent='BASSLINES COULD NOT LOAD: '+e.message;}
