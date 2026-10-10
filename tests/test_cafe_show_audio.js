const vm=require('node:vm'),fs=require('node:fs'),assert=require('node:assert/strict');
const audios=[],timers=new Map();let serial=0,failShow=false,resolveShow=null,deferShow=false;
class Audio {
 constructor(src=''){this.src=String(src);this.paused=true;this.currentTime=0;this.volume=1;this.ended=false;audios.push(this);}
 play(){if(this.src.endsWith('cafe-good.mp3')){if(failShow)return Promise.reject(Error('blocked'));if(deferShow)return new Promise(resolve=>{resolveShow=()=>{this.paused=false;resolve();};});}this.paused=false;return Promise.resolve();}
 pause(){this.paused=true;}removeAttribute(){this.src='';}load(){}
}
const host={Audio,URL,Math,Number,document:{currentScript:{src:'http://localhost/src/game-music.js'}},fetch:async()=>({json:async()=>[{source:'assets/music/a.mp3'}]}),setTimeout:(fn,delay)=>{timers.set(++serial,{fn,delay});return serial;},clearTimeout:id=>timers.delete(id)};host.window=host;host.parent=host;
vm.runInNewContext(fs.readFileSync('src/game-music.js','utf8'),host);
const flush=()=>new Promise(setImmediate);
(async()=>{
 const a=host.RX3Audio,scene=audios[2];await a.start();a.music.currentTime=41;a.cafe(true);scene.currentTime=23.5;
 a.setGains(.8,.7,.5);a.setMusicLevel(.5);assert.equal(scene.volume,.2);
 a.cafeShow(true);assert(scene.src.endsWith('cafe-good.mp3')&&!scene.loop);assert(!a.cafeShowState().active&&a.cafeShowState().pending);await flush();assert(a.cafeShowState().active);
 scene.currentTime=1.2349;assert.equal(a.cafeShowState().ms,1234);assert.equal(a.music.currentTime,41);assert.equal(scene.volume,.2);
 const count=audios.length;a.meow(1);for(const t of [...timers.values()])t.fn();assert.equal(audios.length,count,'Show must suppress café meows and purrs');
 a.setEffectsLevel(.01);assert.equal(scene.volume,.2);a.setGains(0,1,1);assert.equal(scene.volume,0);a.setGains(1,1,1);a.setMusicLevel(1);
 a.cafeShow();assert(!a.cafeShowState().active&&scene.src.endsWith('pixel-plains-158773.mp3'));assert(scene.loop&&!scene.paused);assert.equal(scene.currentTime,23.5);
 a.cafeShow();await flush();scene.currentTime=175.25;scene.onended();assert(!a.cafeShowState().active&&scene.loop);assert.equal(scene.currentTime,23.5);
 a.cafeShow(true);await flush();scene.onerror();assert(!a.cafeShowState().active&&scene.src.endsWith('pixel-plains-158773.mp3'));
 failShow=true;a.cafeShow(true);await flush();assert(!a.cafeShowState().active&&!a.cafeShowState().pending);assert(scene.src.endsWith('pixel-plains-158773.mp3'));assert.equal(scene.currentTime,23.5);failShow=false;
 deferShow=true;a.cafeShow(true);assert(a.cafeShowState().pending&&!a.cafeShowState().active);a.cafe(false);resolveShow();await flush();assert(!a.cafeShowState().active&&!a.cafeShowState().pending,'Late play promise cannot resurrect exited show');assert(!scene.src&&!a.music.paused);assert.equal(a.music.currentTime,41);
 a.cafeShow(true);assert(!a.cafeShowState().active&&!scene.src,'Only the café may own its vinyl show');
 a.music.pause();deferShow=false;a.cafe(true);a.cafeShow(true);await flush();a.cafe(false);assert(a.music.paused&&!a.cafeShowState().active,'Exit preserves original paused playlist');
 console.log('Cafe show browser audio: media-time clock, gains, no meows, toggle/natural/error restore, blocked playback and exit cleanup passed.');
})().catch(e=>{console.error(e);process.exitCode=1;});
