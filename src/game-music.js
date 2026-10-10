/* Shared, RAM-only audio host across launcher/game iframes. */
(() => {
 const base=new URL('../',document.currentScript.src);
 if(parent!==window&&parent.RX3Audio){window.RX3Audio=parent.RX3Audio;return;}
 const music=new Audio(),fanfare=new Audio(new URL('assets/sounds/rescue-fanfare.mp3',base)),sceneAudio=new Audio();
 sceneAudio.loop=true;
 const voices=['cat-cute','cat-meow-1','cat-meow-2','cat-meow-3','cat-purr-soft','cat-purr-warm'];
 let master=1,fx=1,musicGain=1,level=1,rescuing=false,resume=false,cat=null,cafeTimer=0,rescueTimer=0,catTimer=0,tracks=null,index=0,owned=true;
 const buses=new Set(),soundtrackBuses=new Set();
 let soundtrackOwner=null,resumePlaylist=false,creditsResume=null,sceneOwner=null,sceneSource=null;
 const cafeSource='assets/music/junipersona-pixel-plains-158773.mp3',cafeShowSource='assets/music/cafe-good.mp3';
 let cafeShowRequested=false,cafeShowReady=false,cafeShowEpoch=0,cafeShowResume=0;
 const unlocked=t=>!t.locked||(t.unlock==='dj-wars'&&window.RX3Session?.rescued)||(t.unlock==='credits'&&window.RX3Session?.secret);
 const apply=()=>{music.volume=sceneAudio.volume=master*musicGain*level;fanfare.volume=master*fx*.8;if(cat)cat.volume=master*fx*(cat.purr?.22:.6);for(const node of buses)node.gain.value=master*fx;for(const node of soundtrackBuses)node.gain.value=master*musicGain*level;};
 async function start(){if(rescuing||soundtrackOwner)return;if(!music.src){tracks ||= await fetch(new URL('content/music-library.json',base)).then(r=>r.json());const list=tracks.filter(unlocked);if(!list.length)return;music.src=new URL(list[index%list.length].source,base).href;}if(!rescuing&&!soundtrackOwner)await music.play().catch(()=>{});}
 music.onended=()=>{if(!owned)return;const list=tracks?.filter(unlocked)||[];if(list.length){index=(index+1)%list.length;music.src=new URL(list[index].source,base).href;start();}};
 function stopSceneMusic(owner){if(sceneOwner!==owner)return;if(owner==='cafe'){cafeShowRequested=cafeShowReady=false;cafeShowEpoch++;}sceneAudio.pause();sceneAudio.currentTime=0;sceneAudio.removeAttribute?.('src');sceneAudio.load?.();sceneOwner=sceneSource=null;}
 function sceneMusic(owner,source,playing=true){
  if(soundtrackOwner!==owner)return;
  const src=new URL(source,base).href;
  if(sceneOwner!==owner||sceneSource!==src){if(sceneOwner)stopSceneMusic(sceneOwner);sceneOwner=owner;sceneSource=src;sceneAudio.src=src;sceneAudio.currentTime=0;sceneAudio.loop=true;}
  apply();if(playing){if(sceneAudio.paused)sceneAudio.play().catch(()=>{});}else sceneAudio.pause();
 }
 function finishCafeShow(){
  if(!cafeShowRequested)return;
  const position=cafeShowResume;stopSceneMusic('cafe');
  if(soundtrackOwner==='cafe'){sceneMusic('cafe',cafeSource);sceneAudio.currentTime=position;}
 }
 function cafeShow(on){
  if(on===undefined)on=!cafeShowRequested;
  if(!on){finishCafeShow();return;}
  if(soundtrackOwner!=='cafe'||cafeShowRequested)return;
  cafeShowResume=sceneOwner==='cafe'?sceneAudio.currentTime:0;
  stopSceneMusic('cafe');cat?.pause();sceneOwner='cafe';sceneSource=new URL(cafeShowSource,base).href;
  sceneAudio.src=sceneSource;sceneAudio.currentTime=0;sceneAudio.loop=false;
  cafeShowRequested=true;cafeShowReady=false;const epoch=++cafeShowEpoch;apply();
  /* Only confirmed playback activates visuals; blocked media never runs a fake clock. */
  try{Promise.resolve(sceneAudio.play()).then(()=>{if(cafeShowRequested&&epoch===cafeShowEpoch)cafeShowReady=true;},()=>{if(cafeShowRequested&&epoch===cafeShowEpoch)finishCafeShow();});}
  catch{if(cafeShowRequested&&epoch===cafeShowEpoch)finishCafeShow();}
 }
 function cafeShowState(){return {active:cafeShowRequested&&cafeShowReady&&!sceneAudio.paused&&!sceneAudio.ended,pending:cafeShowRequested&&!cafeShowReady,ms:cafeShowRequested&&cafeShowReady&&Number.isFinite(sceneAudio.currentTime)?Math.max(0,Math.floor(sceneAudio.currentTime*1000)):0};}
 sceneAudio.onended=()=>{if(sceneOwner==='cafe')finishCafeShow();};
 sceneAudio.onerror=()=>{if(sceneOwner==='cafe')finishCafeShow();};
 function meow(id=0,purr=false){if(cafeShowRequested)return;cat?.pause();cat=new Audio(new URL('assets/sounds/cats/'+voices[purr?4+id%2:id%4]+'.mp3',base));cat.purr=purr;apply();cat.play().catch(()=>{});}
 function finish(){clearTimeout(rescueTimer);clearTimeout(catTimer);rescuing=false;fanfare.pause();if(resume)start();resume=false;}
 window.RX3Audio={music,start,get rescuing(){return rescuing;},adopt(){owned=false;return music;},
  setEffectsLevel(v){fx=Math.max(0,Math.min(1,v));apply();},
  setMusicLevel(v){level=Math.max(0,Math.min(1,v));apply();},get musicLevel(){return level;},
  setGains(m,s,b){master=Math.max(0,Math.min(1,m));fx=Math.max(0,Math.min(1,s));musicGain=Math.max(0,Math.min(1,b));apply();},
  effects(context){const node=context.createGain();node.connect(context.destination);buses.add(node);apply();context.addEventListener('statechange',()=>{if(context.state==='closed')buses.delete(node);});return node;},
  soundtrack(context){const node=context.createGain();node.connect(context.destination);soundtrackBuses.add(node);apply();context.addEventListener('statechange',()=>{if(context.state==='closed')soundtrackBuses.delete(node);});return node;},
  claimSoundtrack(owner='dj-wars',resumeState){if(soundtrackOwner===owner)return;if(!soundtrackOwner)resumePlaylist=resumeState??!music.paused;if(sceneOwner)stopSceneMusic(sceneOwner);soundtrackOwner=owner;music.pause();},
  sceneMusic,stopSceneMusic,cafeShow,cafeShowState,
  releaseSoundtrack(owner='dj-wars',next='playlist'){if(soundtrackOwner!==owner)return;stopSceneMusic(owner);soundtrackOwner=null;const resumeNow=resumePlaylist;resumePlaylist=false;if(next==='credits')creditsResume=resumeNow;else if(resumeNow)start();},
  takeCreditsResume(){const value=creditsResume;creditsResume=null;return value;},
  rescue(id){finish();resume=!music.paused;music.pause();rescuing=true;fanfare.currentTime=0;apply();fanfare.play().catch(()=>{});catTimer=setTimeout(()=>meow(id),17500);rescueTimer=setTimeout(finish,20000);},
  finishRescue:finish,meow,
  cafe(on){clearTimeout(cafeTimer);cat?.pause();if(!on){this.releaseSoundtrack('cafe');return;}this.claimSoundtrack('cafe');stopSceneMusic('cafe');sceneMusic('cafe',cafeSource);meow(0,true);const next=()=>{meow(Math.floor(Math.random()*4),Math.random()<.3);cafeTimer=setTimeout(next,10000+Math.random()*12000);};cafeTimer=setTimeout(next,12000);}
 };
})();
