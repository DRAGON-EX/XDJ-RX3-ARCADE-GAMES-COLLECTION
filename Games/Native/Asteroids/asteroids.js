// RX3_AUDIO_HOOK: playlist stays in the parent audio host, never on disk.
let rx3Effects;
const rx3Bus=()=>rx3Effects||(rx3Effects=window.RX3Audio.effects(audio));
for(const event of ['pointerdown','keydown'])window.addEventListener(event,()=>window.RX3Audio?.start());
window.RX3Audio?.start();
const canvas=document.querySelector('#game'),ctx=canvas.getContext('2d'),status=document.querySelector('#status');
const theme=new URLSearchParams(location.search).get('theme')==='neon'?1:0;
let game,image,raf,last=0,accumulator=0,lastDraw=0,held=0,paused=false,audio=null,audioEnabled=false,audioNext=0,volume=65,disposed=false;
const sources=new Set();
function clearAudio(){for(const source of sources)try{source.stop()}catch{}sources.clear();audioNext=0;}
function stop(){disposed=true;cancelAnimationFrame(raf);held=0;clearAudio();audio?.close();game=image=null;}
function menu(){if(game)window.RX3Session.score('asteroids',game.game_value(6));stop();window.RX3Session.navigate(new URL('../../../launcher-preview.html?v=session-1',location.href).href);}
function pause(){if(game?.game_value(8))return;paused=!paused;held=0;clearAudio();last=0;}
function reset(){if(game?.game_value(8))return;game.game_reset(0x5833+Math.floor(performance.now()));paused=false;held=0;clearAudio();}
function playSound(){const pointer=game.game_sound(audio&&audio.state==='running'?100:0);if(!audio||audio.state!=='running')return;
 if(audioNext<audio.currentTime)audioNext=audio.currentTime+.025;if(audioNext>audio.currentTime+.12)return;
 const pcm=new Int16Array(game.memory.buffer,pointer,1470),buffer=audio.createBuffer(2,735,44100);
 for(let ch=0;ch<2;ch++){const data=buffer.getChannelData(ch);for(let i=0;i<735;i++)data[i]=pcm[2*i+ch]/32768;}
 const source=audio.createBufferSource();source.buffer=buffer;source.connect(rx3Bus());source.onended=()=>sources.delete(source);sources.add(source);source.start(audioNext);audioNext+=735/44100;
}
function frame(now){if(disposed)return;let delta=last?Math.min(100,now-last):0;last=now;if(!paused&&!document.hidden){accumulator+=delta;while(accumulator>=1000/60){game.game_tick(held);const rescued=window.RX3Session.reportProgress('asteroids',game.game_value(7));if(rescued>=0){clearAudio();held=0;game.game_rescue_begin(rescued);}playSound();accumulator-=1000/60;}}
 if(now-lastDraw>=1000/30){window.RX3Session.score('asteroids',game.game_value(6));game.game_frame(theme,paused?1:0);ctx.putImageData(image,0,0);lastDraw=now;document.querySelector('#hud').textContent=`Score ${game.game_value(0)}, wave ${game.game_value(1)}, lives ${game.game_value(2)}`;}
 raf=requestAnimationFrame(frame);
}
const keys={ArrowUp:1,w:1,W:1,ArrowDown:2,s:2,S:2,b:2,B:2,ArrowLeft:4,a:4,A:4,ArrowRight:8,d:8,D:8,' ':16};
window.addEventListener('keydown',e=>{if(game?.game_value(8)){e.preventDefault();return;}if(e.target instanceof HTMLInputElement)return;if(keys[e.key]){e.preventDefault();held|=keys[e.key];}else if(!e.repeat){if(e.key==='Escape')menu();else if(e.key==='p'||e.key==='P')pause();else if(e.key==='r'||e.key==='R')reset();}});
window.addEventListener('keyup',e=>{if(keys[e.key]){e.preventDefault();held&=~keys[e.key];}});
window.addEventListener('blur',()=>{held=0;});
document.addEventListener('visibilitychange',()=>{last=0;accumulator=0;held=0;if(document.hidden){clearAudio();audio?.suspend();}else if(audio&&audioEnabled)audio.resume().catch(()=>{});});
canvas.addEventListener('pointerdown',e=>{if(game?.game_value(8))return;const box=canvas.getBoundingClientRect(),x=(e.clientX-box.left)*1280/box.width,y=(e.clientY-box.top)*800/box.height;if(x>=30&&x<300&&y>=540&&y<596)menu();else if(x>=30&&x<155&&y>=640&&y<676)pause();else if(x>=175&&x<300&&y>=640&&y<676)reset();else if(x<300&&y>=450&&y<488)held|=16;else if(x<300&&y>=488&&y<528)held|=2;canvas.setPointerCapture(e.pointerId);});
canvas.addEventListener('pointerup',()=>held=0);canvas.addEventListener('pointercancel',()=>held=0);
for(const b of document.querySelectorAll('[data-key]')){const mask=Number(b.dataset.key);b.onpointerdown=e=>{if(game?.game_value(8))return;e.preventDefault();b.setPointerCapture(e.pointerId);held|=mask;};b.onpointerup=b.onpointercancel=()=>held&=~mask;}
document.querySelector('#volume').oninput=e=>{volume=Number(e.target.value);window.RX3Audio.setEffectsLevel(volume/100);};
document.querySelector('#sound').onclick=async e=>{if(!audio)audio=new AudioContext();audioEnabled=!audioEnabled;if(audioEnabled){await audio.resume();e.target.textContent='SOUND: ON';}else{clearAudio();await audio.suspend();e.target.textContent='SOUND: OFF';}};
window.addEventListener('pagehide',stop,{once:true});
try{const response=await fetch('asteroids.wasm?v=rescue-audio-1');if(!response.ok)throw Error('Game module not available');const result=await WebAssembly.instantiate(await response.arrayBuffer(),{});game=result.instance.exports;game.game_reset(0x5833);game.game_set_best(window.RX3Session.best('asteroids'));image=new ImageData(new Uint8ClampedArray(game.memory.buffer,game.game_buffer(),1280*800*4),1280,800);status.textContent='ARROWS / WASD: TURN + THRUST · SPACE: FIRE · DOWN / B: HYPERSPACE · P: PAUSE · R: RESTART · ESC: MENU';raf=requestAnimationFrame(frame);}catch(e){status.textContent='BASS ROCKS COULD NOT LOAD: '+e.message;}
