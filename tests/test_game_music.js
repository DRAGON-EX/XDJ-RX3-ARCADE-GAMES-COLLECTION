const vm=require('node:vm'),fs=require('node:fs'),assert=require('node:assert/strict');
const audios=[],timers=new Map();let serial=0;
class Audio{constructor(src=''){this.src=String(src);this.paused=true;this.currentTime=0;this.volume=1;audios.push(this);}play(){this.paused=false;return Promise.resolve();}pause(){this.paused=true;}removeAttribute(){this.src='';}load(){}}
const host={Audio,URL,Math,document:{currentScript:{src:'http://localhost/src/game-music.js'}},fetch:async()=>({json:async()=>[{source:'assets/music/a.mp3'},{source:'assets/music/war.mp3',locked:true,unlock:'dj-wars'},{source:'assets/music/secret.mp3',locked:true,unlock:'credits'}]}),setTimeout:(fn,delay)=>{timers.set(++serial,{fn,delay});return serial;},clearTimeout:id=>timers.delete(id)};host.window=host;host.parent=host;
vm.runInNewContext(fs.readFileSync('src/game-music.js','utf8'),host);
(async()=>{
 const a=host.RX3Audio;await a.start();a.music.currentTime=37.25;a.setGains(.8,.5,.25);assert.equal(a.music.volume,.2);
 a.rescue(2);assert(a.music.paused&&a.rescuing);assert.equal(a.music.currentTime,37.25);assert.equal(audios[1].paused,false);assert(Math.abs(audios[1].volume-.32)<.00001);
 for(const t of [...timers.values()])if(t.delay===17500)t.fn();assert(audios.at(-1).src.endsWith('cat-meow-2.mp3'));
 a.finishRescue();assert.equal(a.music.currentTime,37.25);assert(!a.music.paused&&!a.rescuing);
 a.music.pause();a.rescue(0);a.finishRescue();assert(a.music.paused,'User-paused music must stay paused');
 a.cafe(true);assert(audios.at(-1).src.endsWith('cat-purr-soft.mp3'));a.setGains(0,1,1);assert.equal(audios.at(-1).volume,0);a.cafe(false);assert(audios.at(-1).paused);
 a.setGains(.8,.5,.25);a.setMusicLevel(.6);
 const context={destination:{},createGain:()=>({gain:{value:0},connect(){}}),addEventListener(){}};
 const fx=a.effects(context),score=a.soundtrack(context);assert.equal(fx.gain.value,.4);assert.equal(score.gain.value,.12);
 await a.start();a.claimSoundtrack('dj-wars');assert(a.music.paused);await a.start();assert(a.music.paused);
 a.releaseSoundtrack('wrong-owner');assert(a.music.paused);a.releaseSoundtrack('dj-wars');assert(!a.music.paused);assert.equal(a.music.currentTime,37.25);
 a.music.pause();a.claimSoundtrack('dj-wars');a.releaseSoundtrack('dj-wars');assert(a.music.paused);
 await a.start();a.claimSoundtrack('dj-wars');a.releaseSoundtrack('dj-wars','credits');assert(a.music.paused,'Credits handoff must not briefly restart the playlist');assert.equal(a.takeCreditsResume(),true);assert.equal(a.takeCreditsResume(),null);
 a.music.pause();a.claimSoundtrack('dj-wars');a.releaseSoundtrack('dj-wars','credits');assert.equal(a.takeCreditsResume(),false);assert(a.music.paused);
 // Scene music has a dedicated element; pause/resume never seeks the user's song.
 await a.start();const originalSource=a.music.src;a.music.currentTime=51.25;a.claimSoundtrack('dj-wars');
 a.sceneMusic('wrong','assets/music/alex-morgan-game-level-pixel-quest-loop-578505.mp3');assert.equal(audios[2].src,'');
 a.sceneMusic('dj-wars','assets/music/alex-morgan-game-level-pixel-quest-loop-578505.mp3');const scene=audios[2];
 assert(scene.loop&&!scene.paused);assert.equal(scene.volume,.12);assert.equal(a.music.currentTime,51.25);assert(a.music.paused);
 scene.currentTime=3.75;a.sceneMusic('dj-wars','assets/music/alex-morgan-game-level-pixel-quest-loop-578505.mp3',false);assert(scene.paused);assert.equal(scene.currentTime,3.75);
 a.setEffectsLevel(.02);assert.equal(scene.volume,.12);a.setGains(.4,.9,.5);assert.equal(scene.volume,.12);
 a.sceneMusic('dj-wars','assets/music/alex-morgan-game-level-pixel-quest-loop-578505.mp3');assert(!scene.paused);assert.equal(scene.currentTime,3.75);
 a.releaseSoundtrack('dj-wars','credits');assert(scene.paused&&!scene.src);assert(a.music.paused);assert.equal(a.takeCreditsResume(),true);
 // Credits restore their source while paused, then hand the previous play intent to the cafe.
 a.claimSoundtrack('cafe',true);a.cafe(true);assert(scene.src.endsWith('junipersona-pixel-plains-158773.mp3'));assert(scene.loop&&!scene.paused);assert.equal(scene.currentTime,0);
 scene.currentTime=17;a.cafe(true);assert.equal(scene.currentTime,0,'Every cafe entry starts Pixel Plains again');assert(a.music.paused);
 a.cafe(false);assert(scene.paused&&!scene.src);assert(!a.music.paused);assert.equal(a.music.currentTime,51.25);assert.equal(a.music.src,originalSource);
 a.music.pause();a.cafe(true);a.cafe(false);assert(a.music.paused,'Cafe exit preserves a user-paused playlist');
 a.setGains(0,1,1);assert.equal(scene.volume,0);assert.equal(fx.gain.value,0);assert.equal(score.gain.value,0);
 host.RX3Session={rescued:false,secret:false};a.music.onended();assert(a.music.src.endsWith('a.mp3'));
 host.RX3Session.rescued=true;a.music.onended();assert(a.music.src.endsWith('war.mp3'));a.music.onended();assert(a.music.src.endsWith('a.mp3'),'Credits secret stays locked after DJ Wars');
 host.RX3Session.secret=true;a.music.onended();a.music.onended();assert(a.music.src.endsWith('secret.mp3'));
 assert(!fs.readFileSync('src/game-music.js','utf8').match(/localStorage|sessionStorage|indexedDB/));
 console.log('Browser audio host: exact resume, user pause, independent gains, rescue meow, cafe purr and cleanup passed.');
})().catch(e=>{console.error(e);process.exitCode=1;});
