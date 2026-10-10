/* The status remains live while the rest of the native-rendered menu is static. */
(() => {
 const canvas=document.createElement('canvas');canvas.width=148;canvas.height=48;
 Object.assign(canvas.style,{position:'absolute',left:'83.59375%',top:'55.25%',width:'11.5625%',height:'6%',imageRendering:'pixelated',pointerEvents:'none'});
 document.querySelector('main').append(canvas);const ctx=canvas.getContext('2d');
 function word(value,x,y,scale,color){ctx.fillStyle=color;for(const ch of value){const n=ch>='0'&&ch<='9'?ch.charCodeAt(0)-48:ch>='A'&&ch<='Z'?ch.charCodeAt(0)-55:-1;
  if(n>=0)for(let yy=0;yy<7;yy++)for(let xx=0;xx<5;xx++)if(retroFont[n][yy]&(1<<(4-xx)))ctx.fillRect(x+xx*scale,y+yy*scale,scale,scale);
  if(ch==='/')for(let k=0;k<5;k++)ctx.fillRect(x+(4-k)*scale,y+(k+1)*scale,scale,scale);x+=6*scale;
 }}
 window.RX3Finale={sync(){canvas.hidden=!!page;if(page)return;ctx.fillStyle=theme==='red'?'#251117':'#10243c';ctx.fillRect(0,0,148,48);
  const s=window.RX3Session,count=[0,1,2,3].reduce((n,i)=>n+((s.catMask>>i)&1),0);
  word(s.allCats?'PLAY':'LOCKED',15,6,2,'#ffcd55');word(count+' / 4 CATS',15,29,1,'#fff');
  const card=document.querySelector('[data-index="3"]');card.setAttribute('aria-label','DJ Wars - Final Battle - '+(s.allCats?'Play':'Locked, '+count+' of 4 cats rescued'));
 }};
 window.RX3Finale.sync();if(new URLSearchParams(location.search).get('finale')==='1')window.RX3Credits.requestRescue();
})();
