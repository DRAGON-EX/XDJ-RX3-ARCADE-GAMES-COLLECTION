const vm=require('node:vm'),fs=require('node:fs'),assert=require('node:assert/strict');
const source=fs.readFileSync('src/session-state.js','utf8');
function host(parent){
 const frames=[],window={};window.parent=parent||window;
 const c={window,URL,location:{href:'http://localhost/launcher-preview.html',origin:'http://localhost'},document:{createElement(){return {style:{}}},body:{style:{},append(f){frames.push(f)}}}};
 for(const key of ['localStorage','sessionStorage','indexedDB'])Object.defineProperty(window,key,{get(){throw Error('Persistent storage accessed')}});
 vm.runInNewContext(source,c);return {window,frames,state:window.RX3Session};
}
const a=host(),s=a.state;
s.unlockSecret();assert.equal(s.secret,false);s.score('basslines',100);s.score('basslines',20);s.score('basslines',NaN);assert.equal(s.best('basslines'),100);assert.equal(s.best('invaders'),0);
s.navigate('/Games/Native/Basslines/basslines.html');s.navigate('/launcher-preview.html');assert.equal(a.frames.length,1);
const child=host(a.window);assert.equal(child.state,s);child.state.rescueWon();assert.equal(s.rescued,false);s.saveCat('pacman');assert.equal(s.catMask,0);for(const g of ['equalizarr','asteroids','basslines'])s.saveCat(g);s.rescueWon();assert.equal(s.rescued,false);s.saveCat('invaders');assert.equal(s.allCats,true);s.rescueWon();assert.equal(s.rescued,true);s.unlockSecret();assert.equal(s.secret,true);
assert.throws(()=>s.navigate('https://example.com'));
const fresh=host().state;assert.equal(fresh.catMask,0);assert.equal(fresh.best('basslines'),0);assert.equal(fresh.rescued,false);assert.equal(fresh.secret,false);
console.log('Volatile browser session: shared across games, isolated scores, gated unlock, fresh-load reset, no persistent storage.');

const goals=host().state;
for(const [game,goal] of Object.entries({equalizarr:10,asteroids:4,basslines:8,invaders:4})){
 const before=goals.catMask;for(const n of [0,goal-1,NaN,Infinity,-1,3.5])goals.reportProgress(game,n);assert.equal(goals.catMask,before);
 goals.reportProgress(game,goal);assert.notEqual(goals.catMask,before);const saved=goals.catMask;goals.reportProgress(game,0);assert.equal(goals.catMask,saved);
}
assert.equal(goals.allCats,true);assert.equal(goals.rescued,false);assert.equal(host().state.catMask,0);
console.log('Exact level/wave goals, invalid inputs and reset after a fresh host passed.');

const visits=host().state;assert.equal(visits.cafeSeen,0);visits.greetCats(15);assert.equal(visits.cafeSeen,0);
assert.equal(visits.reportProgress('equalizarr',10),0);assert.equal(visits.reportProgress('equalizarr',10),-1);
visits.greetCats(15);assert.equal(visits.cafeSeen,1);assert.equal(host().state.cafeSeen,0);
console.log('First rescue event fires once; cafe greetings only acknowledge rescued cats and reset in a fresh session.');
