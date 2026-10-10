// DJ Wars owns an original score; keep the shared playlist paused at its position.
window.RX3Audio?.claimSoundtrack('dj-wars');
const canvas=document.querySelector('#game'),ctx=canvas.getContext('2d',{alpha:false}),status=document.querySelector('#status'),start=document.querySelector('#start');
const names=['DJ-DRAGONX','YUKI','PEACH','COOKIE','MOCHI','DEADBEAT 1','DEADBEAT 2','DEADBEAT 3','DEADBEAT 4','DECK BOSS'];
const params=new URLSearchParams(location.search),theme=params.get('theme')==='neon'?1:0;
const demo=['guetta','victory','play'].includes(params.get('demo'))?params.get('demo'):null;
if(demo)document.title=demo==='guetta'?'DJ Wars · Guetta Rescue':demo==='victory'?'DJ Wars · Victory Party':'DJ Wars · Play with Sound';
let game,image,raf,last=0,accumulator=0,lastDraw=0,held=0,pressed=0,paused=true,disposed=false,audio,sound=true,processor,splitter,fxBus,musicBus,volume=.45,drag=null,creditsTransition=false;
// Both buses use the exact same allocation-free C synthesizer as the RX3.
// Audio is enabled by the play gesture, and suspended with the game.
const fireworksSong='assets/music/alex-morgan-game-level-pixel-quest-loop-578505.mp3';
function syncFireworks(){if(!game||disposed)return;if(game.game_value(19))window.RX3Audio?.sceneMusic?.('dj-wars',fireworksSong,!paused&&sound&&!document.hidden);else window.RX3Audio?.stopSceneMusic?.('dj-wars');}
async function startSound(){
 if(!sound||paused||disposed)return;syncFireworks();
 if(!audio){
  audio=new AudioContext({sampleRate:44100});
  fxBus=window.RX3Audio.effects(audio);musicBus=window.RX3Audio.soundtrack(audio);
  processor=audio.createScriptProcessor(2048,0,2);splitter=audio.createChannelSplitter(2);
  processor.connect(splitter);splitter.connect(fxBus,0);splitter.connect(musicBus,1);
  processor.onaudioprocess=e=>{
   const effects=e.outputBuffer.getChannelData(0),score=e.outputBuffer.getChannelData(1);
   if(!game||paused||!sound||disposed){effects.fill(0);score.fill(0);return;}
   game.game_audio_render(effects.length,audio.sampleRate);
   const stems=new Float32Array(game.memory.buffer,game.game_audio_buffer(),4096);
   effects.set(stems.subarray(0,effects.length));if(game.game_value(19))score.fill(0);else score.set(stems.subarray(2048,2048+score.length));
  };
  window.RX3Audio.setEffectsLevel(volume);
 }
 await audio.resume();if(paused||!sound||disposed)await audio.suspend();
}
function stopSound(){if(audio&&audio.state!=='closed')audio.suspend();if(game?.game_value(19))window.RX3Audio?.sceneMusic?.('dj-wars',fireworksSong,false);}
function update(){syncFireworks();if(!demo&&game.game_value(0)===3&&game.game_value(11)===0){openCredits();return;}const state=game.game_value(0),team=game.game_value(1),active=game.game_value(2),remaining=Math.ceil(game.game_value(3)/60),turn=game.game_value(6),weapon=game.game_value(9);
 const text=state===3?(game.game_value(17)?'DAVID GUETTA SAVED THE CREW':'DRAGON CREW WINS · ALL CATS ALIVE'):state===4?(game.game_value(17)?(game.game_value(18)<90?'THE CREW IS DOWN…':'DAVID GUETTA'):'VICTORY · BRINGING THE CREW BACK'):paused?'PAUSED · ENTER THE CLUB':game.game_value(16)?'HIT · DAMAGE SUMMARY':`${team?'ENEMY':'YOUR'} TURN · ${names[active]} · ${remaining}s · ${game.game_value(7)} VS ${game.game_value(8)} · TURN ${turn}`;
 status.textContent=(demo?'PREVIEW · ':'')+text;document.querySelector('#hud').textContent=text+` · Dragon crew HP ${game.game_value(13)} · Mouse crew HP ${game.game_value(14)}`;
 start.textContent=paused?'ENTER / RESUME':'PAUSE';for(const b of document.querySelectorAll('[data-weapon]'))b.setAttribute('aria-pressed',Number(b.dataset.weapon)===weapon?'true':'false');
}
function frame(now){if(disposed)return;const delta=last?Math.min(100,now-last):0;last=now;if(!paused&&!document.hidden){accumulator+=delta;let steps=0;while(accumulator>=1000/60&&steps++<6){game.game_tick(held|pressed);pressed=0;accumulator-=1000/60;}}

 if(!demo&&game.game_value(0)===3&&game.game_value(11)===0){openCredits();return;}
 if(now-lastDraw>=1000/30){game.game_frame(theme,paused?1:0);ctx.putImageData(image,0,0);lastDraw=now;update();}raf=requestAnimationFrame(frame);}
function pause(){if(!game||disposed)return;paused=!paused;held=0;pressed=0;accumulator=0;last=0;if(paused)stopSound();else startSound().catch(()=>{});update();}
function reset(){if(!game||disposed)return;game.game_reset(0xD1A6+Math.floor(performance.now()));if(demo&&demo!=='play')game.game_demo(demo==='guetta'?1:0);held=0;pressed=0;accumulator=0;last=0;stopSound();window.RX3Audio?.stopSceneMusic?.('dj-wars');paused=true;update();}
function dispose(next='playlist'){if(disposed)return;disposed=true;cancelAnimationFrame(raf);held=0;pressed=0;processor?.disconnect();splitter?.disconnect();fxBus?.disconnect();musicBus?.disconnect();audio?.close();window.RX3Audio?.releaseSoundtrack('dj-wars',next);}
function openCredits(){
 if(demo||disposed||creditsTransition||!game||game.game_value(0)!==3||game.game_value(11)!==0)return;
 window.RX3Session.rescueWon();if(!window.RX3Session.rescued)return;
 creditsTransition=true;dispose('credits');
 window.RX3Session.navigate(new URL('../../../launcher-preview.html?credits=1&after=finale&theme='+(theme?'neon':'red'),location.href).href);
}
function menu(){if(disposed)return;dispose();window.RX3Session.navigate(new URL('../../../launcher-preview.html?v=session-1',location.href).href);}
const keys={ArrowLeft:1,a:1,A:1,ArrowRight:2,d:2,D:2,ArrowUp:4,w:4,W:4,ArrowDown:8,s:8,S:8,' ':16,Enter:32,e:64,E:64,t:128,T:128};
window.addEventListener('keydown',e=>{if(e.target instanceof HTMLInputElement||!game||disposed)return;if(keys[e.key]){e.preventDefault();if(paused&&e.key==='Enter'&&!e.repeat){pause();return;}held|=keys[e.key];if(!e.repeat)pressed|=keys[e.key];}else if(!e.repeat){if(e.key==='p'||e.key==='P')pause();else if(e.key==='r'||e.key==='R')reset();else if(e.key==='Escape')menu();else if(e.key==='m'||e.key==='M')game.game_camera(0,0,0);else if(e.key==='c'||e.key==='C')game.game_camera(1,0,0);else if(/^[1-7]$/.test(e.key))game.game_weapon(Number(e.key)-1);}});
window.addEventListener('keyup',e=>{if(keys[e.key]){e.preventDefault();held&=~keys[e.key];}});
window.addEventListener('blur',()=>{held=0;pressed=0;if(!paused)pause();});
document.addEventListener('visibilitychange',()=>{held=0;pressed=0;last=0;accumulator=0;if(document.hidden&&!paused)pause();});
for(const b of document.querySelectorAll('[data-key]')){const mask=Number(b.dataset.key);b.onpointerdown=e=>{e.preventDefault();b.setPointerCapture(e.pointerId);held|=mask;pressed|=mask;};b.onpointerup=b.onpointercancel=()=>held&=~mask;}
for(const b of document.querySelectorAll('[data-weapon]'))b.onclick=()=>game?.game_weapon(Number(b.dataset.weapon));
function canvasPoint(e){const r=canvas.getBoundingClientRect();return {x:(e.clientX-r.left)*1280/r.width,y:(e.clientY-r.top)*800/r.height};}
canvas.onpointerdown=e=>{if(!game||disposed)return;const {x,y}=canvasPoint(e);canvas.focus();canvas.setPointerCapture(e.pointerId);drag=null;if(game.game_value(0)===3||game.game_value(0)===4)return;if(x>=1003&&x<=1233&&y>=79&&y<=225){game.game_camera(3,Math.round((x-1008)*12),Math.round((y-86)*12));}else if(x>=360&&x<1240&&y>=70&&y<770){if(paused)pause();else drag={x,y,lastX:x,lastY:y,moved:false};}else if(x<320&&y>=336&&y<560)game.game_weapon(Math.floor((y-336)/32));else if(x<320&&y>=633&&y<682){held|=16;pressed|=16;}else if(y>=742&&y<778&&x<158)pause();else if(y>=742&&y<778&&x<320)menu();};
canvas.onpointermove=e=>{if(!drag)return;const {x,y}=canvasPoint(e);if(Math.abs(x-drag.x)+Math.abs(y-drag.y)>5)drag.moved=true;if(drag.moved){game.game_camera(2,Math.round(drag.lastX-x),Math.round(drag.lastY-y));drag.lastX=x;drag.lastY=y;}};
canvas.onpointerup=e=>{held&=~16;if(drag&&!drag.moved){const {x,y}=canvasPoint(e);game.game_target(Math.round(x-360),Math.round(y-70));}drag=null;};canvas.onpointercancel=()=>{held&=~16;pressed=0;drag=null;};
document.querySelector('#overview').onclick=()=>game?.game_camera(0,0,0);document.querySelector('#follow').onclick=()=>game?.game_camera(1,0,0);
start.onclick=pause;document.querySelector('#restart').onclick=reset;document.querySelector('#menu').onclick=menu;
document.querySelector('#volume').oninput=e=>{volume=Number(e.target.value)/100;window.RX3Audio.setEffectsLevel(volume);};
document.querySelector('#sound').textContent='SOUND: ON';
document.querySelector('#sound').onclick=e=>{sound=!sound;if(sound)startSound().catch(()=>{});else stopSound();e.target.textContent=sound?'SOUND: ON':'SOUND: OFF';};
window.addEventListener('pagehide',dispose,{once:true});
if(!demo&&!window.RX3Session.allCats){dispose();window.RX3Session.navigate(new URL('../../../launcher-preview.html?finale=1&v=cat-finale-1',location.href).href);}else try{const response=await fetch('dj-wars.wasm?v=15-victory-party');if(!response.ok)throw Error('Game module missing');const result=await WebAssembly.instantiate(await response.arrayBuffer(),{});game=result.instance.exports;game.game_reset(0xD1A6);if(demo&&demo!=='play')game.game_demo(demo==='guetta'?1:0);image=new ImageData(new Uint8ClampedArray(game.memory.buffer,game.game_buffer(),1280*800*4),1280,800);update();raf=requestAnimationFrame(frame);}catch(e){dispose();status.textContent='DJ WARS COULD NOT LOAD: '+e.message;}
