/* Lifecycle checks with the real browser controller/session, without UI automation. */
const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict');
const controller=fs.readFileSync('Games/Native/DJ Wars/dj-wars.js','utf8');
const withoutBootstrap=controller.slice(0,controller.indexOf('if(!demo&&!window.RX3Session.allCats)'));
function element(){return {style:{},hidden:false,textContent:'',dataset:{},append(){},setAttribute(){},addEventListener(){},getContext(){return {putImageData(){}}},focus(){},setPointerCapture(){}};}
function fixture(demo='',allCats=true){
 const nodes=new Map(),frames=[],releases=[],scenes=[];let renders=0;
 const document={hidden:false,querySelector(s){if(!nodes.has(s))nodes.set(s,element());return nodes.get(s)},querySelectorAll(){return []},createElement:element,addEventListener(){},body:{style:{},append(f){frames.push(f)}}};
 const c={document,URL,URLSearchParams,location:{search:demo?'?demo='+demo:'',href:'http://localhost/Games/Native/DJ%20Wars/dj-wars.html',origin:'http://localhost'},performance:{now:()=>100},requestAnimationFrame(){return 1},cancelAnimationFrame(){},HTMLInputElement:class{}};
 c.window=c;c.parent=c;c.addEventListener=()=>{};c.RX3Audio={rescue(){},claimSoundtrack(){},releaseSoundtrack(...args){releases.push(args)},sceneMusic(...args){scenes.push(args)},stopSceneMusic(owner){scenes.push([owner,'stop'])},setEffectsLevel(){},effects(){return {disconnect(){}}},soundtrack(){return {disconnect(){}}}};
 c.AudioContext=class{constructor(){this.sampleRate=44100;this.state='suspended'}createScriptProcessor(){return c.processor={connect(){},disconnect(){}}}createChannelSplitter(){return {connect(){},disconnect(){}}}resume(){this.state='running';return Promise.resolve()}suspend(){this.state='suspended';return Promise.resolve()}close(){this.state='closed'}};
 vm.createContext(c);vm.runInContext(fs.readFileSync('src/session-state.js','utf8'),c);
 if(allCats)for(const id of ['equalizarr','asteroids','basslines','invaders'])c.RX3Session.saveCat(id);
 const values={0:4,1:0,2:0,3:5400,6:1,7:5,8:5,9:0,11:-1,13:540,14:580,17:0};
 c.fakeGame={game_value:i=>values[i]||0,game_frame(){renders++},game_tick(){},game_reset(){values[0]=0;values[19]=0},game_demo(result){values[0]=4;values[17]=result;values[19]=result?0:1}};
 vm.runInContext(withoutBootstrap,c);vm.runInContext('game=fakeGame;',c);
 return {c,frames,releases,scenes,values,get renders(){return renders},run:s=>vm.runInContext(s,c)};
}
for(const assisted of [0,1]){
 const f=fixture();f.values[17]=assisted;f.run('update()');assert.equal(f.frames.length,0);assert.equal(f.c.RX3Session.rescued,false);
 f.values[0]=3;f.values[11]=0;f.run('frame(100)');
 assert.equal(f.frames.length,1);assert.match(f.frames[0].src,/credits=1&after=finale/);assert.equal(f.renders,0,'No result/menu frame before credits');
 assert.equal(f.c.RX3Session.rescued,true);assert.equal(f.c.RX3Session.secret,false);assert.deepEqual(f.releases,[['dj-wars','credits']]);
 f.run('openCredits();update();frame(200);menu();reset();');assert.equal(f.frames.length,1);assert.equal(f.releases.length,1);
}
for(const mode of ['guetta','victory','play']){
 const f=fixture(mode,false);f.values[0]=3;f.values[11]=0;f.run('update();frame(100);openCredits();');
 assert.equal(f.frames.length,0);assert.equal(f.c.RX3Session.rescued,false);assert.equal(f.c.RX3Session.catMask,0);assert.match(f.c.document.querySelector('#status').textContent,/PREVIEW/);
 assert.equal(f.run('paused'),true);
}
const gated=fixture('',false);gated.values[0]=3;gated.values[11]=0;gated.run('openCredits()');assert.equal(gated.frames.length,0);assert.equal(gated.c.RX3Session.rescued,false);

(async()=>{
 const musicFixture=fixture('victory',false);musicFixture.values[19]=1;
 musicFixture.c.fakeGame.memory={buffer:new ArrayBuffer(4096*4)};
 const stems=new Float32Array(musicFixture.c.fakeGame.memory.buffer);stems.fill(.2,0,2048);stems.fill(.4,2048);
 musicFixture.c.fakeGame.game_audio_buffer=()=>0;musicFixture.c.fakeGame.game_audio_render=()=>{};
 await musicFixture.run('paused=false;startSound()');
 assert(musicFixture.scenes.some(s=>s[1]?.endsWith('alex-morgan-game-level-pixel-quest-loop-578505.mp3')&&s[2]));
 let fx=new Float32Array(1024),music=new Float32Array(1024);const event={outputBuffer:{getChannelData:i=>i?music:fx}};
 musicFixture.c.processor.onaudioprocess(event);assert(fx.every(n=>n>.19&&n<.21));assert(music.every(n=>n===0),'Fireworks silence only the synthesized score');
 musicFixture.values[19]=0;musicFixture.c.processor.onaudioprocess(event);assert(music.every(n=>n>.39&&n<.41));
 musicFixture.values[19]=1;musicFixture.run('pause()');assert.equal(musicFixture.scenes.at(-1)[2],false);
 musicFixture.c.processor.onaudioprocess(event);assert(fx.every(n=>n===0)&&music.every(n=>n===0));
 musicFixture.run('pause()');assert.equal(musicFixture.scenes.at(-1)[2],true);
 musicFixture.run('dispose()');assert.deepEqual(musicFixture.releases,[['dj-wars','playlist']]);assert.equal(musicFixture.c.RX3Session.rescued,false);
 console.log('DJ Wars fireworks: scene MP3, silent score with live effects, paused transport and isolated demo cleanup passed');
 const nodes=new Map(),classes=new Set(['credits-handoff']);let callback,begin=0,end=0,unlocked=0,cafe=0,painted=0,complete=false;
 const c={URLSearchParams,location:{search:'?credits=1&after=finale'},page:'',theme:'red',performance:{now:()=>1},AbortController,ImageData:class{},Uint8ClampedArray,requestAnimationFrame(fn){callback=fn;return 1},cancelAnimationFrame(){},fetch:async()=>({ok:true,arrayBuffer:async()=>new ArrayBuffer(0)}),WebAssembly:{instantiate:async()=>({instance:{exports:{memory:{buffer:new ArrayBuffer(1280*670*4)},credits_duration:()=>1000,credits_buffer:()=>0,credits_frame(){painted++}}}})}};
 c.document={hidden:false,documentElement:{classList:{remove:x=>classes.delete(x)}},querySelector(s){if(!nodes.has(s))nodes.set(s,element());return nodes.get(s)},createElement:element,addEventListener(){}};
 c.window=c;c.RX3Session={rescued:true};c.RX3MusicPlayer={beginCredits(){begin++},endCredits(){end++},creditsProgress(){return {fraction:complete?1:.5,complete}},unlockSecret(){unlocked++}};
 c.render=()=>c.RX3Credits?.sync();c.RX3Cafe={open(){cafe++;c.page='cafe';c.render()}};
 vm.createContext(c);vm.runInContext(fs.readFileSync('src/credits.js','utf8'),c);
 await new Promise(setImmediate);
 assert.equal(begin,1);assert(painted>0);assert.equal(classes.has('credits-handoff'),false);assert.equal(cafe,0);
 callback(100);assert.equal(unlocked,0);assert.equal(cafe,0);
 complete=true;callback(200);assert.equal(unlocked,1);assert.equal(cafe,1);assert.equal(end,1);assert.equal(c.page,'cafe');
 callback(300);assert.equal(cafe,1);assert.equal(unlocked,1);
 console.log('Browser finale -> credits -> cafe: once-only handoff, no result/menu flash, soundtrack release, exact completion and isolated paused demos passed.');
})().catch(e=>{console.error(e);process.exitCode=1});
