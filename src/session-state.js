/* A same-origin host keeps session progress in JS memory while game frames change.
 * No localStorage, sessionStorage, IndexedDB, cookies or disk progress files. */
(() => {
 if(window.parent!==window){try{if(window.parent.RX3Session){window.RX3Session=window.parent.RX3Session;return;}}catch{}}
 const goals={equalizarr:10,asteroids:4,basslines:8,invaders:4};
 const scores=Object.create(null);let cats=0,rescued=false,secret=false,frame=null,cafeSeen=0;
 window.RX3Session={
  best(id){return scores[id]||0;},
  score(id,value){if(Number.isFinite(value)&&value>=0)scores[id]=Math.max(scores[id]||0,Math.floor(value));return scores[id]||0;},
  saveCat(game){const i=['equalizarr','asteroids','basslines','invaders'].indexOf(game);if(i<0||(cats&(1<<i)))return -1;cats|=1<<i;window.RX3Audio?.rescue(i);return i;},
  reportProgress(game,value){if(Object.hasOwn(goals,game)&&Number.isInteger(value)&&value>=goals[game])return this.saveCat(game);return -1;},
  get cafeSeen(){return cafeSeen;},greetCats(mask){cafeSeen|=mask&cats&15;},get catMask(){return cats;},get allCats(){return cats===15;},
  rescueWon(){if(cats===15)rescued=true;},get rescued(){return rescued;},
  unlockSecret(){if(rescued)secret=true;},get secret(){return secret;},
  navigate(url){
   const target=new URL(url,location.href);if(target.origin!==location.origin)throw Error('Game navigation must stay on this origin');
   if(!frame){frame=document.createElement('iframe');frame.title='RX3 Games Collection';Object.assign(frame.style,{position:'fixed',inset:'0',width:'100%',height:'100%',border:'0',background:'#090709',zIndex:'10000'});frame.allow='autoplay; fullscreen';document.body.append(frame);document.body.style.overflow='hidden';}
   frame.src=target.href;
  }
 };
})();
