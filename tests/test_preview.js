/* Exercise hidden debug gestures and timer teardown without a browser/GPU. */
const fs=require('fs'),vm=require('vm'),assert=require('assert');
const nodes=new Map(),intervals=new Map();let time=1000,id=0;
const context={fillRect(){},clearRect(){},drawImage(){}};
function node(selector){if(!nodes.has(selector))nodes.set(selector,{hidden:false,open:false,tagName:'DIV',getContext(){return context},addEventListener(){},setAttribute(){},showModal(){this.open=true},close(){this.open=false}});return nodes.get(selector)}
const sandbox={document:{querySelector:node,querySelectorAll(){return []},activeElement:{tagName:'BODY'}},performance:{now(){return time}},Image:function(){},location:{search:'',href:''},URLSearchParams,requestAnimationFrame(){},setTimeout(){return ++id},clearTimeout(){},setInterval(fn){const next=++id;intervals.set(next,fn);return next},clearInterval(i){intervals.delete(i)},window:{}};
vm.createContext(sandbox);
const source=fs.readFileSync('launcher-preview.html','utf8').split('<script>')[1].split('</script>')[0];vm.runInContext(source,sandbox);
assert.equal(intervals.size,0);
for(let i=0;i<4;i++){node('#version').onclick();time+=100}assert.equal(intervals.size,0);
time+=600;node('#version').onclick();assert.equal(intervals.size,0);
for(let i=0;i<4;i++){time+=100;node('#version').onclick()}
assert.equal(intervals.size,1);assert.equal(node('#debug-canvas').hidden,false);
for(const fn of intervals.values())fn();node('#info-close').onclick();assert.equal(intervals.size,0);assert.equal(node('#debug-canvas').hidden,true);
console.log('Browser five-tap timeout, live debug timer and close cleanup passed');
