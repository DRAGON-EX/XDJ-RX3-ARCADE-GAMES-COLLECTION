/* Test soundtrack lifecycle without waiting for two real-time song passes. */
const fs=require('fs'),vm=require('vm'),assert=require('assert');
const nodes=new Map(),store=new Map();
function node(){return {style:{},hidden:true,tagName:'DIV',append(){},setAttribute(){},addEventListener(){},getContext(){return {}},getAttribute(){return null}};}
const document={body:node(),activeElement:node(),createElement:node,querySelector(selector){if(!nodes.has(selector))nodes.set(selector,node());return nodes.get(selector)}};
class Audio {constructor(){this.src='';this.currentTime=0;this.duration=73.247;this.paused=true;this.plays=0;this.loop=false;}getAttribute(){return this.src||null}removeAttribute(){this.src=''}load(){}play(){this.paused=false;this.plays++;return Promise.resolve()}pause(){this.paused=true}}
const sandbox={document,Audio,window:{addEventListener(){}},page:'',theme:'red',location:{search:''},URLSearchParams,localStorage:{getItem:k=>store.get(k),setItem:(k,v)=>store.set(k,v)},requestAnimationFrame(){},cancelAnimationFrame(){}};
vm.createContext(sandbox);sandbox.window.parent=sandbox.window;
vm.runInContext(fs.readFileSync('src/session-state.js','utf8'),sandbox);
vm.runInContext(fs.readFileSync('src/music-player.js','utf8'),sandbox);
const player=sandbox.window.RX3MusicPlayer,a=player.audio;
const fireworksIndex=player.tracks.findIndex(t=>t.unlock==='dj-wars');
assert(fireworksIndex>=0&&fireworksIndex!==player.secretIndex);assert(player.isLocked(player.tracks[fireworksIndex]));assert.equal(player.tracks[fireworksIndex].lock_message,'SECRET SONG WIN DJ WARS TO UNLOCK');
player.choose(fireworksIndex);assert(!a.src,'Locked fireworks song cannot be selected');
assert.equal(player.tracks[player.secretIndex].unlock,'credits');assert(player.tracks[player.secretIndex].locked);
player.beginCredits();assert(a.src.endsWith('moodmode-that-game-arcade-236111.mp3')&&!a.loop&&!a.paused&&a.plays===1);
a.currentTime=a.duration/2;assert.equal(player.creditsProgress().fraction,0.25);
a.currentTime=a.duration;a.paused=true;a.onended();assert.equal(a.plays,2);assert.equal(player.creditsProgress().fraction,0.5);assert(!player.creditsProgress().complete);
a.currentTime=a.duration;a.paused=true;a.onended();assert.equal(a.plays,2);assert.equal(player.creditsProgress().fraction,1);assert(player.creditsProgress().complete&&a.paused);assert(player.tracks[player.secretIndex].locked); // credits renderer owns the reward
player.endCredits();assert(a.paused&&!a.src);
player.beginCredits();a.currentTime=15;player.endCredits();assert(a.paused&&!a.src&&player.tracks[player.secretIndex].locked);
player.unlockSecret();assert(player.tracks[player.secretIndex].locked);
player.choose(fireworksIndex-1);a.onended();assert(a.src.endsWith(player.tracks[0].src),'Autoplay skips both secret tracks before campaign completion');
for(const game of ['equalizarr','asteroids','basslines','invaders'])sandbox.window.RX3Session.saveCat(game);
assert(player.isLocked(player.tracks[fireworksIndex]));sandbox.window.RX3Session.rescueWon();assert(!player.isLocked(player.tracks[fireworksIndex]));assert(player.isLocked(player.tracks[player.secretIndex]),'DJ Wars unlock must not unlock the credits song');player.choose(fireworksIndex);assert(a.src.endsWith(player.tracks[fireworksIndex].src));player.unlockSecret();assert(!player.tracks[player.secretIndex].locked);assert(sandbox.window.RX3Session.secret);assert.equal(store.size,0);
player.beginCredits();a.currentTime=23;player.restartCredits();assert.equal(player.creditsProgress().fraction,0);assert(!player.creditsProgress().complete);player.endCredits();
console.log('Browser credits: audio clock, exactly two passes, no third playback, abort/restore and replay passed');

// A game-owned score pauses the playlist, but credits restore the user's original play state.
a.src='assets/music/slimeyfox-hyperarcade-bonus-points.mp3';a.currentTime=37.25;a.pause();
sandbox.window.RX3Audio={takeCreditsResume:()=>true};player.beginCredits();player.endCredits();assert.equal(a.currentTime,37.25);assert(!a.paused);
a.pause();sandbox.window.RX3Audio.takeCreditsResume=()=>false;player.beginCredits();player.endCredits();assert(a.paused);
console.log('Credits after game soundtrack: preserved playlist position and original paused/playing state passed');

let cafeClaim=null;sandbox.window.RX3Audio={takeCreditsResume:()=>true,claimSoundtrack:(...args)=>{cafeClaim=args}};
a.src='assets/music/slimeyfox-hyperarcade-bonus-points.mp3';a.currentTime=42;a.pause();player.beginCredits();const beforeExitPlays=a.plays;player.endCredits('cafe');
assert.deepEqual(cafeClaim,['cafe',true]);assert.equal(a.currentTime,42);assert(a.paused);assert.equal(a.plays,beforeExitPlays,'Credits-to-cafe must not play the old playlist for one frame');
console.log('Two independent RAM secrets, locked selection/autoplay and seamless credits-to-cafe music handoff passed');
