/* The player uses the same 5x7 pixel font and palettes as the native RX3 UI. */
(() => {
 const tracks=[
  {
    "title": "HYPERARCADE BONUS POINTS",
    "artist": "SLIMEYFOX",
    "duration": 81.371406,
    "src": "assets/music/slimeyfox-hyperarcade-bonus-points.mp3",
    "locked": false
  },
  {
    "title": "RETRO GAME ARCADE",
    "artist": "MOODMODE",
    "duration": 65.776313,
    "src": "assets/music/moodmode-retro-game-arcade.mp3",
    "locked": false
  },
  {
    "title": "8 BIT RETRO GAME MUSIC",
    "artist": "MOODMODE",
    "duration": 70.164875,
    "src": "assets/music/moodmode-8-bit-retro-game-music.mp3",
    "locked": false
  },
  {
    "title": "BEST GAME CONSOLE",
    "artist": "DJARTMUSIC",
    "duration": 86.491406,
    "src": "assets/music/djartmusic-best-game-console.mp3",
    "locked": false
  },
  {
    "title": "CHIPTUNE HAPPINESS",
    "artist": "MUSICINMEDIA",
    "duration": 108.721625,
    "src": "assets/music/musicinmedia-chiptune-happiness-retro-8bit-game-music-453214.mp3",
    "locked": false
  },
  {
    "title": "HYPERARCADE QUICKEST",
    "artist": "SLIMEYFOX",
    "duration": 106.762438,
    "src": "assets/music/slimeyfox-hyperarcade-quickest-582346.mp3",
    "locked": false
  },
  {
    "title": "PIXEL PLAINS",
    "artist": "JUNIPERSONA",
    "duration": 149.76,
    "src": "assets/music/junipersona-pixel-plains-158773.mp3",
    "locked": false
  },
  {
    "title": "EPIC CHIPTUNE END CREDITS MUSIC",
    "artist": "NICKPANEK",
    "duration": 66.192,
    "src": "assets/music/nickpanek-epic-chiptune-end-credits-music-235893.mp3",
    "locked": false
  },
  {
    "title": "THE RETURN OF THE 8-BIT ERA",
    "artist": "DJARTMUSIC",
    "duration": 71.758344,
    "src": "assets/music/djartmusic-the-return-of-the-8-bit-era-301292.mp3",
    "locked": false
  },
  {
    "title": "SO HAPPY WITH MY 8-BIT GAME",
    "artist": "DJARTMUSIC",
    "duration": 75.25875,
    "src": "assets/music/djartmusic-so-happy-with-my-8-bit-game-301275.mp3",
    "locked": false
  },
  {
    "title": "FUN WITH MY 8-BIT GAME",
    "artist": "DJARTMUSIC",
    "duration": 76.460406,
    "src": "assets/music/djartmusic-fun-with-my-8-bit-game-301278.mp3",
    "locked": false
  },
  {
    "title": "GAME LEVEL PIXEL QUEST LOOP",
    "artist": "ALEX MORGAN",
    "duration": 193.512,
    "src": "assets/music/alex-morgan-game-level-pixel-quest-loop-578505.mp3",
    "locked": true,
    "unlock": "dj-wars",
    "lock_message": "SECRET SONG WIN DJ WARS TO UNLOCK"
  },
  {
    "title": "THAT GAME ARCADE",
    "artist": "MOODMODE",
    "duration": 73.247344,
    "src": "assets/music/moodmode-that-game-arcade-236111.mp3",
    "locked": true,
    "lock_message": "SECRET SONG WATCH CREDITS TO UNLOCK",
    "unlock": "credits"
  },
  {
    "title": "SYNTHWAVE RETRO 80S MUSIC",
    "artist": "TUNETANK",
    "duration": 152.032625,
    "src": "assets/music/tunetank-synthwave-retro-80s-music-349182.mp3",
    "locked": true
  },
  {
    "title": "NEWS SUMMARY",
    "artist": "MOODMODE",
    "duration": 103.41875,
    "src": "assets/music/moodmode-news-summary-449042.mp3",
    "locked": true
  },
  {
    "title": "VIDEO GAMES",
    "artist": "ECHOES OF LUMEN",
    "duration": 109.872,
    "src": "assets/music/echoes_of_lumen-video-games-584898.mp3",
    "locked": true
  }
];
 const secretIndex=tracks.findIndex(t=>t.unlock==='credits'||t.src.endsWith('moodmode-that-game-arcade-236111.mp3'));
 const isLocked=t=>!!t.locked&&!(t.unlock==='dj-wars'&&window.RX3Session.rescued)&&!(t.unlock==='credits'&&window.RX3Session.secret);
 const hiddenSecret=t=>isLocked(t)&&!!t.lock_message;
 function unlockSecret(){if(secretIndex<0||!window.RX3Session.rescued)return;tracks[secretIndex].locked=false;delete tracks[secretIndex].lock_message;window.RX3Session.unlockSecret();draw();}
 if(secretIndex>=0&&window.RX3Session.secret){tracks[secretIndex].locked=false;delete tracks[secretIndex].lock_message;}
 let creditsAudioState=null;
 function beginCredits(){if(creditsAudioState||secretIndex<0)return;const resume=window.RX3Audio?.takeCreditsResume?.();creditsAudioState={index,selected,browserPage,time:audio.currentTime,paused:resume==null?audio.paused:!resume,src:audio.getAttribute('src'),passes:0,complete:false};audio.pause();index=secretIndex;audio.src=tracks[index].src;audio.loop=false;play();}
 function creditsProgress(){if(!creditsAudioState)return {fraction:0,complete:false};const s=creditsAudioState,d=duration();return {fraction:s.complete?1:Math.min(1,(s.passes+(d?audio.currentTime/d:0))/2),complete:s.complete};}
 function restartCredits(){if(!creditsAudioState)return;creditsAudioState.passes=0;creditsAudioState.complete=false;audio.currentTime=0;play();}
 function endCredits(next='playlist'){if(!creditsAudioState)return;const state=creditsAudioState;creditsAudioState=null;playAttempt++;audio.pause();audio.loop=false;index=state.index;selected=state.selected;browserPage=state.browserPage;if(state.src){audio.src=state.src;try{audio.currentTime=state.time}catch{}if(next!=='cafe'&&!state.paused)play();}else{audio.removeAttribute('src');audio.load();}if(next==='cafe')window.RX3Audio?.claimSoundtrack('cafe',!state.paused);}
 const audio=window.RX3Audio?.adopt()||new Audio();audio.id='launcher-audio';audio.hidden=true;document.body.append(audio);audio.preload='metadata';
 const canvas=document.createElement('canvas');canvas.width=1280;canvas.height=670;canvas.hidden=true;canvas.id='music-player-canvas';canvas.setAttribute('aria-label','Retro music player');
 Object.assign(canvas.style,{position:'absolute',left:'0',top:'16.25%',width:'100%',height:'83.75%',touchAction:'none'});document.querySelector('main').append(canvas);
 const fallback=document.createElement('img');fallback.alt='Yuki — rescued DJ cat';fallback.hidden=true;Object.assign(fallback.style,{position:'absolute',left:'8.59375%',top:'29.375%',width:'14.0625%',height:'22.5%',imageRendering:'pixelated',pointerEvents:'none'});document.querySelector('main').append(fallback);
 const controls=document.createElement('div');controls.id='music-player-controls';controls.hidden=true;document.querySelector('main').append(controls);
 let index=Math.max(0,tracks.findIndex(t=>audio.src.endsWith(t.src))),selected=0,browserPage=0,remaining=false,visible=false,raf=0,error='',drag='',playAttempt=0;
 function button(label,x,y,w,h,action){const b=document.createElement('button');b.type='button';b.textContent=label;b.setAttribute('aria-label',label);Object.assign(b.style,{position:'absolute',left:x/12.8+'%',top:y/8+'%',width:w/12.8+'%',height:h/8+'%',opacity:'0',cursor:'pointer'});b.onclick=action;controls.append(b);return b;}
 const ctx=canvas.getContext('2d');
 function color(c){return theme==='red'?({'#080f1b':'#090709','#14283d':'#201014','#10243c':'#251117','#243b50':'#39131e','#64ffe1':'#ff405c','#8fa1b5':'#b7a1a8','#763044':'#6c1428'}[c]||c):c;}
 function rect(x,y,w,h,c){ctx.fillStyle=color(c);ctx.fillRect(x,y-130,w,h);}
 function disk(x,y,r,c){for(let yy=-r;yy<=r;yy++){const xx=Math.floor(Math.sqrt(r*r-yy*yy));rect(x-xx,y+yy,xx*2+1,1,c);}}
 function word(value,x,y,scale,c,limit=100){ctx.fillStyle=color(c);for(const ch of value.toUpperCase().slice(0,limit)){const n=ch>='0'&&ch<='9'?ch.charCodeAt(0)-48:ch>='A'&&ch<='Z'?ch.charCodeAt(0)-55:-1;let bits=n>=0?retroFont[n]:ch===':'?[0,0,4,0,4,0,0]:ch==='.'?[0,0,0,0,0,0,4]:ch==='-'?[0,0,0,31,0,0,0]:ch==='/'?[1,2,2,4,8,8,16]:ch==='%'?[17,2,4,4,8,16,17]:null;if(bits)for(let yy=0;yy<7;yy++)for(let xx=0;xx<5;xx++)if(bits[yy]&(1<<(4-xx)))ctx.fillRect(x+xx*scale,y-130+yy*scale,scale,scale);x+=6*scale;}}
 function time(v,minus=false){const ms=Math.max(0,Math.floor(v*1000));return (minus?'-':'')+String(Math.floor(ms/60000)).padStart(2,'0')+':'+String(Math.floor(ms/1000)%60).padStart(2,'0')+'.'+String(ms%1000).padStart(3,'0');}
 function duration(){return Number.isFinite(audio.duration)?audio.duration:tracks[index].duration;}
 function play(){const attempt=++playAttempt;error='';audio.play().then(()=>{if(attempt===playAttempt){error='';draw();}}).catch(()=>{if(attempt===playAttempt){error='PRESS PLAY TO START AUDIO';draw();}});}
 function choose(i){i=(i+tracks.length)%tracks.length;if(isLocked(tracks[i])){error=tracks[i].lock_message||'NOT YET UNLOCKED';draw();return;}index=i;selected=index;browserPage=Math.floor(index/4);audio.src=tracks[index].src;play();draw();}
 function toggle(){if(!audio.src){audio.src=tracks[index].src;play();}else if(audio.paused)play();else audio.pause();draw();}
 function adjacent(step){let i=index;for(let n=0;n<tracks.length;n++){i=(i+step+tracks.length)%tracks.length;if(!isLocked(tracks[i])){choose(i);return;}}}
 const playButton=button('Play / pause',215,568,270,62,toggle);
 button('Previous track',110,568,90,62,()=>adjacent(-1));button('Next track',500,568,90,62,()=>adjacent(1));
 button('Switch elapsed / remaining time',555,490,200,48,()=>{remaining=!remaining;draw();});
 button('Close music player',1070,180,100,46,()=>{page='';render();});
 button('Previous browser page',805,660,165,42,()=>{browserPage=Math.max(0,browserPage-1);draw();});button('Next browser page',1005,660,165,42,()=>{browserPage=Math.min(Math.ceil(tracks.length/4)-1,browserPage+1);draw();});
 for(let row=0;row<4;row++)button('Play track '+(row+1),805,278+row*82,365,72,()=>{const i=browserPage*4+row;if(i<tracks.length)choose(i);});
 function slider(label,x,y,w,h,action){const input=document.createElement('input');input.type='range';input.min=0;input.max=1000;input.value=label==='Music volume'?1000:0;input.setAttribute('aria-label',label);Object.assign(input.style,{position:'absolute',left:x/12.8+'%',top:y/8+'%',width:w/12.8+'%',height:h/8+'%',opacity:'0',margin:0,cursor:'pointer'});input.oninput=()=>action(Number(input.value)/1000);controls.append(input);return input;}
 const seek=slider('Seek within current track',110,430,645,52,v=>{if(!audio.src)audio.src=tracks[index].src;audio.currentTime=v*duration();draw();});
 const volume=slider('Music volume',210,651,455,59,v=>{window.RX3Audio?window.RX3Audio.setMusicLevel(v):audio.volume=v;draw();});
 function draw(){if(!visible)return;const t=tracks[index],d=duration(),elapsed=Math.min(audio.currentTime||0,d);
  rect(0,130,1280,670,'#10243c');rect(20,150,1240,3,'#64ffe1');word('DJ DRAGON-X - MUSIC',110,190,4,'#64ffe1');rect(1070,180,100,46,'#763044');word('CLOSE',1084,194,2,'#fff');
  fallback.hidden=!(window.RX3Session.catMask&1)||!!(t.coverImage?.complete&&t.coverImage.naturalWidth);if(fallback.hidden){if(fallback.hasAttribute('src'))fallback.removeAttribute('src');}else if(!fallback.hasAttribute('src'))fallback.src='assets/music/no-cover-cat.gif';rect(110,235,180,180,'#14283d');if(t.coverImage?.complete&&t.coverImage.naturalWidth)ctx.drawImage(t.coverImage,110,105,180,180);else{disk(200,320,65,'#080f1b');disk(200,320,46,'#243b50');disk(200,320,25,'#64ffe1');disk(200,320,6,'#080f1b');word('NO COVER',137,390,2,'#8fa1b5');}
  word('NOW PLAYING',325,242,2,'#64ffe1');word(hiddenSecret(t)?'SECRET SONG':t.title,325,280,2,hiddenSecret(t)?'#ff405c':'#fff',35);word(hiddenSecret(t)?t.lock_message.replace('SECRET SONG ',''):t.artist,325,320,2,'#8fa1b5',35);word(error||audio.paused?'PAUSED':'PLAYING',325,365,2,'#64ffe1');word('TRACK '+(index+1)+' / '+tracks.length,325,393,2,'#8fa1b5');
  rect(110,453,645,12,'#243b50');const fill=d?elapsed/d*645:0;rect(110,453,fill,12,'#64ffe1');rect(107+fill,446,6,26,'#fff');word(time(remaining?d-elapsed:elapsed,remaining),110,501,4,'#fff');rect(555,490,200,48,'#14283d');word(remaining?'REMAINING':'ELAPSED',570,506,2,'#64ffe1');
  rect(110,568,90,62,'#14283d');word('PREV',127,591,2,'#64ffe1');rect(215,568,270,62,'#64ffe1');word(audio.paused?'PLAY':'PAUSE',270,588,3,'#080f1b');rect(500,568,90,62,'#14283d');word('NEXT',517,591,2,'#64ffe1');
  word('VOLUME',110,671,2,'#8fa1b5');word(Math.round((window.RX3Audio?.musicLevel??audio.volume)*100)+'%',690,671,2,'#64ffe1');rect(225,675,425,10,'#243b50');rect(225,675,425*(window.RX3Audio?.musicLevel??audio.volume),10,'#64ffe1');rect(222+425*(window.RX3Audio?.musicLevel??audio.volume),667,6,26,'#fff');word('TRACK BROWSER',805,242,2,'#64ffe1');
  for(let row=0;row<4;row++){const i=browserPage*4+row;if(i>=tracks.length)break;const y=278+row*82;rect(805,y,365,72,'#14283d');if(i===selected){rect(805,y,3,72,'#64ffe1');rect(805,y,365,2,'#64ffe1');}if(hiddenSecret(tracks[i])){word('SECRET SONG',818,y+13,2,'#ff405c');word(tracks[i].lock_message.replace('SECRET SONG ',''),818,y+43,2,'#8fa1b5',28);}else{word(tracks[i].title,818,y+13,2,'#fff',28);word(isLocked(tracks[i])?'NOT YET UNLOCKED':tracks[i].artist,818,y+43,2,'#8fa1b5',23);}if(i===index)rect(1155,y+46,5,10,'#64ffe1');}
  word(error?'PRESS PLAY TO START':'BROWSE - PRESS TO PLAY',805,620,2,'#8fa1b5');rect(805,660,165,42,'#14283d');word('PREVIOUS',823,674,2,'#64ffe1');rect(1005,660,165,42,'#14283d');word('NEXT',1050,674,2,'#64ffe1');
  playButton.setAttribute('aria-label',audio.paused?'Play':'Pause');seek.value=d?elapsed/d*1000:0;
 }
 function loop(){if(!visible)return;draw();raf=requestAnimationFrame(loop);}
 audio.onended=()=>{if(creditsAudioState){creditsAudioState.passes++;if(creditsAudioState.passes<2){audio.currentTime=0;play();}else creditsAudioState.complete=true;}else adjacent(1);};audio.onerror=()=>{error='TRACK UNAVAILABLE';draw();};audio.onloadedmetadata=draw;
 canvas.addEventListener('wheel',e=>{e.preventDefault();if(e.deltaY){selected=(selected+(e.deltaY>0?1:-1)+tracks.length)%tracks.length;browserPage=Math.floor(selected/4);draw();}},{passive:false});
 window.addEventListener('keydown',e=>{if(!visible)return;if(e.key===' '){e.preventDefault();toggle();}else if(e.key==='ArrowDown'||e.key==='ArrowUp'){e.preventDefault();selected=(selected+(e.key==='ArrowDown'?1:-1)+tracks.length)%tracks.length;browserPage=Math.floor(selected/4);draw();}else if(e.key==='Enter'&&document.activeElement.tagName!=='BUTTON'){e.preventDefault();choose(selected);}});
 window.RX3MusicPlayer={sync(){const next=page==='music';if(next!==visible){visible=next;canvas.hidden=controls.hidden=!visible;if(visible){if(!audio.src){audio.src=tracks[index].src;play();}loop();}else{cancelAnimationFrame(raf);fallback.hidden=true;fallback.removeAttribute('src');}}else draw();},audio,tracks,isLocked,choose,toggle,time,beginCredits,endCredits,creditsProgress,restartCredits,unlockSecret,secretIndex};
 if(new URLSearchParams(location.search).get('music')==='1'){page='music';document.querySelector('#ready').hidden=true;render();}window.RX3MusicPlayer.sync();
})();
