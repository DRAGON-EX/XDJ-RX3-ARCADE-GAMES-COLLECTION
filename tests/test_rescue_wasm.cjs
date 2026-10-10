const fs=require('fs'),assert=require('assert/strict');
(async()=>{for(const [folder,stem] of [['Equalizarr','equalizarr'],['Asteroids','asteroids'],['Basslines','basslines'],['Space Invaders','space-invaders']]){
 const a=(await WebAssembly.instantiate(fs.readFileSync(`Games/Native/${folder}/${stem}.wasm`),{})).instance.exports;
 a.game_reset(1729,0);a.game_frame(0,0);const frame=()=>Buffer.from(new Uint8Array(a.memory.buffer,a.game_buffer(),1280*800*4));const before=frame(),score=a.game_value(0),progress=a.game_value(7);
 a.game_rescue_begin(2);a.game_frame(0,0);assert.notDeepEqual(frame(),before);
 for(let i=0;i<1199;i++){a.game_tick(31,25);a.game_sound(65);assert.equal(a.game_value(8),1);}a.game_tick(31,25);assert.equal(a.game_value(8),0);a.game_frame(0,0);
 assert.equal(a.game_value(0),score);assert.equal(a.game_value(7),progress);assert.deepEqual(frame(),before,`${folder}: gameplay must freeze during rescue`);
 a.game_rescue_begin(0);a.game_reset(42,0);assert.equal(a.game_value(8),0);console.log(folder+': 20-second rescue freezes simulation and ignores held controls; reset clears playback.');
}})().catch(e=>{console.error(e);process.exitCode=1});
